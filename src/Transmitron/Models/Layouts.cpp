#include "Layouts.hpp"
#include <cstring>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <optional>
#include <system_error>
#include <wx/log.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <cppcodec/base32_rfc4648.hpp>

#define wxLOG_COMPONENT "models/layouts" // NOLINT

namespace fs = std::filesystem;
using namespace Transmitron::Models;

Layouts::Layouts()
{
  const auto id = mAvailableId++;
  auto layout = std::make_unique<Node>();
  layout->name = "Default";
  layout->perspective = DefaultPerspective;
  layout->path = "";
  layout->saved = true;
  mLayouts.insert({id, std::move(layout)});

  const auto parent = wxDataViewItem(nullptr);
  const auto item = toItem(id);
  ItemAdded(parent, item);
}

// Public {

bool Layouts::load(const std::string &configDir)
{
  if (configDir.empty())
  {
    wxLogWarning("No directory provided");
    return false;
  }

  mLayoutsDir = configDir + "/layouts";

  const bool exists = fs::exists(mLayoutsDir);
  const bool isDir = fs::is_directory(mLayoutsDir);

  std::error_code ec;
  if (exists && !isDir && !fs::remove(mLayoutsDir, ec))
  {
    wxLogWarning("Could not remove file %s: %s", mLayoutsDir, ec.message());
    return false;
  }

  if (!exists && !fs::create_directory(mLayoutsDir))
  {
    wxLogWarning(
      "Could not create profiles directory: %s",
      mLayoutsDir
    );
    return false;
  }

  for (const auto &entry : fs::directory_iterator(mLayoutsDir))
  {
    wxLogMessage("Checking %s", entry.path().u8string());

    // Get name.
    std::vector<uint8_t> decoded;
    try
    {
      decoded = cppcodec::base32_rfc4648::decode(
        entry.path().stem().u8string()
      );
    }
    catch (cppcodec::parse_error &e)
    {
      wxLogError(
        "Could not decode '%s': %s",
        entry.path().u8string(),
        e.what()
      );
      continue;
    }
    const std::string name{decoded.begin(), decoded.end()};

    std::ifstream input(entry.path());
    if (!input.is_open())
    {
      wxLogWarning("Could not open '%s'", entry.path().u8string());
      continue;
    }

    std::stringstream buffer;
    buffer << input.rdbuf();
    if (!nlohmann::json::accept(buffer.str()))
    {
      wxLogWarning("Could not parse '%s'", entry.path().u8string());
      continue;
    }

    const auto j = nlohmann::json::parse(buffer.str());
    const auto perspectiveIt = j.find("perspective");
    if (
      perspectiveIt == std::end(j)
      || perspectiveIt->type() != nlohmann::json::value_t::string
    ) {
      wxLogWarning("Could not parse '%s'", entry.path().u8string());
      continue;
    }

    const auto id = mAvailableId++;
    auto layout = std::make_unique<Node>();
    layout->name        = name;
    layout->perspective = perspectiveIt->get<std::string>();
    layout->path        = entry.path();
    layout->saved       = false;
    mLayouts.insert({id, std::move(layout)});

    const auto parent = wxDataViewItem(nullptr);
    const auto item = toItem(id);
    ItemAdded(parent, item);
  }

  return true;
}

bool Layouts::remove(wxDataViewItem item)
{
  const auto id = toId(item);
  const auto &node = mLayouts.at(id);

  std::error_code ec;
  fs::remove_all(node->path, ec);
  if (ec)
  {
    wxLogError("Could not delete '%s': %s", node->name, ec.message());
    return false;
  }

  mLayouts.erase(id);

  const auto parent = wxDataViewItem(nullptr);
  ItemDeleted(parent, item);

  return true;
}

wxDataViewItem Layouts::create(
  const std::string &name,
  const std::string &perspective
) {
  const bool nameExists = std::any_of(
    std::begin(mLayouts),
    std::end(mLayouts),
    [&name](const auto &layout)
    {
      return layout.second->name == name;
    }
  );

  if (nameExists)
  {
    wxLogError("Could not create '%s': layout exists", name);
    return wxDataViewItem(0);
  }

  std::string encoded;
  try { encoded = cppcodec::base32_rfc4648::encode(name); }
  catch (cppcodec::parse_error &e)
  {
    wxLogError("Could not encode '%s': %s", name, e.what());
    return wxDataViewItem(0);
  }

  const std::string path = fmt::format(
    "{}/{}",
    mLayoutsDir,
    encoded
  );

  const auto id = mAvailableId++;
  auto layout = std::make_unique<Node>();
  layout->name = name;
  layout->perspective = perspective;
  layout->path = path;
  layout->saved = false;
  mLayouts.insert({id, std::move(layout)});

  const auto parent = wxDataViewItem(nullptr);
  const auto item = toItem(id);
  ItemAdded(parent, item);

  save(id);

  return item;
}

// Getters {

wxDataViewItem Layouts::getDefault()
{
  return toItem(1);
}

const Layouts::Perspective_t &Layouts::getPerspective(wxDataViewItem item) const
{
  const auto id = toId(item);
  const auto &node = mLayouts.at(id);
  return node->perspective;
}

const std::string &Layouts::getName(wxDataViewItem item) const
{
  const auto id = toId(item);
  const auto &node = mLayouts.at(id);
  return node->name;
}

std::string Layouts::getUniqueName() const
{
  const constexpr std::string_view NewLayoutName{"New Layout"};
  std::string uniqueName{NewLayoutName};
  unsigned postfix = 0;
  while (std::any_of(
      std::begin(mLayouts),
      std::end(mLayouts),
      [=](const auto &layout)
      {
        return layout.second->name == uniqueName;
      }
  )) {
    ++postfix;
    uniqueName = fmt::format("{} - {}", NewLayoutName, postfix);
  }

  return uniqueName;
}

wxDataViewItem Layouts::findItemByName(const std::string &name) const
{
  const auto it = std::find_if(
    std::begin(mLayouts),
    std::end(mLayouts),
    [name](const auto &layout){
      return layout.second->name == name;
    }
  );

  if (it == std::end(mLayouts))
  {
    return wxDataViewItem(nullptr);
  }

  return toItem(it->first);
}

// Getters }

// wxDataViewModel interface {

unsigned Layouts::GetChildren(
  const wxDataViewItem &parent,
  wxDataViewItemArray &children
) const {
  if (parent.IsOk())
  {
    return 0;
  }

  for (const auto &node : mLayouts)
  {
    children.push_back(toItem(node.first));
  }

  std::sort(
    std::begin(children),
    std::end(children),
    [this](wxDataViewItem lhs, wxDataViewItem rhs)
    {
      const auto lhsid = toId(lhs);
      const auto rhsid = toId(rhs);

      auto lhsv = mLayouts.at(lhsid)->name;
      auto rhsv = mLayouts.at(rhsid)->name;

      std::transform(
        rhsv.begin(),
        rhsv.end(),
        rhsv.begin(),
        [](unsigned char c)
        {
          return std::tolower(c);
        }
      );

      std::transform(
        lhsv.begin(),
        lhsv.end(),
        lhsv.begin(),
        [](unsigned char c)
        {
          return std::tolower(c);
        }
      );

      return lhsv < rhsv;
    }
  );

  return (unsigned)children.size();
}

unsigned Layouts::GetColumnCount() const
{
  return (unsigned)Column::Max;
}

wxString Layouts::GetColumnType(unsigned int /* col */) const
{
  return wxDataViewTextRenderer::GetDefaultType();
}

wxDataViewItem Layouts::GetParent(
  const wxDataViewItem &/* item */
) const {
  return wxDataViewItem(nullptr);
}

bool Layouts::IsEnabled(
  const wxDataViewItem &/* item */,
  unsigned int /* col */
) const {
  return true;
}

bool Layouts::IsContainer(
  const wxDataViewItem &item
) const {
  return !item.IsOk();
}

void Layouts::GetValue(
  wxVariant &value,
  const wxDataViewItem &item,
  unsigned int col
) const {
  const auto id = toId(item);
  const auto &node = mLayouts.at(id);

  switch ((Column)col) {
    case Column::Name: {
      value = node->name;
    } break;
    default: {}
  }
}

bool Layouts::SetValue(
  const wxVariant &value,
  const wxDataViewItem &item,
  unsigned int col
) {
  const auto id = toId(item);
  if (col != (unsigned)Column::Name) { return false; }
  if (value.GetType() != "string") { return false; }
  if (value.GetString().empty()) { return false; }

  const std::string newName = value.GetString().ToStdString();

  auto &node = mLayouts.at(id);

  if (node->name == newName)
  {
    return true;
  }

  for (const auto &layout : mLayouts)
  {
    if (layout.second->name == newName)
    {
      return false;
    }
  }

  wxLogInfo("Renaming '%s' to '%s'", node->name, newName);

  std::string encoded;
  try { encoded = cppcodec::base32_rfc4648::encode(newName); }
  catch (cppcodec::parse_error &e)
  {
    wxLogError("Could not encode '%s': %s", newName, e.what());
    return false;
  }

  const std::string newPath = fmt::format(
    "{}/{}",
    mLayoutsDir,
    encoded
  );

  std::error_code ec;
  fs::rename(node->path, newPath, ec);
  if (ec)
  {
    wxLogError(
      "Could not rename '%s' to '%s': %s",
      node->path.c_str(),
      newPath,
      ec.message()
    );
    return false;
  }

  node->name = newName;
  node->path = newPath;

  ItemChanged(item);

  return true;
}

// wxDataViewModel interface }

// Public }

// Private {

bool Layouts::save(size_t index)
{
  auto &layout = mLayouts.at(index);
  if (layout->saved) { return true; }

  std::ofstream output(layout->path);
  if (!output.is_open())
  {
    wxLogError(
      "Could not save '%s':",
      layout->path.c_str(),
      std::strerror(errno)
    );
    return false;
  }

  output << nlohmann::json{{"perspective", layout->perspective}};
  layout->saved = true;
  return true;
}

// Static {

Layouts::Node::Id_t Layouts::toId(const wxDataViewItem &item)
{
  uintptr_t result = 0;
  const void *id = item.GetID();
  std::memcpy(&result, &id, sizeof(item.GetID()));
  return result;
}

wxDataViewItem Layouts::toItem(Node::Id_t id)
{
  void *itemId = nullptr;
  const uintptr_t value = id;
  std::memcpy(&itemId, &value, sizeof(id));
  return wxDataViewItem(itemId);
}

// Static }

// Private }

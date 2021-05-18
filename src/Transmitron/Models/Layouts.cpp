#include "Layouts.hpp"
#include <cstring>
#include <fstream>
#include <algorithm>
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
  mLayouts.push_back(nullptr);

  auto layout = std::make_unique<Layout>();
  layout->name = "Default";
  layout->perspective = DefaultPerspective;
  layout->path = "";
  layout->saved = true;
  mLayouts.push_back(std::move(layout));
}

bool Layouts::load(const std::string &configDir)
{
  if (configDir.empty())
  {
    wxLogWarning("No directory provided");
    return false;
  }

  mLayoutsDir = configDir + "/layouts";

  bool exists = fs::exists(mLayoutsDir);
  bool isDir = fs::is_directory(mLayoutsDir);

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

    auto layout = std::make_unique<Layout>();
    layout->name = name;
    layout->perspective = perspectiveIt->get<std::string>();
    layout->path = entry.path();
    layout->saved = false;
    mLayouts.push_back(std::move(layout));
    wxLogInfo("Loaded %s", entry.path().u8string());
    wxLogInfo("Current size %zu", mLayouts.size());
  }

  return true;
}
wxArrayString Layouts::getNames() const
{
  wxArrayString result;
  for (auto it = mLayouts.begin() + 1; it != mLayouts.end(); ++it)
  {
    result.push_back((*it)->name);
  }
  return result;
}

bool Layouts::remove(wxDataViewItem item)
{
  const auto index = toIndex(item);
  auto &node = mLayouts.at(index);

  std::error_code ec;
  fs::remove_all(node->path, ec);
  if (ec)
  {
    wxLogError("Could not delete '%s': %s", node->name, ec.message());
    return false;
  }

  auto toRemoveIt = std::begin(mLayouts);
  std::advance(toRemoveIt, index);
  wxLogInfo("Size: %zu", mLayouts.size());
  for (const auto &l : mLayouts)
  {
    if (l == nullptr) { continue; }
    wxLogInfo("  - %s", l->name);
  }
  wxLogInfo("removing index: %zu, name: %s", index, (*toRemoveIt)->name);
  mLayouts.erase(toRemoveIt);
  wxDataViewItem parent(0);
  ItemDeleted(parent, item);
  for (const auto &l : mLayouts)
  {
    if (l == nullptr) { continue; }
    wxLogInfo("  - %s", l->name);
  }
  wxLogInfo("Size: %zu", mLayouts.size());

  return true;
}

std::optional<std::string> Layouts::getLayout(const std::string &name) const
{
  const auto it = std::find_if(
    std::begin(mLayouts) + 1,
    std::end(mLayouts),
    [name](const auto &layout){
      return layout->name == name;
    }
  );

  if (it == std::end(mLayouts))
  {
    return std::nullopt;
  }

  return (*it)->perspective;
}

std::string Layouts::getUniqueName() const
{
  const constexpr std::string_view NewLayoutName{"New Layout"};
  std::string uniqueName{NewLayoutName};
  unsigned postfix = 0;
  while (std::any_of(
      std::begin(mLayouts) + 1,
      std::end(mLayouts),
      [=](const auto &layout)
      {
        return layout->name == uniqueName;
      }
  )) {
    ++postfix;
    uniqueName = fmt::format("{} - {}", NewLayoutName, postfix);
  }

  return uniqueName;
}

wxDataViewItem Layouts::create(
  const std::string &name,
  const std::string &perspective
) {
  wxLogError("Creating layout '%s'...", name);

  const bool nameExists = std::any_of(
    std::begin(mLayouts) + 1,
    std::end(mLayouts),
    [&name](const auto &layout)
    {
      return layout->name == name;
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

  auto layout = std::make_unique<Layout>();
  layout->name = name;
  layout->perspective = perspective;
  layout->path = path;
  layout->saved = false;
  mLayouts.push_back(std::move(layout));

  wxDataViewItem parent(nullptr);
  const auto index = mLayouts.size() - 1;
  const auto item = toItem(index);
  save(index);
  ItemAdded(parent, item);

  return item;
}

unsigned Layouts::GetColumnCount() const
{
  return (unsigned)Column::Max;
}

wxString Layouts::GetColumnType(unsigned int /* col */) const
{
  return wxDataViewTextRenderer::GetDefaultType();
}

void Layouts::GetValue(
  wxVariant &variant,
  const wxDataViewItem &item,
  unsigned int col
) const {
  const auto &layout = mLayouts.at(toIndex(item));

  switch ((Column)col) {
    case Column::Name: {
      variant = layout->name;
    } break;
    default: {}
  }
}

bool Layouts::SetValue(
  const wxVariant &/* variant */,
  const wxDataViewItem &/* item */,
  unsigned int /* col */
) {
  return false;
}

bool Layouts::IsEnabled(
  const wxDataViewItem &/* item */,
  unsigned int /* col */
) const {
  return true;
}

wxDataViewItem Layouts::GetParent(
  const wxDataViewItem &/* item */
) const {
  return wxDataViewItem(nullptr);
}

bool Layouts::IsContainer(
  const wxDataViewItem &item
) const {
  return !item.IsOk();
}

unsigned Layouts::GetChildren(
  const wxDataViewItem &/* parent */,
  wxDataViewItemArray &array
) const {
  wxLogInfo("Current count: %zu", mLayouts.size() - 1);
  for (size_t i = 1; i != mLayouts.size(); ++i)
  {
    array.Add(toItem(i));
  }
  return (unsigned)mLayouts.size() - 1;
}

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

size_t Layouts::toIndex(const wxDataViewItem &item)
{
  uintptr_t result = 0;
  const void *id = item.GetID();
  std::memcpy(&result, &id, sizeof(uintptr_t));
  return result;
}

wxDataViewItem Layouts::toItem(size_t index)
{
  void *id = nullptr;
  const uintptr_t value = index;
  std::memcpy(&id, &value, sizeof(uintptr_t));
  return wxDataViewItem(id);
}

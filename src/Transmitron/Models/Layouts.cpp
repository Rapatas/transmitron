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
  const auto id = mAvailableId++;
  auto layout = std::make_unique<Layout>();
  layout->name = "Default";
  layout->perspective = DefaultPerspective;
  layout->path = "";
  layout->saved = true;
  mLayouts.insert({id, std::move(layout)});
  mRemap.push_back(id);
  RowAppended();
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

    const auto id = mAvailableId++;
    auto layout = std::make_unique<Layout>();
    layout->name = name;
    layout->perspective = perspectiveIt->get<std::string>();
    layout->path = entry.path();
    layout->saved = false;
    mLayouts.insert({id, std::move(layout)});
    mRemap.push_back(id);
    RowAppended();
  }

  return true;
}
wxArrayString Layouts::getNames() const
{
  wxArrayString result;
  for (const auto &layout : mLayouts)
  {
    result.push_back(layout.second->name);
  }
  return result;
}

bool Layouts::remove(wxDataViewItem item)
{
  const auto row = GetRow(item);
  const auto index = mRemap.at(row);

  std::error_code ec;
  fs::remove_all(mLayouts.at(index)->path, ec);
  if (ec)
  {
    wxLogError("Could not delete '%s': %s", mLayouts.at(index)->name, ec.message());
    return false;
  }

  auto toRemoveIt = std::begin(mRemap);
  std::advance(toRemoveIt, GetRow(item));
  mLayouts.erase(index);
  mRemap.erase(toRemoveIt);

  RowDeleted(row);

  return true;
}

std::optional<std::string> Layouts::getLayout(const std::string &name) const
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
    return std::nullopt;
  }

  return (*it).second->perspective;
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
  auto layout = std::make_unique<Layout>();
  layout->name = name;
  layout->perspective = perspective;
  layout->path = path;
  layout->saved = false;
  mLayouts.insert({id, std::move(layout)});
  mRemap.push_back(id);

  save(id);
  RowAppended();

  return GetItem((unsigned)mRemap.size() - 1);
}

unsigned Layouts::GetColumnCount() const
{
  return (unsigned)Column::Max;
}

wxString Layouts::GetColumnType(unsigned int /* col */) const
{
  return wxDataViewTextRenderer::GetDefaultType();
}

unsigned Layouts::GetCount() const
{
  return (unsigned)mLayouts.size();
}

void Layouts::GetValueByRow(
  wxVariant &variant,
  unsigned int row,
  unsigned int col
) const {
  const auto &layout = mLayouts.at(mRemap.at(row));

  switch ((Column)col) {
    case Column::Name: {
      variant = layout->name;
    } break;
    default: {}
  }
}

bool Layouts::SetValueByRow(
  const wxVariant &/* variant */,
  unsigned int /* row */,
  unsigned int /* col */
) {
  return false;
}

bool Layouts::GetAttrByRow(
  unsigned int /* row */,
  unsigned int /* col */,
  wxDataViewItemAttr &/* attr */
) const {
  return false;
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

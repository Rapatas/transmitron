#include <fstream>
#include <iterator>
#include <wx/log.h>
#include <fmt/core.h>
#include <cppcodec/base32_rfc4648.hpp>
#include "Transmitron/Info.hpp"
#include "Profiles.hpp"

#define wxLOG_COMPONENT "models/profiles" // NOLINT

namespace fs = std::filesystem;
using namespace Transmitron::Models;
using namespace Transmitron;

Profiles::Profiles() {}

bool Profiles::load(const std::string &configDir)
{
  if (configDir.empty())
  {
    wxLogWarning("No directory provided");
    return false;
  }

  mProfilesDir = configDir + "/profiles";

  const bool exists = fs::exists(mProfilesDir);
  const bool isDir = fs::is_directory(mProfilesDir);

  if (exists && !isDir && !fs::remove(mProfilesDir))
  {
    wxLogWarning("Could not remove file %s", mProfilesDir);
    return false;
  }

  if (!exists && !fs::create_directory(mProfilesDir))
  {
    wxLogWarning(
      "Could not create profiles directory: %s",
      mProfilesDir
    );
    return false;
  }

  for (const auto &entry : fs::directory_iterator(mProfilesDir))
  {
    wxLogMessage("Checking %s", entry.path().u8string());
    if (entry.status().type() != fs::file_type::directory) { continue; }

    const auto item = loadProfileFile(entry.path());
    if (!item.IsOk())
    {
      continue;
    }

    wxLogMessage("Loaded %s", entry.path().u8string());
  }

  return true;
}

bool Profiles::updateBrokerOptions(
  wxDataViewItem item,
  const MQTT::BrokerOptions &brokerOptions
) {
  if (!item.IsOk())
  {
    return false;
  }

  const auto id = toId(item);
  auto &profile = mProfiles.at(id);
  profile->brokerOptions = brokerOptions;
  profile->saved = false;

  save(id);
  ItemChanged(item);
  return true;
}

bool Profiles::rename(
  wxDataViewItem item,
  const std::string &name
) {
  if (!item.IsOk())
  {
    return false;
  }

  const auto id = toId(item);
  auto &profile = mProfiles.at(id);

  if (profile->name == name)
  {
    return true;
  }

  const bool nameExists = std::any_of(
    std::begin(mProfiles),
    std::end(mProfiles),
    [&name](const auto &profile)
    {
      return profile.second->name == name;
    }
  );
  if (nameExists)
  {
    wxLogError("Could not rename '%s': file exists", profile->name);
    return false;
  }

  std::string encoded;
  try { encoded = cppcodec::base32_rfc4648::encode(name); }
  catch (cppcodec::parse_error &e)
  {
    wxLogError("Could not rename '%s': %s", profile->name, e.what());
    return false;
  }

  const std::string pathNew = fmt::format(
    "{}/{}",
    mProfilesDir,
    encoded
  );

  std::error_code ec;
  fs::rename(profile->path, pathNew, ec);
  if (ec)
  {
    wxLogError(
      "Could not rename '%s' to '%s': %s",
      profile->name,
      name,
      ec.message()
    );
    return false;
  }

  profile->name = name;
  profile->path = pathNew;
  profile->saved = false;

  save(id);
  ItemChanged(item);

  return true;
}

bool Profiles::remove(wxDataViewItem item)
{
  const auto id = toId(item);
  auto &node = mProfiles.at(id);

  std::error_code ec;
  fs::remove_all(node->path, ec);
  if (ec)
  {
    wxLogError("Could not delete '%s': %s", node->name, ec.message());
    return false;
  }

  mProfiles.erase(id);

  const auto parent = wxDataViewItem(nullptr);
  ItemDeleted(parent, item);

  return true;
}

std::string Profiles::getUniqueName() const
{
  const constexpr std::string_view NewProfileName{"New Profile"};
  std::string uniqueName{NewProfileName};
  unsigned postfix = 0;
  while (std::any_of(
      std::begin(mProfiles),
      std::end(mProfiles),
      [=](const auto &profile)
      {
        return profile.second->name == uniqueName;
      }
  )) {
    ++postfix;
    uniqueName = fmt::format("{} - {}", NewProfileName, postfix);
  }

  return uniqueName;
}

wxDataViewItem Profiles::createProfile()
{
  const std::string uniqueName = getUniqueName();

  std::string encoded;
  try { encoded = cppcodec::base32_rfc4648::encode(uniqueName); }
  catch (cppcodec::parse_error &e)
  {
    wxLogError("Could not encode '%s': %s", uniqueName, e.what());
    return wxDataViewItem(0);
  }

  const std::string path = fmt::format(
    "{}/{}",
    mProfilesDir,
    encoded
  );

  if (!fs::exists(path) && !fs::create_directory(path))
  {
    wxLogWarning("Could not create profile '%s' directory", path);
    return wxDataViewItem(0);
  }

  wxObjectDataPtr<Models::Snippets> snippetsModel{new Models::Snippets};
  snippetsModel->load(path);

  const auto id = mAvailableId++;
  auto profile = std::make_unique<Node>();
  profile->name = uniqueName;
  profile->path = path;
  profile->saved = false;
  profile->snippetsModel = snippetsModel;
  mProfiles.insert({id, std::move(profile)});

  save(id);

  const auto parent = wxDataViewItem(nullptr);
  const auto item = toItem(id);
  ItemAdded(parent, item);

  return item;
}

const MQTT::BrokerOptions &Profiles::getBrokerOptions(wxDataViewItem item) const
{
  return mProfiles.at(toId(item))->brokerOptions;
}

std::string Profiles::getName(wxDataViewItem item) const
{
  return mProfiles.at(toId(item))->name;
}

wxObjectDataPtr<Snippets> Profiles::getSnippetsModel(wxDataViewItem item)
{
  return mProfiles.at(toId(item))->snippetsModel;
}

unsigned Profiles::GetColumnCount() const
{
  return (unsigned)Column::Max;
}

wxString Profiles::GetColumnType(unsigned int /* col */) const
{
  return wxDataViewTextRenderer::GetDefaultType();
}

void Profiles::GetValue(
  wxVariant &variant,
  const wxDataViewItem &item,
  unsigned int col
) const {
  const auto &profile = mProfiles.at(toId(item));

  switch ((Column)col) {
    case Column::Name: {
      variant = profile->name;
    } break;
    case Column::URL: {
      variant =
        profile->brokerOptions.getHostname()
        + ":"
        + std::to_string(profile->brokerOptions.getPort());
    } break;
    default: {}
  }
}

bool Profiles::SetValue(
  const wxVariant &/* variant */,
  const wxDataViewItem &/* item */,
  unsigned int /* col */
) {
  return false;
}

bool Profiles::IsEnabled(
  const wxDataViewItem &/* item */,
  unsigned int /* col */
) const {
  return true;
}

wxDataViewItem Profiles::GetParent(
  const wxDataViewItem &/* item */
) const {
  return wxDataViewItem(nullptr);
}

bool Profiles::IsContainer(
  const wxDataViewItem &item
) const {
  return !item.IsOk();
}

unsigned Profiles::GetChildren(
  const wxDataViewItem &parent,
  wxDataViewItemArray &children
) const {
  if (parent.IsOk())
  {
    return 0;
  }

  for (const auto &node : mProfiles)
  {
    children.Add(toItem(node.first));
  }

  std::sort(
    std::begin(children),
    std::end(children),
    [this](wxDataViewItem lhs, wxDataViewItem rhs)
    {
      const auto lhsid = toId(lhs);
      const auto rhsid = toId(rhs);

      auto lhsv = mProfiles.at(lhsid)->name;
      auto rhsv = mProfiles.at(rhsid)->name;

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

bool Profiles::save(size_t id)
{
  auto &profile = mProfiles.at(id);
  if (profile->saved) { return true; }

  const auto brokerOptionsFilepath = fmt::format(
    "{}/{}",
    profile->path.string(),
    BrokerOptionsFilename
  );

  std::ofstream output(brokerOptionsFilepath);
  if (!output.is_open())
  {
    wxLogError(
      "Could not save '%s':",
      brokerOptionsFilepath,
      std::strerror(errno)
    );
    return false;
  }

  output << profile->brokerOptions.toJson();
  profile->saved = true;
  return true;
}

wxDataViewItem Profiles::loadProfileFile(const std::filesystem::path &filepath)
{
  std::vector<uint8_t> decoded;
  try
  {
    decoded = cppcodec::base32_rfc4648::decode(
      filepath.stem().u8string()
    );
  }
  catch (cppcodec::parse_error &e)
  {
    wxLogError(
      "Could not decode '%s': %s",
      filepath.u8string(),
      e.what()
    );
    return wxDataViewItem(nullptr);
  }
  const std::string name{decoded.begin(), decoded.end()};

  const auto brokerOptionsFilepath = fmt::format(
    "{}/{}",
    filepath.string(),
    BrokerOptionsFilename
  );

  std::ifstream brokerOptionsFile(brokerOptionsFilepath);
  if (!brokerOptionsFile.is_open())
  {
    wxLogWarning("Could not open '%s'", filepath.u8string());
    return wxDataViewItem(nullptr);
  }

  std::stringstream buffer;
  buffer << brokerOptionsFile.rdbuf();
  if (!nlohmann::json::accept(buffer.str()))
  {
    wxLogWarning("Could not parse '%s'", filepath.u8string());
    return wxDataViewItem(nullptr);
  }

  auto j = nlohmann::json::parse(buffer.str());
  auto brokerOptions = MQTT::BrokerOptions::fromJson(j);

  wxObjectDataPtr<Models::Snippets> snippetsModel{new Models::Snippets};
  snippetsModel->load(filepath.string());

  const auto id = mAvailableId++;
  auto profile = std::make_unique<Node>(Node{
    name,
    brokerOptions,
    filepath,
    snippetsModel,
    true
  });
  mProfiles.insert({id, std::move(profile)});

  return toItem(id);
}

size_t Profiles::toId(const wxDataViewItem &item)
{
  uintptr_t result = 0;
  const void *id = item.GetID();
  std::memcpy(&result, &id, sizeof(uintptr_t));
  return result;
}

wxDataViewItem Profiles::toItem(size_t id)
{
  void *itemId = nullptr;
  const uintptr_t value = id;
  std::memcpy(&itemId, &value, sizeof(uintptr_t));
  return wxDataViewItem(itemId);
}


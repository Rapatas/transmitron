#include <fstream>
#include <wx/log.h>
#include <fmt/core.h>
#include <cppcodec/base32_rfc4648.hpp>
#include "Transmitron/Info.hpp"
#include "Profiles.hpp"

#define wxLOG_COMPONENT "models/profiles"

namespace fs = std::filesystem;
using namespace Transmitron::Models;
using namespace Transmitron;

Profiles::Profiles()
{
  mProfiles.push_back(nullptr);
}

bool Profiles::load(const std::string &configDir)
{
  if (configDir.empty())
  {
    wxLogWarning("No directory provided");
    return false;
  }

  mProfilesDir = configDir + "/profiles";

  bool exists = fs::exists(mProfilesDir);
  bool isDir = fs::is_directory(mProfilesDir);

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
    std::string name{decoded.begin(), decoded.end()};

    // Get BrokerOptions.
    auto brokerOptionsFilepath = fmt::format(
      "{}/{}",
      entry.path().c_str(),
      BrokerOptionsFilename
    );

    std::ifstream brokerOptionsFile(brokerOptionsFilepath);
    if (!brokerOptionsFile.is_open())
    {
      wxLogWarning("Could not open '%s'", entry.path().u8string());
      continue;
    }

    std::stringstream buffer;
    buffer << brokerOptionsFile.rdbuf();
    if (!nlohmann::json::accept(buffer.str()))
    {
      wxLogWarning("Could not parse '%s'", entry.path().u8string());
      continue;
    }

    auto j = nlohmann::json::parse(buffer.str());
    auto brokerOptions = ValueObjects::BrokerOptions::fromJson(j);

    wxObjectDataPtr<Models::Snippets> snippetsModel{new Models::Snippets};
    snippetsModel->load(entry.path());

    auto profile = std::make_unique<Profile>(Profile{
      name,
      brokerOptions,
      entry.path(),
      std::move(snippetsModel),
      true
    });
    mProfiles.push_back(std::move(profile));
  }

  return true;
}

bool Profiles::updateBrokerOptions(
  wxDataViewItem &item,
  const ValueObjects::BrokerOptions &brokerOptions
) {
  const auto &profile =  mProfiles.at(toIndex(item));
  profile->brokerOptions = brokerOptions;

  std::string dir = toDir(profile->name);

  if (
    !profile->saved
    && !fs::create_directory(dir)
  ) {
      wxLogError("Could not create profile directory");
      return false;
  }

  auto brokerOptionsFilepath = fmt::format(
    "{}/{}",
    dir,
    BrokerOptionsFilename
  );

  std::ofstream brokerOptionsFile(brokerOptionsFilepath);
  if (!brokerOptionsFile.is_open())
  {
    wxLogError("Could not save broker options");
    return false;
  }

  brokerOptionsFile << brokerOptions.toJson().dump(2);
  brokerOptionsFile.close();

  profile->brokerOptions = brokerOptions;
  profile->saved = true;
  ItemChanged(item);

  return true;
}

bool Profiles::updateName(
  wxDataViewItem &item,
  const std::string &name
) {
  const auto &profile =  mProfiles.at(toIndex(item));
  if (profile->name == name)
  {
    return true;
  }

  std::string before = toDir(profile->name);
  std::string after = toDir(name);

  fs::rename(before, after);
  if (errno != 0)
  {
    wxLogError("Could not rename profile to '%s'", name);
    return false;
  }

  return true;
}

wxDataViewItem Profiles::createProfile()
{
  const char *newProfileName = "New Profile";
  std::string uniqueName{newProfileName};
  unsigned postfix = 0;
  while (std::any_of(
      std::begin(mProfiles) + 1,
      std::end(mProfiles),
      [=](const auto &profile)
      {
        return profile->name == uniqueName;
      }
  )) {
    ++postfix;
    uniqueName = fmt::format("{} - {}", newProfileName, postfix);
  }

  auto profile = std::make_unique<Profile>();
  profile->name = uniqueName;
  profile->saved = false;
  mProfiles.push_back(std::move(profile));

  wxDataViewItem parent(nullptr);
  auto item = toItem(mProfiles.size() - 1);
  ItemAdded(parent, item);

  return item;
}

const ValueObjects::BrokerOptions &Profiles::getBrokerOptions(wxDataViewItem item) const
{
  return mProfiles.at(toIndex(item))->brokerOptions;
}

std::string Profiles::getName(wxDataViewItem item) const
{
  return mProfiles.at(toIndex(item))->name;
}

const wxObjectDataPtr<Snippets> Profiles::getSnippetsModel(wxDataViewItem item)
{
  return mProfiles.at(toIndex(item))->snippetsModel;
}

unsigned Profiles::GetColumnCount() const
{
  return (unsigned)Column::Max;
}

wxString Profiles::GetColumnType(unsigned int col) const
{
  switch ((Column)col)
  {
    case Column::Name: { return wxDataViewTextRenderer::GetDefaultType(); } break;
    case Column::URL:  { return wxDataViewTextRenderer::GetDefaultType(); } break;
    default: { return "string"; }
  }
}

void Profiles::GetValue(
  wxVariant &variant,
  const wxDataViewItem &item,
  unsigned int col
) const {
  const auto &profile = mProfiles.at(toIndex(item));

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
  const wxVariant &variant,
  const wxDataViewItem &item,
  unsigned int col
) {
  return false;
}

bool Profiles::IsEnabled(
  const wxDataViewItem &item,
  unsigned int col
) const {
  return true;
}

wxDataViewItem Profiles::GetParent(
  const wxDataViewItem &item
) const {
  return wxDataViewItem(nullptr);
}

bool Profiles::IsContainer(
  const wxDataViewItem &item
) const {
  if (!item.IsOk())
  {
    return true;
  }
  return false;
}

unsigned int Profiles::GetChildren(
  const wxDataViewItem &parent,
  wxDataViewItemArray &array
) const {
  for (size_t i = 1; i != mProfiles.size(); ++i)
  {
    array.Add(toItem(i));
  }
  return mProfiles.size() - 1;
}

std::string Profiles::toDir(const std::string &name) const
{
  auto encV = cppcodec::base32_rfc4648::encode(name);
  std::string encoded{std::begin(encV), std::end(encV)};
  return fmt::format("{}/{}", mProfilesDir, encoded);
}

size_t Profiles::toIndex(const wxDataViewItem &item)
{
  return reinterpret_cast<size_t>(item.GetID());
}

wxDataViewItem Profiles::toItem(size_t index)
{
  return wxDataViewItem(reinterpret_cast<void*>(index));
}


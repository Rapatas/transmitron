#include "Common/Filesystem.hpp"
#include <fstream>
#include <iterator>
#include <optional>
#include <stdexcept>

#include <fmt/core.h>

#include "Common/Log.hpp"
#include "Common/Url.hpp"
#include "MQTT/BrokerOptions.hpp"
#include "Profiles.hpp"
#include "Transmitron/Models/KnownTopics.hpp"
#include "Transmitron/Types/ClientOptions.hpp"
#include "Transmitron/Notifiers/Layouts.hpp"

namespace fs = Common::fs;
using namespace Transmitron::Models;
using namespace Transmitron;
using namespace Common;

Profiles::Profiles(const wxObjectDataPtr<Layouts> &layouts) :
  mLayoutsModel(layouts)
{
  mLogger = Common::Log::create("Models::Profiles");

  auto *notifier = new Notifiers::Layouts;
  mLayoutsModel->AddNotifier(notifier);

  notifier->Bind(Events::LAYOUT_REMOVED, &Profiles::onLayoutRemoved, this);
  notifier->Bind(Events::LAYOUT_CHANGED, &Profiles::onLayoutChanged, this);
}

bool Profiles::load(
  const std::string &configDir,
  const std::string &cacheDir
) {
  if (configDir.empty())
  {
    mLogger->warn("No config directory provided");
    return false;
  }

  if (cacheDir.empty())
  {
    mLogger->warn("No cache directory provided");
    return false;
  }

  mCacheProfilesDir = cacheDir + "/profiles";
  mConfigProfilesDir = configDir + "/profiles";

  if (!ensureDirectoryExists(mConfigProfilesDir)) { return false; }
  if (!ensureDirectoryExists(mCacheProfilesDir))  { return false; }

  for (const auto &entry : fs::directory_iterator(mConfigProfilesDir))
  {
    mLogger->info("Checking {}", entry.path().u8string());
    if (entry.status().type() != fs::file_type::directory) { continue; }

    const auto item = loadProfile(entry.path());
    if (!item.IsOk())
    {
      continue;
    }

    mLogger->info("Loaded {}", entry.path().u8string());
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

bool Profiles::updateClientOptions(
  wxDataViewItem item,
  const Types::ClientOptions &clientOptions
) {
    if (!item.IsOk())
    {
      return false;
    }

    const auto id = toId(item);
    auto &profile = mProfiles.at(id);
    profile->clientOptions = clientOptions;
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
    mLogger->error("Could not rename '{}': file exists", profile->name);
    return false;
  }

  const std::string encoded = Url::encode(name);

  const std::string pathNew = fmt::format(
    "{}/{}",
    mConfigProfilesDir,
    encoded
  );

  std::error_code ec;
  fs::rename(profile->path, pathNew, ec);
  if (ec)
  {
    mLogger->error(
      "Could not rename '{}' to '{}': {}",
      profile->name,
      name,
      ec.message()
    );
    return false;
  }

  wxObjectDataPtr<Models::Snippets> snippets{new Models::Snippets};
  snippets->load(pathNew);

  profile->name = name;
  profile->path = pathNew;
  profile->snippets = snippets;
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
    mLogger->error("Could not delete '{}': {}", node->name, ec.message());
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
  const std::string encoded = Url::encode(uniqueName);

  const std::string path = fmt::format(
    "{}/{}",
    mConfigProfilesDir,
    encoded
  );

  if (!fs::exists(path) && !fs::create_directory(path))
  {
    mLogger->warn("Could not create profile '{}' directory", path);
    return wxDataViewItem(0);
  }

  wxObjectDataPtr<Models::Snippets> snippets{new Models::Snippets};
  snippets->load(path);

  wxObjectDataPtr<Models::KnownTopics> topicsSubscribed{new Models::KnownTopics};
  wxObjectDataPtr<Models::KnownTopics> topicsPublished{new Models::KnownTopics};

  const auto id = mAvailableId++;
  auto profile = std::make_unique<Node>();
  profile->name = uniqueName;
  profile->path = path;
  profile->saved = false;
  profile->snippets = snippets;
  profile->topicsSubscribed = topicsSubscribed;
  profile->topicsPublished = topicsPublished;
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

const Types::ClientOptions &Profiles::getClientOptions(wxDataViewItem item) const
{
    return mProfiles.at(toId(item))->clientOptions;
}

wxString Profiles::getName(wxDataViewItem item) const
{
  const auto name = mProfiles.at(toId(item))->name;
  const auto wxs = wxString::FromUTF8(name.data(), name.length());
  return wxs;
}

wxDataViewItem Profiles::getItemFromName(const std::string &profileName) const
{
  const auto it = std::find_if(
    std::begin(mProfiles),
    std::end(mProfiles),
    [=](const auto &profile)
    {
      const auto &node = profile.second;
      return node->name == profileName;
    }
  );

  if (it == std::end(mProfiles))
  {
    return wxDataViewItem(nullptr);
  }

  return toItem(it->first);
}

wxObjectDataPtr<Snippets> Profiles::getSnippetsModel(wxDataViewItem item)
{
  return mProfiles.at(toId(item))->snippets;
}

wxObjectDataPtr<KnownTopics> Profiles::getTopicsSubscribed(wxDataViewItem item)
{
  return mProfiles.at(toId(item))->topicsSubscribed;
}

wxObjectDataPtr<KnownTopics> Profiles::getTopicsPublished(wxDataViewItem item)
{
  return mProfiles.at(toId(item))->topicsPublished;
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
      const auto &name = profile->name;
      const auto wxs = wxString::FromUTF8(name.data(), name.length());
      variant = wxs;
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
  const bool saved = saveOptionsBroker(id) && saveOptionsClient(id);
  profile->saved = saved;
  return saved;
}

bool Profiles::saveOptionsBroker(size_t id)
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
    mLogger->error(
      "Could not save '{}':",
      brokerOptionsFilepath,
      std::strerror(errno)
    );
    return false;
  }

  output << profile->brokerOptions.toJson();
  return true;
}

bool Profiles::saveOptionsClient(size_t id)
{
  auto &profile = mProfiles.at(id);
  if (profile->saved) { return true; }

  const auto clientOptionsFilepath = fmt::format(
    "{}/{}",
    profile->path.string(),
    ClientOptionsFilename
  );

  std::ofstream output(clientOptionsFilepath);
  if (!output.is_open())
  {
    mLogger->error(
      "Could not save '{}':",
      clientOptionsFilepath,
      std::strerror(errno)
    );
    return false;
  }

  output << profile->clientOptions.toJson();
  return true;
}

wxDataViewItem Profiles::loadProfile(const Common::fs::path &directory)
{
  const std::string encoded = directory.stem().u8string();
  std::string decoded;
  try
  {
    decoded = Url::decode(encoded);
  }
  catch (std::runtime_error &e)
  {
    mLogger->error("Could not decode '{}': {}", encoded, e.what());
    return wxDataViewItem(nullptr);
  }
  const std::string name{decoded.begin(), decoded.end()};

  const auto brokerOptions = [this, directory]()
  {
    const auto opt = loadProfileOptionsBroker(directory);
    if (!opt) { return MQTT::BrokerOptions{}; }
    return opt.value();
  }();

  const auto clientOptions = [this, directory]()
  {
    const auto opt = loadProfileOptionsClient(directory);
    if (!opt) { return Types::ClientOptions{}; }
    return opt.value();
  }();

  wxObjectDataPtr<Models::Snippets> snippets{new Models::Snippets};
  snippets->load(directory.string());

  wxObjectDataPtr<Models::KnownTopics> topicsSubscribed{new Models::KnownTopics};
  wxObjectDataPtr<Models::KnownTopics> topicsPublished{new Models::KnownTopics};

  const auto cacheProfile = fmt::format("{}/{}", mCacheProfilesDir, encoded);
  const auto topics = cacheProfile + "/topics";
  if (true // NOLINT
    && ensureDirectoryExists(cacheProfile)
    && ensureDirectoryExists(topics)
  ) {
    topicsSubscribed->load(topics + "/subscribed.txt");
    topicsPublished->load(topics + "/published.txt");
  }

  topicsPublished->append(snippets->getKnownTopics());

  const auto id = mAvailableId++;
  auto profile = std::make_unique<Node>(Node{
    name,
    clientOptions,
    brokerOptions,
    directory,
    snippets,
    topicsSubscribed,
    topicsPublished,
    true
  });
  mProfiles.insert({id, std::move(profile)});

  return toItem(id);
}

std::optional<MQTT::BrokerOptions> Profiles::loadProfileOptionsBroker(
  const Common::fs::path &directory
) {
  const auto brokerOptionsFilepath = fmt::format(
    "{}/{}",
    directory.string(),
    BrokerOptionsFilename
  );

  std::ifstream brokerOptionsFile(brokerOptionsFilepath);
  if (!brokerOptionsFile.is_open())
  {
    mLogger->warn("Could not open '{}'", brokerOptionsFilepath);
    return std::nullopt;
  }

  std::stringstream buffer;
  buffer << brokerOptionsFile.rdbuf();
  if (!nlohmann::json::accept(buffer.str()))
  {
    mLogger->warn("Could not parse '{}'", brokerOptionsFilepath);
    return std::nullopt;
  }

  const auto j = nlohmann::json::parse(buffer.str());
  auto brokerOptions = MQTT::BrokerOptions::fromJson(j);
  return brokerOptions;
}

std::optional<Types::ClientOptions> Profiles::loadProfileOptionsClient(
  const Common::fs::path &directory
) {
  const auto clientOptionsFilepath = fmt::format(
    "{}/{}",
    directory.string(),
    ClientOptionsFilename
  );

  std::ifstream clientOptionsFile(clientOptionsFilepath);
  if (!clientOptionsFile.is_open())
  {
    mLogger->warn("Could not open '{}'", clientOptionsFilepath);
    return std::nullopt;
  }

  std::stringstream buffer;
  buffer << clientOptionsFile.rdbuf();
  if (!nlohmann::json::accept(buffer.str()))
  {
    mLogger->warn("Could not parse '{}'", clientOptionsFilepath);
    return std::nullopt;
  }

  const auto j = nlohmann::json::parse(buffer.str());
  auto clientOptions = Types::ClientOptions::fromJson(j);
  return clientOptions;

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

void Profiles::onLayoutRemoved(Events::Layout &/* event */)
{
  const std::string newName{Layouts::DefaultName};
  renameLayoutIfMissing(newName);
}

void Profiles::onLayoutChanged(Events::Layout &event)
{
  const auto item = event.getItem();
  const std::string newName = mLayoutsModel->getName(item);
  renameLayoutIfMissing(newName);
}

bool Profiles::ensureDirectoryExists(const std::string &dir) const
{
  const bool exists = fs::exists(dir);
  const bool isDir = fs::is_directory(dir);

  if (exists && !isDir && !fs::remove(dir))
  {
    mLogger->warn("Could not remove file {}", dir);
    return false;
  }

  if (!exists && !fs::create_directory(dir))
  {
    mLogger->warn(
      "Could not create cache directory: {}",
      dir
    );
    return false;
  }

  return true;
}

void Profiles::renameLayoutIfMissing(const std::string &newName)
{
  const auto &layouts = mLayoutsModel->getLabelVector();

  for (auto &profile : mProfiles)
  {
    const auto it = std::find(
      std::begin(layouts),
      std::end(layouts),
      profile.second->clientOptions.getLayout()
    );

    if (it != std::end(layouts))
    {
      continue;
    }

    auto &node = profile.second;
    node->clientOptions = Types::ClientOptions{newName};
    node->saved = false;

    const auto id = profile.first;
    save(id);
  }
}

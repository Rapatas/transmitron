
#include <limits>
#include <algorithm>
#include <fstream>
#include <optional>
#include <stdexcept>

#include <fmt/core.h>

#include "Common/Log.hpp"
#include "Common/String.hpp"
#include "Common/Url.hpp"
#include "MQTT/BrokerOptions.hpp"
#include "Profiles.hpp"
#include "GUI/Models/KnownTopics.hpp"
#include "GUI/Types/ClientOptions.hpp"
#include "GUI/Notifiers/Layouts.hpp"

using namespace Rapatas::Transmitron;
using namespace GUI::Models;
using namespace GUI;
using namespace Common;

using Port = MQTT::BrokerOptions::Port;
constexpr Port DefaultMqttPort = 1883;
constexpr std::string_view NewName {"New profile"};

Profiles::Profiles(
  const wxObjectDataPtr<Layouts> &layouts,
  const ArtProvider &artProvider
) :
  FsTree(
    "Profiles",
    static_cast<size_t>(Column::Max),
    artProvider
  ),
  mLogger(Common::Log::create("Models::Profiles")),
  mArtProvider(artProvider),
  mLayoutsModel(layouts),
  mQuickConnect(*this, mArtProvider)
{

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

  const bool result = FsTree::load(mConfigProfilesDir);
  createQuickConnect();
  return result;
}

bool Profiles::updateBrokerOptions(
  wxDataViewItem item,
  const MQTT::BrokerOptions &brokerOptions
) {
  auto *leaf = getLeaf(item);
  if (leaf == nullptr) { return false; }
  auto &profile = *dynamic_cast<Profile*>(leaf);

  profile.brokerOptions = brokerOptions;

  const auto id = toId(item);
  leafSave(id);

  ItemChanged(item);

  return true;
}

bool Profiles::updateClientOptions(
  wxDataViewItem item,
  const Types::ClientOptions &clientOptions
) {
    auto *leaf = getLeaf(item);
    if (leaf == nullptr) { return false; }
    auto &profile = *dynamic_cast<Profile*>(leaf);

    profile.clientOptions = clientOptions;

    const auto id = toId(item);
    leafSave(id);

    ItemChanged(item);

    return true;
}

void Profiles::updateQuickConnect(const std::string &url)
{
  const auto parts = String::split(url, ':');

  const auto port = [&]() -> Port
  {
    if (parts.size() != 2) { return DefaultMqttPort; }
    const auto &portStr = parts[1];
    const auto isNumeric = std::all_of(
      portStr.begin(),
      portStr.end(),
      [](char value){ return std::isdigit(value) != 0; }
    );
    if (!isNumeric) { return DefaultMqttPort; }
    return static_cast<Port>(std::stoul(portStr));
  }();

  const auto hostname = [&]()
  {
    if (parts.empty()) { return std::string("localhost"); }
    return parts[0];
  }();

  mQuickConnect.brokerOptions.setPort(port);
  mQuickConnect.brokerOptions.setHostname(hostname);
}

void Profiles::createQuickConnect()
{
  if (mQuickConnectId != 0) { return; }

  const std::string uniqueName = "QuickConnect";
  const std::string encoded = Url::encode(uniqueName);

  const std::string path = fmt::format(
    "{}/{}",
    mConfigProfilesDir,
    encoded
  );

  bool canSave = true;
  if (!fs::exists(path) && !fs::create_directory(path))
  {
    mLogger->warn("Could not create profile '{}' directory", path);
    canSave = false;
  }

  const wxObjectDataPtr<Models::Messages> messages{new Models::Messages(mArtProvider)};
  messages->load(path);

  const wxObjectDataPtr<Models::KnownTopics> topicsSubscribed{new Models::KnownTopics};
  const wxObjectDataPtr<Models::KnownTopics> topicsPublished{new Models::KnownTopics};

  mQuickConnectId = std::numeric_limits<Id>::max();
  mQuickConnect.messages = messages;
  mQuickConnect.topicsSubscribed = topicsSubscribed;
  mQuickConnect.topicsPublished = topicsPublished;

  if (canSave)
  {
    const auto cache = String::replace(path, mConfigProfilesDir, mCacheProfilesDir);
    mQuickConnect.save(path, cache);
  }
}

wxDataViewItem Profiles::createProfile(wxDataViewItem parentItem)
{
  const std::string uniqueName = createUniqueName(wxDataViewItem(nullptr), NewName);

  const wxObjectDataPtr<Models::Messages> messages{new Models::Messages(mArtProvider)};
  const wxObjectDataPtr<Models::KnownTopics> topicsSubscribed{new Models::KnownTopics};
  const wxObjectDataPtr<Models::KnownTopics> topicsPublished{new Models::KnownTopics};

  auto profile = std::make_unique<Profile>(*this, mArtProvider);
  profile->messages = messages;
  profile->topicsSubscribed = topicsSubscribed;
  profile->topicsPublished = topicsPublished;

  const auto item = leafCreate(parentItem, std::move(profile), uniqueName);
  const auto id = toId(item);
  const auto path = getNodePath(id);

  messages->load(path);

  return item;
}

const MQTT::BrokerOptions &Profiles::getBrokerOptions(wxDataViewItem item) const
{
  auto *leaf = getLeaf(item);
  if (leaf == nullptr)
  {
    static MQTT::BrokerOptions empty{};
    return empty;
  }

  auto &profile = *dynamic_cast<Profile*>(leaf);
  return profile.brokerOptions;
}

const Types::ClientOptions &Profiles::getClientOptions(wxDataViewItem item) const
{
  auto *leaf = getLeaf(item);
  if (leaf == nullptr)
  {
    static Types::ClientOptions empty{};
    return empty;
  }

  auto &profile = *dynamic_cast<Profile*>(leaf);
  return profile.clientOptions;
}

wxDataViewItem Profiles::getQuickConnect() const
{
  return toItem(mQuickConnectId);
}

wxObjectDataPtr<Messages> Profiles::getMessagesModel(wxDataViewItem item)
{
  auto *leaf = getLeaf(item);
  if (leaf == nullptr) { return wxObjectDataPtr<Messages>{nullptr}; }

  auto &profile = *dynamic_cast<Profile*>(leaf);
  return profile.messages;
}

wxObjectDataPtr<KnownTopics> Profiles::getTopicsSubscribed(wxDataViewItem item)
{
  auto *leaf = getLeaf(item);
  if (leaf == nullptr) { return wxObjectDataPtr<KnownTopics>{nullptr}; }

  auto &profile = *dynamic_cast<Profile*>(leaf);
  return profile.topicsSubscribed;
}

wxObjectDataPtr<KnownTopics> Profiles::getTopicsPublished(wxDataViewItem item)
{
  auto *leaf = getLeaf(item);
  if (leaf == nullptr) { return wxObjectDataPtr<KnownTopics>{nullptr}; }

  auto &profile = *dynamic_cast<Profile*>(leaf);
  return profile.topicsPublished;
}

void Profiles::leafValue(Id id, wxDataViewIconText &value, unsigned int col) const
{
  auto item = toItem(id);
  auto *leaf = getLeaf(item);
  if (leaf == nullptr) { return; }
  auto &profile = *dynamic_cast<Profile*>(leaf);

  if (static_cast<Column>(col) == Column::Name)
  {
    wxIcon icon;
    icon.CopyFromBitmap(mArtProvider.bitmap(Icon::Profile));
    value.SetIcon(icon);
  }

  if (static_cast<Column>(col) == Column::URL)
  {
    const auto url = fmt::format(
      "{}:{}",
      profile.brokerOptions.getHostname(),
      profile.brokerOptions.getPort()
    );
    const auto wxs = wxString::FromUTF8(url.data(), url.length());
    value.SetText(wxs);
  }
}

bool Profiles::leafSave(Id id)
{
  auto item = toItem(id);
  auto *leaf = getLeaf(item);
  if (leaf == nullptr) { return false; }
  auto &profile = *dynamic_cast<Profile*>(leaf);

  const auto config = getNodePath(id);
  const auto cache = String::replace(config, mConfigProfilesDir, mCacheProfilesDir);
  const auto result = profile.save(config, cache);
  profile.messages->load(config);
  return result;
}

bool Profiles::Profile::save(
  const Common::fs::path &config,
  const Common::fs::path &cache
) {
  return true // NOLINT
    && saveOptionsBroker(config)
    && saveOptionsClient(config)
    && saveCache(cache);
}

bool Profiles::Profile::saveOptionsBroker(const Common::fs::path &path)
{
  const auto brokerOptionsFilepath = fmt::format(
    "{}/{}",
    path.string(),
    BrokerOptionsFilename
  );

  std::ofstream output(brokerOptionsFilepath);
  if (!output.is_open())
  {
    const auto ec = std::error_code(errno, std::system_category());
    mLogger->error(
      "Could not save '{}': {}",
      brokerOptionsFilepath,
      ec.message()
    );
    return false;
  }

  output << brokerOptions.toJson();
  return true;
}

bool Profiles::Profile::saveOptionsClient(const Common::fs::path &path)
{
  const auto clientOptionsFilepath = fmt::format(
    "{}/{}",
    path.string(),
    ClientOptionsFilename
  );

  std::ofstream output(clientOptionsFilepath);
  if (!output.is_open())
  {
    const auto ec = std::error_code(errno, std::system_category());
    mLogger->error(
      "Could not save '{}': {}",
      clientOptionsFilepath,
      ec.message()
    );
    return false;
  }

  output << clientOptions.toJson();
  return true;
}

bool Profiles::Profile::saveCache(const Common::fs::path &path) const
{
  fs::create_directories(path);
  topicsPublished->save(path.string() + "/topics/published.txt");
  topicsSubscribed->save(path.string() + "/topics/subscribed.txt");
  return true;
}

std::unique_ptr<FsTree::Leaf> Profiles::leafLoad(Id id, const Common::fs::path &path)
{
  (void)id;
  auto profile = std::make_unique<Profile>(*this, mArtProvider);
  const auto cache = String::replace(path, mConfigProfilesDir, mCacheProfilesDir);
  profile->load(path, cache);
  return std::move(profile);
}

bool Profiles::Profile::load(const Common::fs::path &config, const Common::fs::path &cache)
{
  mLogger->debug("Profile::load({})", config.string());

  if (const auto opt = loadOptionsClient(config))
  {
    clientOptions = opt.value();
  }

  if (const auto opt = loadOptionsBroker(config))
  {
    brokerOptions = opt.value();
  }

  const wxObjectDataPtr<Models::Messages> messages{new Models::Messages(mArtProvider)};
  const wxObjectDataPtr<Models::KnownTopics> topicsSubscribed{new Models::KnownTopics};
  const wxObjectDataPtr<Models::KnownTopics> topicsPublished{new Models::KnownTopics};

  this->messages = messages;
  this->topicsSubscribed = topicsSubscribed;
  this->topicsPublished = topicsPublished;

  messages->load(config.string());

  topicsSubscribed->load(cache.string() + "/topics/subscribed.txt");
  topicsPublished->load(cache.string() + "/topics/published.txt");

  topicsPublished->append(messages->getKnownTopics());

  return true;
}

std::optional<MQTT::BrokerOptions> Profiles::Profile::loadOptionsBroker(
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

  const auto data = nlohmann::json::parse(buffer.str());
  auto brokerOptions = MQTT::BrokerOptions::fromJson(data);
  return brokerOptions;
}

std::optional<Types::ClientOptions> Profiles::Profile::loadOptionsClient(
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

  const auto data = nlohmann::json::parse(buffer.str());
  auto clientOptions = Types::ClientOptions::fromJson(data);
  return clientOptions;

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

  for (auto [nodeId, leaf] : getLeafs())
  {
    auto &profile = *dynamic_cast<Profile*>(leaf);
    const auto it = std::find(
      std::begin(layouts),
      std::end(layouts),
      profile.clientOptions.getLayout()
    );
    if (it != std::end(layouts)) { continue; }

    profile.clientOptions = Types::ClientOptions{newName};
    leafSave(nodeId);
  }
}

bool Profiles::isLeaf(const Common::fs::directory_entry &entry) const
{
  if (entry.status().type() != fs::file_type::directory) { return false; }
  for (const auto &entry : fs::directory_iterator(entry))
  {
    const bool hasIt =  entry.path().filename() == "client-options.json";
    if (hasIt) {
      return true;
    }
  }
  return false;
}

Profiles::Profile::Profile(
  const Profiles &profiles,
  const ArtProvider &artProvider
) :
  mLogger(Common::Log::create("Models::Profile")),
  mProfiles(profiles),
  mArtProvider(artProvider),
  messages(new Models::Messages(mArtProvider)),
  topicsSubscribed(new Models::KnownTopics),
  topicsPublished(new Models::KnownTopics)
{}

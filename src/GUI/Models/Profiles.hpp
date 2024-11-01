#pragma once

#include <spdlog/spdlog.h>
#include <wx/dataview.h>

#include "MQTT/BrokerOptions.hpp"
#include "Messages.hpp"
#include "Layouts.hpp"
#include "GUI/Models/KnownTopics.hpp"
#include "GUI/Types/ClientOptions.hpp"
#include "GUI/Events/Layout.hpp"
#include "GUI/Models/FsTree.hpp"

namespace Rapatas::Transmitron::GUI::Models
{

class Profiles :
  public FsTree
{
public:

  enum class Column : uint8_t
  {
    Name,
    URL,
    Max
  };

  explicit Profiles(
    const wxObjectDataPtr<Layouts> &layouts,
    const ArtProvider &artProvider
  );

  bool load(
    const std::string &configDir,
    const std::string &cacheDir
  );

  bool updateBrokerOptions(
    wxDataViewItem item,
    const MQTT::BrokerOptions &brokerOptions
  );
  bool updateClientOptions(
    wxDataViewItem item,
    const Types::ClientOptions &clientOptions
  );
  wxDataViewItem createProfile(wxDataViewItem parentItem);
  void updateQuickConnect(const std::string &url);

  [[nodiscard]] const MQTT::BrokerOptions &getBrokerOptions(wxDataViewItem item) const;
  [[nodiscard]] const Types::ClientOptions &getClientOptions(wxDataViewItem item) const;
  [[nodiscard]] wxDataViewItem getQuickConnect() const;

  wxObjectDataPtr<Messages> getMessagesModel(wxDataViewItem item);
  wxObjectDataPtr<KnownTopics> getTopicsSubscribed(wxDataViewItem item);
  wxObjectDataPtr<KnownTopics> getTopicsPublished(wxDataViewItem item);

private:

  struct Profile :
    public FsTree::Leaf
  {
    Profile(
      const Profiles &profiles,
      const ArtProvider &artProvider
    );

    std::shared_ptr<spdlog::logger> mLogger;
    const Profiles &mProfiles;
    const ArtProvider &mArtProvider;

    Types::ClientOptions clientOptions;
    MQTT::BrokerOptions brokerOptions;
    wxObjectDataPtr<Messages> messages;
    wxObjectDataPtr<KnownTopics> topicsSubscribed;
    wxObjectDataPtr<KnownTopics> topicsPublished;

    bool saveOptionsBroker(const Common::fs::path &path);
    bool saveOptionsClient(const Common::fs::path &path);
    bool save(const Common::fs::path &config, const Common::fs::path &cache);
    bool load(const Common::fs::path &config, const Common::fs::path &cache);
    [[nodiscard]] bool saveCache(const Common::fs::path &path) const;

    std::optional<MQTT::BrokerOptions> loadOptionsBroker(
      const Common::fs::path &directory
    );
    std::optional<Types::ClientOptions> loadOptionsClient(
      const Common::fs::path &directory
    );
  };

  static constexpr std::string_view BrokerOptionsFilename = "broker-options.json";
  static constexpr std::string_view ClientOptionsFilename = "client-options.json";

  std::shared_ptr<spdlog::logger> mLogger;
  const ArtProvider &mArtProvider;
  std::string mConfigProfilesDir;
  std::string mCacheProfilesDir;
  wxObjectDataPtr<Layouts> mLayoutsModel;
  Id mQuickConnectId{};
  Profile mQuickConnect;

  void createQuickConnect();
  [[nodiscard]] bool ensureDirectoryExists(const std::string &dir) const;
  void renameLayoutIfMissing(const std::string &newName);

  void onLayoutRemoved(Events::Layout &event);
  void onLayoutChanged(Events::Layout &event);

  [[nodiscard]] bool isLeaf(const Common::fs::directory_entry &entry) const override;
  std::unique_ptr<Leaf> leafLoad(Id id, const Common::fs::path &path) override;
  void leafValue(Id id, wxDataViewIconText &value, unsigned int col) const override;
  bool leafSave(Id id) override;

};

} // namespace Rapatas::Transmitron::GUI::Models


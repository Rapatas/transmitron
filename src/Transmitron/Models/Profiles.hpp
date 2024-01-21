#ifndef TRANSMITRON_MODELS_PROFILES_HPP
#define TRANSMITRON_MODELS_PROFILES_HPP

#include "Common/Filesystem.hpp"

#include <spdlog/spdlog.h>
#include <wx/dataview.h>

#include "MQTT/BrokerOptions.hpp"
#include "Messages.hpp"
#include "Layouts.hpp"
#include "Transmitron/Models/KnownTopics.hpp"
#include "Transmitron/Types/ClientOptions.hpp"
#include "Transmitron/Events/Layout.hpp"

namespace Transmitron::Models
{

class Profiles :
  public wxDataViewModel
{
public:

  enum class Column : unsigned
  {
    Name,
    URL,
    Max
  };

  explicit Profiles(const wxObjectDataPtr<Layouts> &layouts);

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
  bool rename(
    wxDataViewItem item,
    const std::string &name
  );
  wxDataViewItem createProfile();
  bool remove(wxDataViewItem item);
  void updateQuickConnect(std::string url);
  std::string getUniqueName() const;

  const MQTT::BrokerOptions &getBrokerOptions(wxDataViewItem item) const;
  const Types::ClientOptions &getClientOptions(wxDataViewItem item) const;
  wxString getName(wxDataViewItem item) const;
  wxDataViewItem getItemFromName(const std::string &profileName) const;
  wxDataViewItem getQuickConnect() const;

  wxObjectDataPtr<Messages> getMessagesModel(wxDataViewItem item);
  wxObjectDataPtr<KnownTopics> getTopicsSubscribed(wxDataViewItem item);
  wxObjectDataPtr<KnownTopics> getTopicsPublished(wxDataViewItem item);

  // wxDataViewModel interface.
  unsigned GetColumnCount() const override;
  wxString GetColumnType(unsigned int col) const override;
  void GetValue(
    wxVariant &variant,
    const wxDataViewItem &item,
    unsigned int col
  ) const override;
  bool SetValue(
    const wxVariant &variant,
    const wxDataViewItem &item,
    unsigned int col
  ) override;
  bool IsEnabled(
    const wxDataViewItem &item,
    unsigned int col
  ) const override;
  wxDataViewItem GetParent(
    const wxDataViewItem &item
  ) const override;
  bool IsContainer(
    const wxDataViewItem &item
  ) const override;
  unsigned int GetChildren(
    const wxDataViewItem &parent,
    wxDataViewItemArray &children
  ) const override;

private:

  struct Node
  {
    using Id_t = size_t;
    std::string name;
    Types::ClientOptions clientOptions;
    MQTT::BrokerOptions brokerOptions;
    Common::fs::path path;
    wxObjectDataPtr<Messages> messages;
    wxObjectDataPtr<KnownTopics> topicsSubscribed;
    wxObjectDataPtr<KnownTopics> topicsPublished;
    bool saved = false;
  };

  static constexpr std::string_view BrokerOptionsFilename = "broker-options.json";
  static constexpr std::string_view ClientOptionsFilename = "client-options.json";

  std::shared_ptr<spdlog::logger> mLogger;
  Node::Id_t mAvailableId = 1;
  std::map<Node::Id_t, std::unique_ptr<Node>> mProfiles;
  std::string mConfigProfilesDir;
  std::string mCacheProfilesDir;
  wxObjectDataPtr<Layouts> mLayoutsModel;
  Node::Id_t mQuickConnectId{};

  bool save(size_t id);
  bool saveOptionsBroker(size_t id);
  bool saveOptionsClient(size_t id);
  void createQuickConnect();
  bool ensureDirectoryExists(const std::string &dir) const;
  wxDataViewItem loadProfile(const Common::fs::path &directory);
  std::optional<MQTT::BrokerOptions> loadProfileOptionsBroker(
    const Common::fs::path &directory
  );
  std::optional<Types::ClientOptions> loadProfileOptionsClient(
    const Common::fs::path &directory
  );
  void renameLayoutIfMissing(const std::string &newName);

  void onLayoutRemoved(Events::Layout &event);
  void onLayoutChanged(Events::Layout &event);

  static size_t toId(const wxDataViewItem &item);
  static wxDataViewItem toItem(size_t id);
};

}

#endif // TRANSMITRON_MODELS_PROFILES_HPP

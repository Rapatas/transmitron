#ifndef TRANSMITRON_MODELS_PROFILES_HPP
#define TRANSMITRON_MODELS_PROFILES_HPP

#include <filesystem>

#include <spdlog/spdlog.h>
#include <wx/dataview.h>

#include "MQTT/BrokerOptions.hpp"
#include "Snippets.hpp"

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

  explicit Profiles();

  bool load(const std::string &configDir);

  bool updateBrokerOptions(
    wxDataViewItem item,
    const MQTT::BrokerOptions &brokerOptions
  );
  bool rename(
    wxDataViewItem item,
    const std::string &name
  );
  wxDataViewItem createProfile();
  bool remove(wxDataViewItem item);
  std::string getUniqueName() const;

  const MQTT::BrokerOptions &getBrokerOptions(wxDataViewItem item) const;
  wxString getName(wxDataViewItem item) const;

  wxObjectDataPtr<Snippets> getSnippetsModel(wxDataViewItem item);

private:

  struct Node
  {
    using Id_t = size_t;
    std::string name;
    MQTT::BrokerOptions brokerOptions;
    std::filesystem::path path;
    wxObjectDataPtr<Models::Snippets> snippetsModel;
    bool saved = false;
  };

  static constexpr const char *BrokerOptionsFilename =
    "broker-options.json";

  std::shared_ptr<spdlog::logger> mLogger;
  Node::Id_t mAvailableId = 1;
  std::map<Node::Id_t, std::unique_ptr<Node>> mProfiles;
  std::string mProfilesDir;

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

  bool save(size_t id);
  wxDataViewItem loadProfileFile(const std::filesystem::path &filepath);

  static size_t toId(const wxDataViewItem &item);
  static wxDataViewItem toItem(size_t id);
};

}

#endif // TRANSMITRON_MODELS_PROFILES_HPP

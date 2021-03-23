#ifndef TRANSMITRON_MODELS_PROFILES_HPP
#define TRANSMITRON_MODELS_PROFILES_HPP

#include <filesystem>
#include <wx/dataview.h>
#include "Snippets.hpp"
#include "Transmitron/ValueObjects/BrokerOptions.hpp"

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
    const ValueObjects::BrokerOptions &brokerOptions
  );
  bool updateName(
    wxDataViewItem &item,
    const std::string &name
  );
  wxDataViewItem createProfile();
  bool remove(wxDataViewItem item);

  const ValueObjects::BrokerOptions &getBrokerOptions(wxDataViewItem item) const;
  std::string getName(wxDataViewItem item) const;

  wxObjectDataPtr<Snippets> getSnippetsModel(wxDataViewItem item);

private:

  struct Profile
  {
    std::string name;
    ValueObjects::BrokerOptions brokerOptions;
    std::filesystem::path path;
    wxObjectDataPtr<Models::Snippets> snippetsModel;
    bool saved = false;
  };

  static constexpr const char *BrokerOptionsFilename =
    "broker-options.json";

  std::vector<std::unique_ptr<Profile>> mProfiles;
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
    wxDataViewItemArray &array
  ) const override;

  bool save(size_t index);

  static size_t toIndex(const wxDataViewItem &item);
  static wxDataViewItem toItem(size_t index);
};

}

#endif // TRANSMITRON_MODELS_PROFILES_HPP

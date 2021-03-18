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
  virtual ~Profiles() = default;

  bool load(const std::string &configDir);

  bool updateBrokerOptions(
    wxDataViewItem &item,
    const ValueObjects::BrokerOptions &brokerOptions
  );
  bool updateName(
    wxDataViewItem &item,
    const std::string &name
  );
  wxDataViewItem createProfile();

  const ValueObjects::BrokerOptions &getBrokerOptions(wxDataViewItem item) const;
  std::string getName(wxDataViewItem item) const;

  const wxObjectDataPtr<Snippets> getSnippetsModel(wxDataViewItem item);

private:

  struct Profile
  {
    std::string name;
    ValueObjects::BrokerOptions brokerOptions;
    std::filesystem::path path;
    wxObjectDataPtr<Models::Snippets> snippetsModel;
    bool saved;
  };

  static constexpr const char *BrokerOptionsFilename =
    "broker-options.json";

  std::vector<std::unique_ptr<Profile>> mProfiles;
  std::string mProfilesDir;

  // wxDataViewModel interface.
  virtual unsigned GetColumnCount() const override;
  virtual wxString GetColumnType(unsigned int col) const override;
  virtual void GetValue(
    wxVariant &variant,
    const wxDataViewItem &item,
    unsigned int col
  ) const override;
  virtual bool SetValue(
    const wxVariant &variant,
    const wxDataViewItem &item,
    unsigned int col
  ) override;
  virtual bool IsEnabled(
    const wxDataViewItem &item,
    unsigned int col
  ) const override;
  virtual wxDataViewItem GetParent(
    const wxDataViewItem &item
  ) const override;
  virtual bool IsContainer(
    const wxDataViewItem &item
  ) const override;
  virtual unsigned int GetChildren(
    const wxDataViewItem &parent,
    wxDataViewItemArray &array
  ) const override;

  std::string toDir(const std::string &name) const;

  static size_t toIndex(const wxDataViewItem &item);
  static wxDataViewItem toItem(size_t index);
};

}

#endif // TRANSMITRON_MODELS_PROFILES_HPP

#pragma once

#include <map>

#include <wx/dataview.h>

#include "GUI/Models/Profiles.hpp"

namespace Rapatas::Transmitron::GUI::Models {

class ProfilesWrapper : public wxDataViewModel
{
public:

  using Id = size_t;
  using Item = wxDataViewItem;

  explicit ProfilesWrapper(const wxObjectDataPtr<Profiles> &profiles);

  [[nodiscard]] unsigned GetColumnCount() const override;
  [[nodiscard]] wxString GetColumnType(unsigned int col) const override;
  void GetValue(
    wxVariant &variant,
    const Item &item,
    unsigned int col //
  ) const override;
  bool SetValue(
    const wxVariant &value,
    const Item &item,
    unsigned int col //
  ) override;
  [[nodiscard]] bool IsEnabled(
    const Item &item,
    unsigned int col //
  ) const override;
  [[nodiscard]] Item GetParent(const Item &item) const override;
  [[nodiscard]] bool IsContainer(const Item &item) const override;
  unsigned int GetChildren(
    const Item &parent,
    wxDataViewItemArray &array //
  ) const override;

private:

  struct Node {
    enum class Type : uint8_t {
      Folder,
      Payload,
    };
    Id parent = 0;
    std::string name;
    Type type = Type::Folder;
    std::list<Id> children;
  };

  const wxObjectDataPtr<Profiles> &mProfiles;
  std::map<Id, Node> mNodes;
};

} // namespace Rapatas::Transmitron::GUI::Models

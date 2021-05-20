#ifndef TRANSMITRON_WIDGETS_LAYOUTS_HPP
#define TRANSMITRON_WIDGETS_LAYOUTS_HPP

#include <wx/wx.h>
#include <wx/aui/aui.h>

#include "Transmitron/Models/Layouts.hpp"

namespace Transmitron::Widgets
{

class Layouts :
  public wxPanel
  // public wxDataViewModelNotifier
{
public:

  explicit Layouts(
    wxWindow* parent,
    wxWindowID id,
    const wxObjectDataPtr<Models::Layouts> &layoutsModel,
    wxAuiManager &auiMan,
    size_t optionsHeight
  );

private:

  const size_t mOptionsHeight;

  wxObjectDataPtr<Models::Layouts> mLayoutsModel;
  wxAuiManager &mAuiMan;
  bool mReadOnly = false;
  wxFont mFont;

  wxBoxSizer *mSizer = nullptr;
  wxButton *mSave = nullptr;
  wxComboBox *mLayoutsEdit = nullptr;
  wxComboBox *mLayoutsLocked = nullptr;

  void onLayoutSaveClicked(wxCommandEvent &event);
  void onLayoutEditEnter(wxCommandEvent &event);
  void onLayoutEditSelected(wxCommandEvent &event);
  void onLayoutLockedSelected(wxCommandEvent &event);
  void onLayoutSelected(const std::string &value);

  // wxDataViewModelNotifier interface.
  // bool Cleared() override;
  // bool ItemChanged(const wxDataViewItem &item) override;
  // void Resort() override;
  // bool ValueChanged(const wxDataViewItem &item, unsigned int col) override;
  // bool ItemAdded(
  //   const wxDataViewItem &parent,
  //   const wxDataViewItem &item
  // ) override;
  // bool ItemDeleted(
  //   const wxDataViewItem &parent,
  //   const wxDataViewItem &item
  // ) override;

};

}

#endif // TRANSMITRON_WIDGETS_LAYOUTS_HPP

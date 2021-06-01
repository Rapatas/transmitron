#ifndef TRANSMITRON_WIDGETS_LAYOUTS_HPP
#define TRANSMITRON_WIDGETS_LAYOUTS_HPP

#include <wx/wx.h>
#include <wx/aui/aui.h>

#include "Transmitron/Notifiers/Layouts.hpp"
#include "Transmitron/Models/Layouts.hpp"
#include "Transmitron/Events/Layout.hpp"

namespace Transmitron::Widgets
{

class Layouts :
  public wxPanel
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
  wxDataViewItem mCurrentSelection;
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
  void onLayoutEditLostFocus(wxFocusEvent &event);
  void onLayoutLockedSelected(wxCommandEvent &event);
  void onLayoutSelected(const std::string &value);
  void onLayoutAdded(Events::Layout &event);
  void onLayoutRemoved(Events::Layout &event);
  void onLayoutChanged(Events::Layout &event);

  wxArrayString getNames() const;
  void resize();
};

}

#endif // TRANSMITRON_WIDGETS_LAYOUTS_HPP

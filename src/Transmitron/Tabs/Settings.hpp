#ifndef TRANSMITRON_TABS_SETTINGS_HPP
#define TRANSMITRON_TABS_SETTINGS_HPP

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/dataview.h>

#include "Transmitron/Models/Layouts.hpp"

namespace Transmitron::Tabs
{

class Settings :
  public wxPanel
{
public:

  explicit Settings(
    wxWindow *parent,
    wxFontInfo labelFont,
    const wxObjectDataPtr<Models::Layouts> &layoutsModel
  );

private:

  enum class ContextIDs : unsigned
  {
    LayoutsDelete,
    LayoutsRename,
  };

  const wxFontInfo mLabelFont;

  // Layouts.
  wxPanel *mLayouts = nullptr;
  wxDataViewListCtrl *mLayoutsCtrl = nullptr;
  wxObjectDataPtr<Models::Layouts> mLayoutsModel;
  wxDataViewColumn *mLayoutColumnName = nullptr;

  void setupLayouts();

  void onLayoutsContext(wxDataViewEvent &event);
  void onContextSelected(wxCommandEvent &event);
  void onLayoutsDelete(wxCommandEvent &event);
  void onLayoutsRename(wxCommandEvent &event);
  void onLayoutsEdit(wxDataViewEvent &event);
};

}

#endif // TRANSMITRON_TABS_SETTINGS_HPP

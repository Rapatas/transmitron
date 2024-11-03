#pragma once

#include <spdlog/spdlog.h>
#include <wx/aui/aui.h>
#include <wx/wx.h>

#include "GUI/ArtProvider.hpp"
#include "GUI/Events/Layout.hpp"
#include "GUI/Models/Layouts.hpp"

namespace Rapatas::Transmitron::GUI::Widgets {

class Layouts : public wxPanel
{
public:

  explicit Layouts(
    wxWindow *parent,
    wxWindowID id,
    const wxObjectDataPtr<Models::Layouts> &layoutsModel,
    wxAuiManager *auiMan,
    const ArtProvider &artProvider,
    int optionsHeight
  );

  bool setSelectedLayout(const std::string &layoutName);

private:

  int mOptionsHeight;

  std::shared_ptr<spdlog::logger> mLogger;
  wxObjectDataPtr<Models::Layouts> mLayoutsModel;
  wxDataViewItem mCurrentSelection;
  wxAuiManager *mAuiMan;
  wxFont mFont;
  const ArtProvider &mArtProvider;

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

  [[nodiscard]] wxArrayString getNames() const;
  void resize();
};

} // namespace Rapatas::Transmitron::GUI::Widgets

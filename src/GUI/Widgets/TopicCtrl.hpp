#pragma once

#include <spdlog/spdlog.h>
#include <wx/dnd.h>
#include <wx/popupwin.h>
#include <wx/textctrl.h>
#include "GUI/Models/KnownTopics.hpp"

namespace Rapatas::Transmitron::GUI::Widgets
{

class TopicCtrl :
  public wxTextCtrl
{
public:

  explicit TopicCtrl(
    wxWindow *parent,
    wxWindowID id
  );

  enum class ContextIDs : unsigned
  {
    Copy
  };

  void setReadOnly(bool readonly);
  void addKnownTopics(
    const wxObjectDataPtr<Models::KnownTopics> &knownTopicsModel
  );

private:

  struct NotAllowedDropTarget :
    public wxDropTarget
  {
    // NOLINTNEXTLINE(readability-identifier-length)
    wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult defResult) override;
  };

  wxFont mFont;
  bool mFakeSelection = false;
  bool mFirstClick = true;
  bool mReadOnly = false;
  long mCursonPosition = 0;
  wxPopupWindow *mAutoComplete = nullptr;
  wxDataViewCtrl *mAutoCompleteList = nullptr;
  wxDataViewColumn *mAutoCompleteTopic = nullptr;
  std::shared_ptr<spdlog::logger> mLogger;
  wxObjectDataPtr<Models::KnownTopics> mKnownTopicsModel;
  bool mPopupShow = false;

  void onContextSelected(wxCommandEvent &event);

  void onContext(wxContextMenuEvent &event);
  void onDoubleClicked(wxMouseEvent &event);
  void onKeyDown(wxKeyEvent &event);
  void onKeyUp(wxKeyEvent &event);
  void onChar(wxKeyEvent &event);
  void onLeftDown(wxMouseEvent &event);
  void onLeftUp(wxMouseEvent &event);
  void onLostFocus(wxFocusEvent &event);
  void onRight(wxMouseEvent &event);
  void onRightClicked(wxMouseEvent &event);
  void onValueChanged(wxCommandEvent &event);
  void onCompletionDoubleClicked(wxDataViewEvent &event);
  void onCompletionLeftUp(wxMouseEvent &event);

  void popupHide();
  void popupShow();
  void popupRefresh();
  void autoCompleteUp();
  void autoCompleteDown();
  void autoCompleteSelect();

};

} // namespace Rapatas::Transmitron::GUI::Widgets


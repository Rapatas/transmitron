#ifndef TRANSMITRON_WIDGETS_TOPICCTRL_HPP
#define TRANSMITRON_WIDGETS_TOPICCTRL_HPP

#include <spdlog/spdlog.h>
#include <wx/dnd.h>
#include <wx/popupwin.h>
#include <wx/textctrl.h>

namespace Transmitron::Widgets
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

private:

  struct NotAllowedDropTarget :
    public wxDropTarget
  {
    wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult defResult) override;
  };

  const wxFont mFont;
  bool mFakeSelection = false;
  bool mFirstClick = true;
  bool mReadOnly = false;
  long mCursonPosition = 0;
  wxPopupWindow *mAutoComplete = nullptr;
  std::shared_ptr<spdlog::logger> mLogger;

  void onContextSelected(wxCommandEvent &e);

  void onContext(wxContextMenuEvent &e);
  void onDoubleClicked(wxMouseEvent &e);
  void onKeyDown(wxKeyEvent &e);
  void onLeftDown(wxMouseEvent &e);
  void onLeftUp(wxMouseEvent &e);
  void onLostFocus(wxFocusEvent &e);
  void onRight(wxMouseEvent &e);
  void onRightClicked(wxMouseEvent &e);

  void popupHide();
  void popupShow();

};

}

#endif // TRANSMITRON_WIDGETS_TOPICCTRL_HPP

#ifndef TRANSMITRON_WIDGETS_TOPICCTRL_HPP
#define TRANSMITRON_WIDGETS_TOPICCTRL_HPP

#include <wx/dnd.h>
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

  struct NotAllowedDropTarget :
    public wxDropTarget
  {};

  void setReadOnly(bool readonly);

private:

  const wxFont mFont;
  bool mFakeSelection = false;
  bool mFirstClick = true;
  bool mReadOnly = false;
  long mCursonPosition = 0;

  void onContextSelected(wxCommandEvent &e);

  void onContext(wxContextMenuEvent &e);
  void onDoubleClicked(wxMouseEvent &e);
  static void onKeyDown(wxKeyEvent &e);
  void onLeftDown(wxMouseEvent &e);
  void onLeftUp(wxMouseEvent &e);
  void onLostFocus(wxFocusEvent &e);
  void onRight(wxMouseEvent &e);
  void onRightClicked(wxMouseEvent &e);

};

}

#endif // TRANSMITRON_WIDGETS_TOPICCTRL_HPP

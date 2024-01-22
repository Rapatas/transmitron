#pragma once

#include <wx/event.h>

namespace Rapatas::Transmitron::GUI::Events
{

class TopicCtrl;
wxDECLARE_EVENT(TOPICCTRL_RETURN, TopicCtrl);

// NOLINTNEXTLINE
class TopicCtrl :
  public wxCommandEvent
{
public:

  TopicCtrl(wxEventType commandType, int id = 0) :
    wxCommandEvent(commandType, id)
  {}

  TopicCtrl(const TopicCtrl& event) :
    wxCommandEvent(event)
  {}

  wxEvent* Clone() const override
  {
    return new TopicCtrl(*this);
  }
};

} // namespace Rapatas::Transmitron::GUI::Events

#ifndef TRANSMITRON_EVENTS_TOPICCTRL_HPP
#define TRANSMITRON_EVENTS_TOPICCTRL_HPP

#include <wx/event.h>

namespace Transmitron::Events
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

}

#endif // TRANSMITRON_EVENTS_TOPICCTRL_HPP

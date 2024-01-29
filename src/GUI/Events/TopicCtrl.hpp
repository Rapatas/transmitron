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

  explicit TopicCtrl(wxEventType commandType, int id = 0) :
    wxCommandEvent(commandType, id)
  {}

  TopicCtrl(const TopicCtrl& event) = default;

  [[nodiscard]] wxEvent* Clone() const override
  {
    return new TopicCtrl(*this);
  }
};

} // namespace Rapatas::Transmitron::GUI::Events

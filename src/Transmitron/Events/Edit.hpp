#ifndef TRANSMITRON_EVENTS_EDIT_HPP
#define TRANSMITRON_EVENTS_EDIT_HPP

#include <wx/event.h>

namespace Transmitron::Events
{

class Edit;
wxDECLARE_EVENT(EDIT_PUBLISH, Edit);
wxDECLARE_EVENT(EDIT_SAVE_SNIPPET, Edit);

class Edit : public wxCommandEvent
{
public:
  Edit(wxEventType commandType, int id = 0) :
    wxCommandEvent(commandType, id)
  {}

  Edit(const Edit& event) :
    wxCommandEvent(event)
  {}

  wxEvent* Clone() const override
  {
    return new Edit(*this);
  }
};

}

#endif // TRANSMITRON_EVENTS_EDIT_HPP

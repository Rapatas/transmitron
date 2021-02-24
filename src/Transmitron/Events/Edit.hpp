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

  wxEvent* Clone() const
  {
    return new Edit(*this);
  }
};

typedef void (wxEvtHandler::*EditFunction)(Edit &);
#define \
  EditHandler(func) \
  wxEVENT_HANDLER_CAST(EditFunction, func)

}

#endif // TRANSMITRON_EVENTS_EDIT_HPP

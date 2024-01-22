#pragma once

#include <wx/event.h>

namespace Rapatas::Transmitron::GUI::Events
{

class Edit;
wxDECLARE_EVENT(EDIT_PUBLISH, Edit);
wxDECLARE_EVENT(EDIT_SAVE_MESSAGE, Edit);

// NOLINTNEXTLINE
class Edit :
  public wxCommandEvent
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

} // namespace Rapatas::Transmitron::GUI::Events

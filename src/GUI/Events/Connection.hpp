#pragma once

#include <wx/dataview.h>
#include <wx/event.h>

namespace Rapatas::Transmitron::GUI::Events
{

class Connection;
wxDECLARE_EVENT(CONNECTION_REQUESTED, Connection);
wxDECLARE_EVENT(CONNECTION_CONNECTED, Connection);
wxDECLARE_EVENT(CONNECTION_DISCONNECTED, Connection);
wxDECLARE_EVENT(CONNECTION_FAILURE, Connection);
wxDECLARE_EVENT(CONNECTION_LOST, Connection);

// NOLINTNEXTLINE
class Connection :
  public wxCommandEvent
{
public:

  Connection(wxEventType commandType = CONNECTION_REQUESTED, int id = 0) :
    wxCommandEvent(commandType, id)
  {}

  Connection(const Connection& event) :
    wxCommandEvent(event)
  {
    this->setProfile(event.getProfile());
  }

  [[nodiscard]] wxEvent* Clone() const override
  {
    return new Connection(*this);
  }

  [[nodiscard]] wxDataViewItem getProfile() const
  {
    return mProfile;
  }

  void setProfile(wxDataViewItem profile)
  {
    mProfile = profile;
  }

private:

  wxDataViewItem mProfile;
};

} // namespace Rapatas::Transmitron::GUI::Events

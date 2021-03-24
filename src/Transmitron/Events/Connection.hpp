#ifndef TRANSMITRON_EVENTS_PROFILE_HPP
#define TRANSMITRON_EVENTS_PROFILE_HPP

#include <wx/dataview.h>
#include <wx/event.h>

namespace Transmitron::Events
{

class Connection;
wxDECLARE_EVENT(CONNECTION, Connection);
wxDECLARE_EVENT(CONNECTED, Connection);
wxDECLARE_EVENT(DISCONNECTED, Connection);

// NOLINTNEXTLINE
class Connection :
  public wxCommandEvent
{
public:

  Connection(wxEventType commandType = CONNECTION, int id = 0) :
    wxCommandEvent(commandType, id)
  {}

  Connection(const Connection& event) :
    wxCommandEvent(event)
  {
    this->setProfile(event.getProfile());
  }

  wxEvent* Clone() const override
  {
    return new Connection(*this);
  }

  wxDataViewItem getProfile() const
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

}

#endif // TRANSMITRON_EVENTS_PROFILE_HPP

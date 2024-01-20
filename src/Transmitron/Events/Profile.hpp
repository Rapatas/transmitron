#ifndef TRANSMITRON_EVENTS_PROFILE_HPP
#define TRANSMITRON_EVENTS_PROFILE_HPP

#include <wx/dataview.h>
#include <wx/event.h>

namespace Transmitron::Events
{

class Profile;
wxDECLARE_EVENT(PROFILE_CREATE, Profile);
wxDECLARE_EVENT(PROFILE_EDIT, Profile);

// NOLINTNEXTLINE
class Profile :
  public wxCommandEvent
{
public:

  Profile(wxEventType commandType, int id = 0) :
    wxCommandEvent(commandType, id)
  {}

  Profile(const Profile& event) :
    wxCommandEvent(event)
  {
    this->setProfile(event.getProfile());
  }

  wxEvent* Clone() const override
  {
    return new Profile(*this);
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

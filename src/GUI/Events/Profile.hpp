#pragma once

#include <wx/dataview.h>
#include <wx/event.h>

namespace Rapatas::Transmitron::GUI::Events
{

class Profile;
wxDECLARE_EVENT(PROFILE_CREATE, Profile);
wxDECLARE_EVENT(PROFILE_EDIT, Profile);

// NOLINTNEXTLINE
class Profile :
  public wxCommandEvent
{
public:

  explicit Profile(wxEventType commandType, int id = 0) :
    wxCommandEvent(commandType, id)
  {}

  Profile(const Profile& event) :
    wxCommandEvent(event)
  {
    this->setProfile(event.getProfile());
  }

  [[nodiscard]] wxEvent* Clone() const override
  {
    return new Profile(*this);
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

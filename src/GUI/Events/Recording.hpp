#pragma once

#include <wx/dataview.h>
#include <wx/event.h>

namespace Rapatas::Transmitron::GUI::Events {

class Recording;
wxDECLARE_EVENT(RECORDING_SAVE, Recording);
wxDECLARE_EVENT(RECORDING_OPEN, Recording);

// NOLINTNEXTLINE
class Recording : public wxCommandEvent
{
public:

  explicit Recording(wxEventType commandType, int id = 0) :
    wxCommandEvent(commandType, id) //
  {}

  Recording(const Recording &event) :
    wxCommandEvent(event) {
    this->setContents(event.getContents());
    this->setName(event.getName());
  }

  [[nodiscard]] wxEvent *Clone() const override { return new Recording(*this); }

  [[nodiscard]] std::string getContents() const { return mContents; }

  void setContents(const std::string &contents) { mContents = contents; }

  [[nodiscard]] wxString getName() const { return mName; }

  void setName(const wxString &name) { mName = name; }

private:

  std::string mContents;
  wxString mName;
};

} // namespace Rapatas::Transmitron::GUI::Events

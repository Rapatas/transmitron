#pragma once

#include <wx/event.h>
#include <wx/dataview.h>

namespace Rapatas::Transmitron::GUI::Events
{

class Layout;
wxDECLARE_EVENT(LAYOUT_SELECTED, Layout);
wxDECLARE_EVENT(LAYOUT_ADDED,    Layout);
wxDECLARE_EVENT(LAYOUT_REMOVED,  Layout);
wxDECLARE_EVENT(LAYOUT_CHANGED,  Layout);
wxDECLARE_EVENT(LAYOUT_RESIZED,  Layout);

// NOLINTNEXTLINE
class Layout :
  public wxCommandEvent
{
public:

  explicit Layout(wxEventType commandType, int id = 0) :
    wxCommandEvent(commandType, id)
  {}

  Layout(const Layout& event) :
    wxCommandEvent(event)
  {}

  [[nodiscard]] wxEvent* Clone() const override
  {
    return new Layout(*this);
  }

  void setPerspective(std::string perspective)
  {
    mPerspective = std::move(perspective);
  }

  [[nodiscard]] std::string getPerspective() const
  {
    return mPerspective;
  }

  [[nodiscard]] wxDataViewItem getItem() const
  {
    return mItem;
  }

  void setItem(wxDataViewItem item)
  {
    mItem = item;
  }

private:

  wxDataViewItem mItem;
  std::string mPerspective;

};

} // namespace Rapatas::Transmitron::GUI::Events

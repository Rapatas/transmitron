#ifndef TRANSMITRON_EVENTS_LAYOUT_HPP
#define TRANSMITRON_EVENTS_LAYOUT_HPP

#include <wx/event.h>
#include <wx/dataview.h>

namespace Transmitron::Events
{

class Layout;
wxDECLARE_EVENT(LAYOUT_SELECTED, Layout);
wxDECLARE_EVENT(LAYOUT_ADDED,    Layout);
wxDECLARE_EVENT(LAYOUT_REMOVED,  Layout);
wxDECLARE_EVENT(LAYOUT_CHANGED,  Layout);

// NOLINTNEXTLINE
class Layout :
  public wxCommandEvent
{
public:

  Layout(wxEventType commandType, int id = 0) :
    wxCommandEvent(commandType, id)
  {}

  Layout(const Layout& event) :
    wxCommandEvent(event)
  {}

  wxEvent* Clone() const override
  {
    return new Layout(*this);
  }

  void setPerspective(std::string perspective)
  {
    mPerspective = std::move(perspective);
  }

  std::string getPerspective() const
  {
    return mPerspective;
  }

  wxDataViewItem getItem() const
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

}

#endif // TRANSMITRON_EVENTS_LAYOUT_HPP

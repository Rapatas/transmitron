#ifndef TRANSMITRON_WIDGETS_CONTAINER_HPP
#define TRANSMITRON_WIDGETS_CONTAINER_HPP

#include <wx/panel.h>
#include <wx/sizer.h>

namespace Transmitron::Widgets
{

class Container :
  public wxPanel
{
public:

  Container(wxPanel *parent);

  void contain(wxPanel *contained);

private:

  wxPanel *mContained = nullptr;
  wxBoxSizer *mSizer = nullptr;

  void onResized(wxSizeEvent& event);
  void onResized(wxSize size);

};

} // namespace Transmitron::Widgets

#endif // TRANSMITRON_WIDGETS_CONTAINER_HPP

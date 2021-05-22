#ifndef TRANSMITRON_NOTIFIERS_LAYOUTS_HPP
#define TRANSMITRON_NOTIFIERS_LAYOUTS_HPP

#include <wx/dataview.h>

namespace Transmitron::Notifiers
{

class Layouts :
  public wxEvtHandler,
  public wxDataViewModelNotifier
{
public:

private:

  // wxDataViewModelNotifier interface.
  bool Cleared() override;
  bool ItemChanged(const wxDataViewItem &item) override;
  void Resort() override;
  bool ValueChanged(const wxDataViewItem &item, unsigned int col) override;
  bool ItemAdded(
    const wxDataViewItem &parent,
    const wxDataViewItem &item
  ) override;
  bool ItemDeleted(
    const wxDataViewItem &parent,
    const wxDataViewItem &item
  ) override;

};

}

#endif // TRANSMITRON_NOTIFIERS_LAYOUTS_HPP

#pragma once

#include <wx/dataview.h>

namespace Rapatas::Transmitron::GUI::Notifiers
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

} // namespace Rapatas::Transmitron::GUI::Notifiers


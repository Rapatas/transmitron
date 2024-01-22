#include "Layouts.hpp"

#include "GUI/Events/Layout.hpp"

using namespace Rapatas::Transmitron;
using namespace GUI::Notifiers;
using namespace GUI;

// wxDataViewModelNotifier interface {

bool Layouts::Cleared()
{
  return true;
}

bool Layouts::ItemChanged(const wxDataViewItem &item)
{
  auto *e = new Events::Layout(Events::LAYOUT_CHANGED);
  e->setItem(item);
  wxQueueEvent(this, e);
  return true;
}

void Layouts::Resort()
{}

bool Layouts::ValueChanged(
  const wxDataViewItem &/* item */,
  unsigned int /* col */
) {
  return true;
}

bool Layouts::ItemAdded(
  const wxDataViewItem &/* parent */,
  const wxDataViewItem &item
) {
  auto *e = new Events::Layout(Events::LAYOUT_ADDED);
  e->setItem(item);
  wxQueueEvent(this, e);
  return true;
}

bool Layouts::ItemDeleted(
  const wxDataViewItem &/* parent */,
  const wxDataViewItem &/* item */
) {
  auto *e = new Events::Layout(Events::LAYOUT_REMOVED);
  wxQueueEvent(this, e);
  return true;
}

// wxDataViewModelNotifier interface }

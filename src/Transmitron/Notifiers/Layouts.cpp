#include "Layouts.hpp"

#include <wx/log.h>

#include "Transmitron/Events/Layout.hpp"

#define wxLOG_COMPONENT "Notifiers/Layout" // NOLINT

using namespace Transmitron::Notifiers;
using namespace Transmitron;

wxDEFINE_EVENT(Events::LAYOUT_ADDED,   Events::Layout); // NOLINT
wxDEFINE_EVENT(Events::LAYOUT_REMOVED, Events::Layout); // NOLINT
wxDEFINE_EVENT(Events::LAYOUT_CHANGED, Events::Layout); // NOLINT

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

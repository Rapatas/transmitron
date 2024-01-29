#include "GUI/Events/Layout.hpp"

using namespace Rapatas::Transmitron::GUI;

// NOLINTBEGIN(cert-err58-cpp)
wxDEFINE_EVENT(Events::LAYOUT_SELECTED, Events::Layout);
wxDEFINE_EVENT(Events::LAYOUT_ADDED, Events::Layout);
wxDEFINE_EVENT(Events::LAYOUT_REMOVED, Events::Layout);
wxDEFINE_EVENT(Events::LAYOUT_CHANGED, Events::Layout);
wxDEFINE_EVENT(Events::LAYOUT_RESIZED, Events::Layout);
// NOLINTEND(cert-err58-cpp)

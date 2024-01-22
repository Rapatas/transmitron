#include "GUI/Events/Connection.hpp"

using namespace Rapatas::Transmitron::GUI;

// NOLINTBEGIN(cert-err58-cpp)
wxDEFINE_EVENT(Events::CONNECTION_REQUESTED, Events::Connection);
wxDEFINE_EVENT(Events::CONNECTION_CONNECTED, Events::Connection);
wxDEFINE_EVENT(Events::CONNECTION_DISCONNECTED, Events::Connection);
wxDEFINE_EVENT(Events::CONNECTION_FAILURE, Events::Connection);
wxDEFINE_EVENT(Events::CONNECTION_LOST, Events::Connection);
// NOLINTEND(cert-err58-cpp)

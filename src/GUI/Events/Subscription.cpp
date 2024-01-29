#include "GUI/Events/Subscription.hpp"

using namespace Rapatas::Transmitron::GUI;

// NOLINTBEGIN(cert-err58-cpp)
wxDEFINE_EVENT(Events::SUBSCRIPTION_SUBSCRIBED, Events::Subscription);
wxDEFINE_EVENT(Events::SUBSCRIPTION_UNSUBSCRIBED, Events::Subscription);
wxDEFINE_EVENT(Events::SUBSCRIPTION_RECEIVED, Events::Subscription);
// NOLINTEND(cert-err58-cpp)

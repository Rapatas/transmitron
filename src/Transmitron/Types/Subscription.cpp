#include "Subscription.hpp"
#include "Transmitron/Events/Subscription.hpp"

using namespace Transmitron::Types;
using namespace Transmitron;

wxDEFINE_EVENT(Events::SUBSCRIBED, Events::Subscription);
wxDEFINE_EVENT(Events::UNSUBSCRIBED, Events::Subscription);
wxDEFINE_EVENT(Events::RECEIVED, Events::Subscription);

Subscription::Subscription(std::shared_ptr<MQTT::Subscription> sub) :
  mSub(sub),
  mMuted(false)
{
  mColor = colorFromString(sub->getFilter());
  sub->attachObserver(this);
}

void Subscription::onSubscribed()
{
  auto e = new Events::Subscription(Events::SUBSCRIBED);
  e->setId(mSub->getId());
  wxQueueEvent(this, e);
}

void Subscription::onUnsubscribed()
{
  auto e = new Events::Subscription(Events::UNSUBSCRIBED);
  e->setId(mSub->getId());
  wxQueueEvent(this, e);
}

void Subscription::onMessage(mqtt::const_message_ptr msg)
{
  auto e = new Events::Subscription(Events::RECEIVED);
  e->setMessage(msg);
  e->setId(mSub->getId());
  wxQueueEvent(this, e);
}

size_t Subscription::getId() const
{
  return mSub->getId();
}

void Subscription::setMuted(bool muted)
{
  mMuted = muted;
}

void Subscription::setColor(const wxColor &color)
{
  mColor = color;
}

void Subscription::unsubscribe()
{
  mSub->unsubscribe();
}

std::string Subscription::getFilter() const
{
  return mSub->getFilter();
}

wxColor Subscription::getColor() const
{
  return mColor;
}

MQTT::QoS Subscription::getQos() const
{
  return mSub->getQos();
}

bool Subscription::getMuted() const
{
  return mMuted;
}

wxColor Subscription::colorFromString(const std::string &data)
{
  constexpr uint8_t MinColorChannel = 100;
  const size_t x = std::hash<std::string>{}(data);
  uint8_t r = ((x >> 0)  & 0xFF);
  uint8_t g = ((x >> 8)  & 0xFF);
  uint8_t b = ((x >> 16) & 0xFF);
  if (r < MinColorChannel) { r = (uint8_t)(r + MinColorChannel); }
  if (g < MinColorChannel) { g = (uint8_t)(r + MinColorChannel); }
  if (b < MinColorChannel) { b = (uint8_t)(r + MinColorChannel); }
  return wxColor(r, g, b);
}

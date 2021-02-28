#include "SubscriptionData.hpp"
#include "Transmitron/Events/Subscription.hpp"

using namespace Transmitron::Types;
using namespace Transmitron;

wxDEFINE_EVENT(Events::SUBSCRIBED, Events::Subscription);
wxDEFINE_EVENT(Events::UNSUBSCRIBED, Events::Subscription);
wxDEFINE_EVENT(Events::RECEIVED, Events::Subscription);

SubscriptionData::SubscriptionData(std::shared_ptr<MQTT::Subscription> sub) :
  mSub(sub),
  mMuted(false)
{
  mColor = colorFromString(sub->getFilter());
  sub->attachObserver(this);
}

void SubscriptionData::onSubscribed()
{
  auto e = new Events::Subscription(Events::SUBSCRIBED);
  e->setSubscription(this);
  wxQueueEvent(this, e);
}

void SubscriptionData::onUnsubscribed()
{
  auto e = new Events::Subscription(Events::UNSUBSCRIBED);
  e->setSubscription(this);
  wxQueueEvent(this, e);
}

void SubscriptionData::onMessage(mqtt::const_message_ptr msg)
{
  auto e = new Events::Subscription(Events::RECEIVED);
  e->setMessage(msg);
  e->setSubscription(this);
  wxQueueEvent(this, e);
}

size_t SubscriptionData::getId() const
{
  return mSub->getId();
}

void SubscriptionData::setMuted(bool muted)
{
  mMuted = muted;
}

void SubscriptionData::setColor(const wxColor &color)
{
  mColor = color;
}

void SubscriptionData::unsubscribe()
{
  mSub->unsubscribe();
}

std::string SubscriptionData::getFilter() const
{
  return mSub->getFilter();
}

wxColor SubscriptionData::getColor() const
{
  return mColor;
}

MQTT::QoS SubscriptionData::getQos() const
{
  return mSub->getQos();
}

bool SubscriptionData::getMuted() const
{
  return mMuted;
}

wxColor SubscriptionData::colorFromString(const std::string &data)
{
  size_t x = std::hash<std::string>{}(data);
  uint8_t r = ((x >> 0)  & 0xFF);
  uint8_t g = ((x >> 8)  & 0xFF);
  uint8_t b = ((x >> 16) & 0xFF);
  r = (r - 0) / (float)(255 - 0) * (255 - 100) + 100;
  g = (g - 0) / (float)(255 - 0) * (255 - 100) + 100;
  b = (b - 0) / (float)(255 - 0) * (255 - 100) + 100;
  return wxColor(r, g, b);
}

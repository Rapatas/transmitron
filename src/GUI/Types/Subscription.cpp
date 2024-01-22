#include "Subscription.hpp"
#include "Common/Helpers.hpp"
#include "GUI/Events/Subscription.hpp"

using namespace Rapatas::Transmitron;
using namespace GUI::Types;
using namespace GUI;

Subscription::Subscription(const std::shared_ptr<MQTT::Subscription> &sub) :
  mSub(sub),
  mMuted(false),
  mId(mSub->getId()),
  mFilter(mSub->getFilter()),
  mQos(mSub->getQos()),
  mColor(colorFromString(mFilter))
{
  sub->attachObserver(this);
}

Subscription::Subscription(
  MQTT::Subscription::Id_t id,
  std::string filter,
  MQTT::QoS qos
) :
  mSub(nullptr),
  mMuted(false),
  mId(id),
  mFilter(std::move(filter)),
  mQos(qos),
  mColor(colorFromString(mFilter))
{}

void Subscription::onSubscribed()
{
  auto *e = new Events::Subscription(Events::SUBSCRIPTION_SUBSCRIBED);
  e->setId(mId);
  wxQueueEvent(this, e);
}

void Subscription::onUnsubscribed()
{
  auto *e = new Events::Subscription(Events::SUBSCRIPTION_UNSUBSCRIBED);
  e->setId(mId);
  wxQueueEvent(this, e);
}

void Subscription::onMessage(const MQTT::Message &message)
{
  auto *e = new Events::Subscription(Events::SUBSCRIPTION_RECEIVED);
  e->setMessage(message);
  e->setId(mId);
  wxQueueEvent(this, e);
}

size_t Subscription::getId() const
{
  return mId;
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
  if (mSub == nullptr) { return; }
  mSub->unsubscribe();
}

std::string Subscription::getFilter() const
{
  return mFilter;
}

wxColor Subscription::getColor() const
{
  return mColor;
}

MQTT::QoS Subscription::getQos() const
{
  return mQos;
}

bool Subscription::getMuted() const
{
  return mMuted;
}

wxColor Subscription::colorFromString(const std::string &data)
{
  const size_t x = std::hash<std::string>{}(data);
  return Common::Helpers::colorFromNumber(x);
}

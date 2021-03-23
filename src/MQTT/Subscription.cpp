#include "Subscription.hpp"

#define wxLOG_COMPONENT "mqtt/subscription" // NOLINT

using namespace MQTT;

Subscription::Subscription(
  Id_t id,
  const std::string &filter,
  QoS qos,
  std::shared_ptr<Client> client
) :
  mId(id),
  mFilter(filter),
  mQos(qos),
  mState(State::Unsubscribed),
  mClient(std::move(client))
{}

size_t Subscription::attachObserver(Observer *o)
{
  size_t id = 0;
  do {
    id = (size_t)std::abs(rand());
  } while (mObservers.find(id) != std::end(mObservers));

  if (mState == State::Subscribed)
  {
    onSubscribed();
  }

  return mObservers.insert(std::make_pair(id, o)).first->first;
}

void Subscription::unsubscribe()
{
  if (
    mState == State::Subscribed
    || mState == State::PendingSubscription
  ) {
    mClient->unsubscribe(mId);
  }
}

void Subscription::onSubscribed()
{
  for (const auto &o : mObservers)
  {
    o.second->onSubscribed();
  }
}

void Subscription::onUnsubscribed()
{
  for (const auto &o : mObservers)
  {
    o.second->onUnsubscribed();
  }
}

void Subscription::onMessage(
  const mqtt::const_message_ptr &msg
) {
  for (const auto &o : mObservers)
  {
    o.second->onMessage(msg);
  }
}

std::string Subscription::getFilter() const
{
  return mFilter;
}

QoS Subscription::getQos() const
{
  return mQos;
}

Subscription::State Subscription::getState() const
{
  return mState;
}

Subscription::Id_t Subscription::getId() const
{
  return mId;
}

void Subscription::setState(State newState)
{
  mState = newState;
}

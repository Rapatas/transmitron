#ifndef MQTT_SUBSCRIPTION_HPP
#define MQTT_SUBSCRIPTION_HPP

#include <functional>
#include <string>
#include <map>

#include <mqtt/async_client.h>

#include "Client.hpp"

namespace MQTT
{

class Subscription :
  public std::enable_shared_from_this<Subscription>
{
public:

  using Id_t = size_t;

  enum class State
  {
    PendingSubscription,
    Subscribed,
    PendingUnsubscription,
    Unsubscribed
  };

  struct Observer
  {
    Observer() = default;
    Observer(const Observer &other) = default;
    Observer(Observer &&other) = default;
    Observer &operator=(const Observer &other) = default;
    Observer &operator=(Observer &&other) = default;
    virtual ~Observer() = default;
    virtual void onSubscribed() = 0;
    virtual void onUnsubscribed() = 0;
    virtual void onMessage(mqtt::const_message_ptr msg) = 0;
  };

  explicit Subscription(
    Id_t id,
    const std::string &filter,
    QoS qos,
    std::shared_ptr<Client> client
  );

  size_t attachObserver(Observer *observer);

  void unsubscribe();

  void onMessage(const mqtt::const_message_ptr &msg);
  void onUnsubscribed();
  void onSubscribed();

  std::string getFilter() const;
  State getState() const;
  QoS getQos() const;
  Id_t getId() const;

private:

  Id_t mId;
  std::string mFilter;
  QoS mQos;
  State mState;

  std::shared_ptr<Client> mClient;
  std::map<size_t, MQTT::Subscription::Observer*> mObservers;

  friend class Client; // Can set the state.
  void setState(State newState);
};

}

#endif // MQTT_SUBSCRIPTION_HPP

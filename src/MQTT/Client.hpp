#ifndef MQTT_CLIENT_HPP
#define MQTT_CLIENT_HPP

#include <list>
#include <map>
#include <memory>

#include <mqtt/async_client.h>
#include <mqtt/disconnect_options.h>
#include <spdlog/spdlog.h>

#include "QualityOfService.hpp"
#include "BrokerOptions.hpp"
#include "Message.hpp"

namespace MQTT
{

class Subscription;

class Client :
  public virtual mqtt::iaction_listener,
  public virtual mqtt::callback,
  public std::enable_shared_from_this<Client>
{
public:

  using SubscriptionId = size_t;

  struct Observer
  {
    Observer() = default;
    Observer(const Observer &other) = default;
    Observer(Observer &&other) = default;
    Observer &operator=(const Observer &other) = default;
    Observer &operator=(Observer &&other) = default;
    virtual ~Observer() = default;

    virtual void onConnected() = 0;
    virtual void onDisconnected() = 0;
    virtual void onConnectionFailure() = 0;
    virtual void onConnectionLost() = 0;
  };

  explicit Client();
  size_t attachObserver(Observer *o);
  void detachObserver(size_t id);

  // Actions.
  void connect();
  void disconnect();
  void cancel();
  void publish(const Message &message);
  std::shared_ptr<Subscription> subscribe(const std::string &topic);
  void unsubscribe(size_t id);

  // Setters.
  void setBrokerOptions(BrokerOptions brokerOptions);

  // Getters.
  const BrokerOptions &brokerOptions() const;
  bool connected() const;

private:

  const std::map<int, std::string> mReturnCodes
  {
    { 0,   "Connection accepted" },
    { 1,   "Unacceptable protocol version" },
    { 2,   "Identifier rejected" },
    { 3,   "Service unavailable" },
    { 4,   "Bad user name or password" },
    { 5,   "Not authorized" },
    { -1,  "Connection timeout" },
    { -2,  "Persistence error" },
    { -3,  "Disconnected" },
    { -4,  "Max messages inflight" },
    { -5,  "Bad utf8 string" },
    { -6,  "Null parameter" },
    { -7,  "Topicname truncated" },
    { -8,  "Bad structure" },
    { -9,  "Bad QOS" },
    { -10, "No more MsgIds" },
    { -11, "Operation incomplete" },
    { -12, "Max buffered messages" },
    { -13, "Ssl not supported" },
    { -14, "Bad protocol" },
    { -15, "Bad MQTT option" },
    { -16, "Wrong MQTT version" },
    { -17, "Zero length will topic" },
    { -18, "Command ignored" },
  };

  std::shared_ptr<spdlog::logger> mLogger;
  BrokerOptions mBrokerOptions;
  SubscriptionId mSubscriptionIds = 0;
  bool mShouldReconnect = false;
  bool mCanceled = false;
  mqtt::connect_options mConnectOptions;
  size_t mRetries = 0;
  std::map<SubscriptionId, std::shared_ptr<Subscription>> mSubscriptions;
  std::map<size_t, MQTT::Client::Observer*> mObservers;
  std::shared_ptr<mqtt::async_client> mClient;

  // mqtt::iaction_listener interface.
  void on_success(const mqtt::token& tok) override;
  void on_failure(const mqtt::token& tok) override;

  // mqtt::callback interface.
  void connected(const std::string& cause) override;
  void connection_lost(const std::string& cause) override;
  void message_arrived(mqtt::const_message_ptr msg) override;
  void delivery_complete(mqtt::delivery_token_ptr token) override ;

  void onSuccessConnect(const mqtt::token& tok);
  void onSuccessDisconnect(const mqtt::token& tok);
  void onSuccessPublish(const mqtt::token& tok);
  void onSuccessSubscribe(const mqtt::token& tok);
  void onSuccessUnsubscribe(const mqtt::token& tok);
  void onFailureConnect(const mqtt::token& tok);
  void onFailureDisconnect(const mqtt::token& tok);
  void onFailurePublish(const mqtt::token& tok);
  void onFailureSubscribe(const mqtt::token& tok);
  void onFailureUnsubscribe(const mqtt::token& tok);

  void reconnect();
  void doSubscribe(size_t id);
  void cleanSubscriptions();

  static bool match(const std::string &filter, const std::string &topic);
};

}

#endif // MQTT_CLIENT_HPP

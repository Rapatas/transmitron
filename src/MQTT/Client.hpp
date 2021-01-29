#ifndef MQTT_CLIENT_HPP
#define MQTT_CLIENT_HPP

#include <memory>
#include <map>
#include <list>
#include <mqtt/async_client.h>
#include "QualityOfService.hpp"

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
    virtual ~Observer() = default;
    virtual void onConnected() = 0;
    virtual void onDisconnected() = 0;
  };

  explicit Client();
  virtual ~Client();

  void connect();
  void disconnect();
  void reconnect();

  size_t attachObserver(Observer *o);

  void setHostname(const std::string &hostname);
  void setPort(unsigned port);
  void setId(const std::string &id);

  void publish(
    const std::string &topic,
    const std::string &payload,
    QoS qos,
    bool retained = false
  );

  std::shared_ptr<Subscription> subscribe(const std::string &topic);
  void unsubscribe(size_t id);

  std::string hostname() const;
  unsigned port() const;
  bool connected() const;
  std::string id() const;

private:

  const std::map<int, std::string> returnCodes
  {
    { 0,  "Connection accepted" },
    { 1,  "Unacceptable protocol version" },
    { 2,  "Identifier rejected" },
    { 3,  "Service unavailable" },
    { 4,  "Bad user name or password" },
    { 5,  "Not authorized" },
    { -1, "Connection timeout" }
  };

  mqtt::connect_options mOptions;
  std::string mHostname;
  std::string mId;
  unsigned mPort;
  unsigned mRetries;
  unsigned mRetriesMax;

  SubscriptionId mSubscriptionIds = 0;
  std::map<SubscriptionId, std::shared_ptr<Subscription>> mSubscriptions;
  std::map<size_t, MQTT::Client::Observer*> mObservers;
  std::shared_ptr<mqtt::async_client> mClient;

  // mqtt::iaction_listener interface.
  void on_success(const mqtt::token& tok) override;
  void on_success_connect(const mqtt::token& tok);
  void on_success_disconnect(const mqtt::token& tok);
  void on_success_publish(const mqtt::token& tok);
  void on_success_subscribe(const mqtt::token& tok);
  void on_success_unsubscribe(const mqtt::token& tok);
  void on_failure(const mqtt::token& tok) override;
  void on_failure_connect(const mqtt::token& tok);
  void on_failure_disconnect(const mqtt::token& tok);
  void on_failure_publish(const mqtt::token& tok);
  void on_failure_subscribe(const mqtt::token& tok);
  void on_failure_unsubscribe(const mqtt::token& tok);

  // mqtt::callback interface.
  void connected(const std::string& cause) override;
  void connection_lost(const std::string& cause) override;
  void message_arrived(mqtt::const_message_ptr msg) override;
  void delivery_complete(mqtt::delivery_token_ptr token) override ;

  void doSubscribe(size_t id);
  void cleanSubscriptions();

  static bool match(const std::string &filter, const std::string &topic);
};

}

#endif // MQTT_CLIENT_HPP

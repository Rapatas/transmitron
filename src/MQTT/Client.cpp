#include "Client.hpp"
#include <wx/log.h>
#include <thread>
#include "Subscription.hpp"

#define wxLOG_COMPONENT "mqtt/client"

using namespace MQTT;

Client::Client() :
  mHostname("0.0.0.0"),
  mPort(1883),
  mRetries(0),
  mRetriesMax(10)
{
  mOptions.set_clean_session(true);
  mOptions.set_keep_alive_interval(20);
  mOptions.set_connect_timeout(5);
  mId = "client" + std::to_string(rand() % 100);
}

Client::~Client() {}

void Client::connect()
{
  std::string address = mHostname + ':' + std::to_string(mPort);
  wxLogMessage("Connecting to %s", address);
  try
  {
    mClient = std::make_shared<mqtt::async_client>(address, mId);
    mClient->set_callback(*this);
    wxLogMessage("Username: %s", mOptions.get_user_name());
    wxLogMessage("Password: %s", mOptions.get_password_str());
    mClient->connect(mOptions, nullptr, *this);
  }
  catch (const mqtt::exception& e)
  {
    wxLogError("Connection failed: %s", e.what());
    exit(1);
  }
}

void Client::disconnect()
{
  wxLogMessage("Disconnecting from %s", mHostname);
  try
  {
    mClient->disconnect(200, nullptr, *this);
    cleanSubscriptions();
  }
  catch (const mqtt::exception& exc)
  {
    wxLogError("Disconnection failed: %s", exc.what());
    exit(1);
  }
}

void Client::reconnect()
{
  std::this_thread::sleep_for(std::chrono::milliseconds(2500));
  wxLogMessage("Reconnecting...");
  try
  {
    mClient->connect(mOptions, nullptr, *this);
  }
  catch (const mqtt::exception& exc)
  {
    std::cerr << "Error: " << exc.what() << std::endl;
    exit(1);
  }
}

size_t Client::attachObserver(Observer *o)
{
  size_t id = 0;
  do {
    id = rand();
  } while (mObservers.find(id) != std::end(mObservers));

  return mObservers.insert(std::make_pair(id, o)).first->first;
}

void Client::publish(
  const std::string &topic,
  const std::string &payload,
  QoS qos,
  bool retained
) {
  mClient->publish(
    topic,
    payload.data(),
    payload.size(),
    (int)qos,
    retained,
    nullptr,
    *this
  );
}

void Client::setHostname(const std::string &hostname)
{
  if (connected()) { return; }
  mHostname = hostname;
}

void Client::setPort(unsigned port)
{
  if (connected()) { return; }
  mPort = port;
}

void Client::setId(const std::string &id)
{
  if (connected()) { return; }
  mId = id;
}

void Client::setUsername(const std::string &username)
{
  if (connected()) { return; }
  mOptions.set_user_name(username);
}

void Client::setPassword(const std::string &password)
{
  if (connected()) { return; }
  mOptions.set_password(password);
}

std::string Client::hostname() const
{
  return mHostname;
}

unsigned Client::port() const
{
  return mPort;
}

std::string Client::id() const
{
  return mId;
}

bool Client::connected() const
{
  return mClient && mClient->is_connected();
}

std::shared_ptr<Subscription> Client::subscribe(const std::string &topic)
{
  auto it = std::find_if(
    std::begin(mSubscriptions),
    std::end(mSubscriptions),
    [topic](const auto &sub)
    {
      return sub.second->getFilter() == topic;
    }
  );

  if (it != std::end(mSubscriptions))
  {
    wxLogMessage("Already subscribed!");
    return it->second;
  }

  wxLogMessage("Creating subscription");
  ++mSubscriptionIds;
  auto sub = std::make_shared<Subscription>(
    mSubscriptionIds,
    topic,
    QoS::ExactlyOnce,
    shared_from_this()
  );
  sub->setState(Subscription::State::Unsubscribed);
  mSubscriptions.insert({mSubscriptionIds, sub});

  wxLogMessage("Checking if connected");
  if (connected())
  {
    doSubscribe(mSubscriptionIds);
  }

  return sub;
}

void Client::unsubscribe(size_t id) {
  auto it = mSubscriptions.find(id);
  if (it == std::end(mSubscriptions)) { return; }
  it->second->setState(Subscription::State::PendingUnsubscription);
  wxLogMessage("Unsubscribing from %s", it->second->getFilter());
  mClient->unsubscribe(it->second->getFilter(), nullptr, *this);
}

// mqtt::iaction_listener interface {

void Client::on_failure(const mqtt::token& tok)
{
  switch (tok.get_type()) {
    case mqtt::delivery_token::Type::CONNECT: {
      on_failure_connect(tok);
    } break;
    case mqtt::delivery_token::Type::DISCONNECT: {
      on_failure_disconnect(tok);
    } break;
    case mqtt::delivery_token::Type::PUBLISH: {
      on_failure_publish(tok);
    } break;
    case mqtt::delivery_token::Type::SUBSCRIBE: {
      on_failure_subscribe(tok);
    } break;
    case mqtt::delivery_token::Type::UNSUBSCRIBE: {
      on_failure_unsubscribe(tok);
    } break;
  }
}

void Client::on_success(const mqtt::token& tok)
{
  switch (tok.get_type()) {
    case mqtt::delivery_token::Type::CONNECT: {
      on_success_connect(tok);
    } break;
    case mqtt::delivery_token::Type::DISCONNECT: {
      on_success_disconnect(tok);
    } break;
    case mqtt::delivery_token::Type::PUBLISH: {
      on_success_publish(tok);
    } break;
    case mqtt::delivery_token::Type::SUBSCRIBE: {
      on_success_subscribe(tok);
    } break;
    case mqtt::delivery_token::Type::UNSUBSCRIBE: {
      on_success_unsubscribe(tok);
    } break;
  }
}

void Client::on_success_connect(const mqtt::token& tok)
{
  for (const auto &o : mObservers)
  {
    o.second->onConnected();
  }
}

void Client::on_success_disconnect(const mqtt::token& tok)
{
  for (const auto &o : mObservers)
  {
    o.second->onDisconnected();
  }
}

void Client::on_success_publish(const mqtt::token& tok)
{
  wxLogMessage("Published!!");
}

void Client::on_success_subscribe(const mqtt::token& tok)
{
  auto it = std::find_if(
    std::begin(mSubscriptions),
    std::end(mSubscriptions),
    [&tok](const auto &sub)
    {
      return sub.second->getFilter() == tok.get_topics()->operator[](0);
    }
  );

  if (it == std::end(mSubscriptions))
  {
    std::cerr << "Unknown subscription ACK!\n";
    exit(1);
  }

  it->second->setState(Subscription::State::Subscribed);
  it->second->onSubscribed();

  wxLogMessage("Subscribed to topics:");
  auto size = tok.get_topics()->size();
  for (size_t i = 0; i != size; ++i)
  {
    wxLogMessage("  - %s", tok.get_topics()->c_arr()[i]);
  }
}

void Client::on_success_unsubscribe(const mqtt::token& tok)
{
  auto it = std::find_if(
    std::begin(mSubscriptions),
    std::end(mSubscriptions),
    [&tok](const auto &sub)
    {
      return sub.second->getFilter() == tok.get_topics()->operator[](0);
    }
  );

  if (it == std::end(mSubscriptions))
  {
    std::cerr << "Unknown unsubscription ACK!\n";
    exit(1);
  }

  wxLogMessage(
    "Client::on_success_unsubscribe: %s",
    it->second->getFilter()
  );

  it->second->setState(Subscription::State::Unsubscribed);
  it->second->onUnsubscribed();
  mSubscriptions.erase(it);
}

void Client::on_failure_connect(const mqtt::token& tok)
{

  wxLogWarning(
    "Connection attempt failed: %s",
    returnCodes.at(tok.get_return_code()).c_str()
  );
  if (++mRetries > mRetriesMax)
  {
    exit(1);
  }
  reconnect();
}

void Client::on_failure_disconnect(const mqtt::token& tok)
{
  wxLogWarning(
    "Disconnection attempt failed: %s",
    returnCodes.at(tok.get_return_code()).c_str()
  );
}

void Client::on_failure_publish(const mqtt::token& tok)
{
  wxLogWarning(
    "Publishing attempt failed: %s",
    returnCodes.at(tok.get_return_code()).c_str()
  );
}

void Client::on_failure_subscribe(const mqtt::token& tok)
{
  wxLogWarning(
    "Subscription attempt failed: %s",
    returnCodes.at(tok.get_return_code()).c_str()
  );
}

void Client::on_failure_unsubscribe(const mqtt::token& tok)
{
  wxLogWarning(
    "Unsubscription attempt failed: %s",
    returnCodes.at(tok.get_return_code()).c_str()
  );
}

// mqtt::iaction_listener interface }

// mqtt::callback interface {

void Client::connected(const std::string& cause)
{
  wxLogMessage("Connected!");
  wxLogMessage("Subscribing to topics:");

  for (const auto &sub : mSubscriptions)
  {
    doSubscribe(sub.first);
  }
}

void Client::connection_lost(const std::string& cause)
{
  wxLogMessage("Connection lost: %s", cause);
  mRetries = 0;
  cleanSubscriptions();
  reconnect();
}

void Client::message_arrived(mqtt::const_message_ptr msg)
{
  for (const auto &sub : mSubscriptions)
  {
    if (Client::match(sub.second->getFilter(), msg->get_topic()))
    {
      sub.second->onMessage(msg);
    }
  }
}

void Client::delivery_complete(mqtt::delivery_token_ptr token)
{
  wxLogMessage("Delivery completed for %s", token->get_message()->get_topic());
}

// mqtt::callback interface }

void Client::doSubscribe(size_t id)
{
  auto it = mSubscriptions.find(id);
  if (it == std::end(mSubscriptions)) { return; }
  if (it->second->getState() == Subscription::State::Unsubscribed)
  {
    wxLogMessage(
      "Actually subscribing: %s (%d)",
      it->second->getFilter(),
      (unsigned)it->second->getQos()
    );
    mClient->subscribe(
      it->second->getFilter(),
      (unsigned)it->second->getQos(),
      nullptr,
      *this
    );
    it->second->setState(Subscription::State::PendingSubscription);
  }
}

void Client::cleanSubscriptions()
{
  for (const auto &sub : mSubscriptions)
  {
    sub.second->setState(Subscription::State::Unsubscribed);
  }
}

bool Client::match(const std::string &filter, const std::string &topic)
{
  auto split = [](const std::string& str, char delim)
    -> std::vector<std::string> {
      std::vector<std::string> strings;
      size_t start;
      size_t end = 0;
      while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
        end = str.find(delim, start);
        strings.push_back(str.substr(start, end - start));
      }
      return strings;
    };

  if (filter == topic)
  {
    return true;
  }

  auto plus = filter.find('+');

  if (filter.back() == '#' && plus == std::string::npos)
  {
    auto root = filter.substr(0, filter.size() - 1);
    if (*root.rend() == '/')
    {
      root = root.substr(0, root.size() - 1);
    }
    return topic.rfind(root, 0) == 0;
  }
  else if (plus != std::string::npos)
  {
    auto filterLevels = split(filter, '/');
    auto topicLevels  = split(topic,  '/');

    if (
      filterLevels.size() != topicLevels.size()
      && filter.back() != '#'
    ) {
      return false;
    }

    for (size_t i = 0; i != filterLevels.size(); ++i)
    {
      auto level = filterLevels.at(i);
      if (level == '#')
      {
        return true;
      }
      else if (level == '+')
      {
        continue;
      }
      else if (
        topicLevels.size() >= filterLevels.size()
        && topicLevels.at(i) == filterLevels.at(i)
      ) {
        continue;
      }
      return false;
    }

    return true;
  }

  return false;
}

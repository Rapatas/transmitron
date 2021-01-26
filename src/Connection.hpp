#ifndef CONNECTION_H
#define CONNECTION_H

#include <nlohmann/json.hpp>
#include <wx/log.h>

class Connection
{
public:

  Connection();
  explicit Connection(const nlohmann::json &data, const std::string &name);
  virtual ~Connection();

  nlohmann::json toJson() const;

  bool getAutoReconnect() const;
  unsigned getPort() const;
  unsigned getKeepAliveInterval() const;
  unsigned getMaxInFlight() const;
  unsigned getTimeout() const;
  std::string getName() const;
  std::string getHostname() const;
  std::string getClientId() const;
  std::string getUsername() const;
  std::string getPassword() const;

  void setAutoReconnect(bool autoReconnect);
  void setPort(unsigned port);
  void setKeepAliveInterval(unsigned keepAliveInterval);
  void setMaxInFlight(unsigned maxInFlight);
  void setTimeout(unsigned timeout);
  void setName(std::string name);
  void setHostname(std::string hostname);
  void setClientId(std::string clientId);
  void setUsername(std::string username);
  void setPassword(std::string password);

private:

  void fromJson(const nlohmann::json &data);

  bool mAutoReconnect = false;
  unsigned mPort = 1883;
  unsigned mKeepAliveInterval = 60;
  unsigned mMaxInFlight = 10;
  unsigned mTimeout = 30;
  std::string mName;
  std::string mHostname = "127.0.0.1";
  std::string mClientId;
  std::string mUsername;
  std::string mPassword;

  template<typename T>
  static bool tryToSet(
    const nlohmann::json &data,
    const std::string &key,
    T& value
  );
};

#endif // CONNECTION_H

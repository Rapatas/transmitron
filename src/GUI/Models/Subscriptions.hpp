#pragma once

#include <memory>

#include <spdlog/spdlog.h>
#include <wx/dataview.h>

#include "GUI/Events/Subscription.hpp"
#include "GUI/Types/Subscription.hpp"
#include "MQTT/Client.hpp"

namespace Rapatas::Transmitron::GUI::Models {

class Subscriptions : public wxDataViewVirtualListModel
{
public:

  struct Observer {
    Observer() = default;
    virtual ~Observer() = default;
    Observer(const Observer &) = default;
    Observer(Observer &&) = default;
    Observer &operator=(const Observer &) = default;
    Observer &operator=(Observer &&) = default;

    virtual void onColorSet(
      MQTT::Subscription::Id subscriptionId,
      wxColor color
    ) = 0;
    virtual void onMuted(MQTT::Subscription::Id subscriptionId) = 0;
    virtual void onSolo(MQTT::Subscription::Id subscriptionId) = 0;
    virtual void onUnmuted(MQTT::Subscription::Id subscriptionId) = 0;
    virtual void onUnsubscribed(MQTT::Subscription::Id subscriptionId) = 0;
    virtual void onCleared(MQTT::Subscription::Id subscriptionId) = 0;
    virtual void onMessage(
      MQTT::Subscription::Id subscriptionId,
      const MQTT::Message &message
    ) = 0;
  };

  enum class Column : uint8_t {
    Icon,
    Qos,
    Topic,
    Max
  };

  explicit Subscriptions(std::shared_ptr<MQTT::Client> client);
  explicit Subscriptions();

  size_t attachObserver(Observer *observer);
  bool detachObserver(size_t id);
  bool load(const std::string &recording);

  void mute(wxDataViewItem item);
  void setColor(wxDataViewItem item, const wxColor &color);
  void solo(wxDataViewItem item);
  void subscribe(const std::string &topic, MQTT::QoS qos);
  void unmute(wxDataViewItem item);
  void unsubscribe(wxDataViewItem item);
  void clear(wxDataViewItem item);

  [[nodiscard]] bool getMuted(MQTT::Subscription::Id subscriptionId) const;
  [[nodiscard]] bool getMuted(wxDataViewItem item) const;
  [[nodiscard]] std::string getFilter(wxDataViewItem item) const;
  [[nodiscard]] wxColor getColor(MQTT::Subscription::Id subscriptionId) const;
  [[nodiscard]] nlohmann::json toJson() const;
  [[nodiscard]] std::string getFilter( //
    MQTT::Subscription::Id subscriptionId
  ) const;

private:

  std::shared_ptr<spdlog::logger> mLogger;
  std::shared_ptr<MQTT::Client> mClient;
  std::map<MQTT::Subscription::Id, std::unique_ptr<Types::Subscription>>
    mSubscriptions;
  std::vector<MQTT::Subscription::Id> mRemap;
  std::map<size_t, Observer *> mObservers;

  // wxDataViewVirtualListModel interface.
  [[nodiscard]] unsigned GetColumnCount() const override;
  [[nodiscard]] wxString GetColumnType(unsigned int col) const override;
  [[nodiscard]] unsigned GetCount() const override;
  void GetValueByRow(
    wxVariant &variant,
    unsigned int row,
    unsigned int col //
  ) const override;
  bool GetAttrByRow(
    unsigned int row,
    unsigned int col,
    wxDataViewItemAttr &attr
  ) const override;
  bool SetValueByRow(
    const wxVariant &variant,
    unsigned int row,
    unsigned int col
  ) override;

  void onSubscribed(Events::Subscription &event);
  void onUnsubscribed(Events::Subscription &event);
  void onMessage(Events::Subscription &event);
};

} // namespace Rapatas::Transmitron::GUI::Models

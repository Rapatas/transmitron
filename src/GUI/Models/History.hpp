#pragma once

#include <mqtt/message.h>
#include <spdlog/spdlog.h>
#include <wx/dataview.h>

#include "GUI/Models/Subscriptions.hpp"
#include "MQTT/Client.hpp"
#include "MQTT/Message.hpp"
#include "MQTT/Subscription.hpp"

namespace Rapatas::Transmitron::GUI::Models {

class History :
  public wxEvtHandler,
  public wxDataViewVirtualListModel,
  public Subscriptions::Observer
{
public:

  struct Observer {
    Observer() = default;
    virtual ~Observer() = default;
    Observer(const Observer &) = default;
    Observer(Observer &&) = default;
    Observer &operator=(const Observer &) = default;
    Observer &operator=(Observer &&) = default;

    virtual void onMessage(wxDataViewItem item) = 0;
  };

  enum class Column : unsigned {
    Icon,
    Qos,
    Topic,
    Max
  };

  explicit History(const wxObjectDataPtr<Subscriptions> &subscriptions);

  size_t attachObserver(Observer *observer);
  bool detachObserver(size_t id);
  void clear();
  bool load(const std::string &recording);
  void setFilter(const std::string &filter);

  [[nodiscard]] std::string getPayload(const wxDataViewItem &item) const;
  [[nodiscard]] std::string getTopic(const wxDataViewItem &item) const;
  [[nodiscard]] MQTT::QoS getQos(const wxDataViewItem &item) const;
  [[nodiscard]] bool getRetained(const wxDataViewItem &item) const;
  [[nodiscard]] std::string getFilter() const;
  [[nodiscard]] nlohmann::json toJson() const;
  [[nodiscard]] const MQTT::Message &getMessage( //
    const wxDataViewItem &item
  ) const;

private:

  struct Node {
    MQTT::Message message;
    MQTT::Subscription::Id subscriptionId{};
  };

  std::shared_ptr<spdlog::logger> mLogger;
  std::vector<Node> mMessages;
  std::vector<size_t> mRemap;
  wxObjectDataPtr<Subscriptions> mSubscriptions;
  std::map<size_t, Observer *> mObservers;
  std::string mFilter;

  void remap();
  void refresh(MQTT::Subscription::Id subscriptionId);

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

  // Models::Subscriptions::Observer interface.
  void onMuted(MQTT::Subscription::Id subscriptionId) override;
  void onUnmuted(MQTT::Subscription::Id subscriptionId) override;
  void onSolo(MQTT::Subscription::Id subscriptionId) override;
  void onUnsubscribed(MQTT::Subscription::Id subscriptionId) override;
  void onCleared(MQTT::Subscription::Id subscriptionId) override;
  void onColorSet(
    MQTT::Subscription::Id subscriptionId,
    wxColor color //
  ) override;
  void onMessage(
    MQTT::Subscription::Id subscriptionId,
    const MQTT::Message &message
  ) override;
};

} // namespace Rapatas::Transmitron::GUI::Models

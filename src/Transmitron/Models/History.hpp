#ifndef TRANSMITRON_MODELS_HISTORY_HPP
#define TRANSMITRON_MODELS_HISTORY_HPP

#include <wx/dataview.h>
#include <mqtt/message.h>
#include "MQTT/Client.hpp"
#include "MQTT/Message.hpp"
#include "MQTT/Subscription.hpp"
#include "Transmitron/Models/Subscriptions.hpp"

namespace Transmitron::Models
{

class History :
  public wxEvtHandler,
  public wxDataViewVirtualListModel,
  public Subscriptions::Observer
{
public:

  struct Observer
  {
    virtual void onMessage(wxDataViewItem item) = 0;
  };

  enum class Column : unsigned
  {
    Icon,
    Qos,
    Topic,
    Max
  };

  explicit History(const wxObjectDataPtr<Subscriptions> &subscriptions);

  size_t attachObserver(Observer *observer);
  bool detachObserver(size_t id);
  void clear();

  std::string getPayload(const wxDataViewItem &item) const;
  std::string getTopic(const wxDataViewItem &item) const;
  MQTT::QoS getQos(const wxDataViewItem &item) const;
  bool getRetained(const wxDataViewItem &item) const;
  const MQTT::Message &getMessage(const wxDataViewItem &item) const;
  void setFilter(const std::string &filter);
  std::string getFilter() const;

private:

  struct Node
  {
    MQTT::Message message;
    MQTT::Subscription::Id_t subscriptionId {};
  };

  std::vector<Node> mMessages;
  std::vector<size_t> mRemap;
  wxObjectDataPtr<Subscriptions> mSubscriptions;
  std::map<size_t, Observer *> mObservers;
  std::string mFilter;

  void remap();
  void refresh(MQTT::Subscription::Id_t subscriptionId);

  // wxDataViewVirtualListModel interface.
  unsigned GetColumnCount() const override;
  wxString GetColumnType(unsigned int col) const override;
  unsigned GetCount() const override;
  void GetValueByRow(
    wxVariant &variant,
    unsigned int row,
    unsigned int col
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
  void onMuted(MQTT::Subscription::Id_t subscriptionId) override;
  void onUnmuted(MQTT::Subscription::Id_t subscriptionId) override;
  void onSolo(MQTT::Subscription::Id_t subscriptionId) override;
  void onColorSet(MQTT::Subscription::Id_t subscriptionId, wxColor color) override;
  void onUnsubscribed(MQTT::Subscription::Id_t subscriptionId) override;
  void onCleared(MQTT::Subscription::Id_t subscriptionId) override;
  void onMessage(MQTT::Subscription::Id_t subscriptionId, const MQTT::Message &message) override;
};

}

#endif // TRANSMITRON_MODELS_HISTORY_HPP

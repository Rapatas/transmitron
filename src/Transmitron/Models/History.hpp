#ifndef TRANSMITRON_MODELS_HISTORY_HPP
#define TRANSMITRON_MODELS_HISTORY_HPP

#include <wx/dataview.h>
#include <mqtt/message.h>
#include "MQTT/Client.hpp"
#include "MQTT/Message.hpp"
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
    virtual void onMessage(wxDataViewItem item) {}
  };

  struct Message
  {
    MQTT::Subscription::Id_t subscriptionId;
    mqtt::const_message_ptr message;
  };

  enum class Column : unsigned
  {
    Icon,
    Qos,
    Topic,
    Max
  };

  explicit History(wxObjectDataPtr<Subscriptions> subscriptions);

  size_t attachObserver(Observer *observer);
  bool detachObserver(size_t id);
  void clear();

  std::string getPayload(const wxDataViewItem &item) const;
  std::string getTopic(const wxDataViewItem &item) const;
  MQTT::QoS getQos(const wxDataViewItem &item) const;
  bool getRetained(const wxDataViewItem &item) const;
  std::shared_ptr<MQTT::Message> getMessage(const wxDataViewItem &item) const;

private:

  std::vector<Message> mMessages;
  std::vector<size_t> mRemap;
  wxObjectDataPtr<Subscriptions> mSubscriptions;
  std::map<size_t, Observer *> mObservers;

  void remap();
  void refresh(MQTT::Subscription::Id_t subscriptionId);

  // wxDataViewVirtualListModel interface.
  virtual unsigned GetColumnCount() const override;
  virtual wxString GetColumnType(unsigned int col) const override;
  virtual unsigned GetCount() const override;
  virtual void GetValueByRow(
    wxVariant &variant,
    unsigned int row,
    unsigned int col
  ) const override;
  virtual bool GetAttrByRow(
    unsigned int row,
    unsigned int col,
    wxDataViewItemAttr &attr
  ) const override;
  virtual bool SetValueByRow(
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
  void onMessage(
    MQTT::Subscription::Id_t subscriptionId,
    mqtt::const_message_ptr message
  ) override;

};

}

#endif // TRANSMITRON_MODELS_HISTORY_HPP

#ifndef TRANSMITRON_MODELS_SUBSCRIPTIONS_HPP
#define TRANSMITRON_MODELS_SUBSCRIPTIONS_HPP

#include <memory>
#include <wx/dataview.h>

#include "MQTT/Client.hpp"
#include "Transmitron/Types/Subscription.hpp"
#include "Transmitron/Events/Subscription.hpp"

namespace Transmitron::Models
{

class Subscriptions :
  public wxDataViewVirtualListModel
{
public:

  struct Observer
  {
    virtual void onColorSet(
      MQTT::Subscription::Id_t subscriptionId,
      wxColor color
    ) = 0;
    virtual void onMessage(
      MQTT::Subscription::Id_t subscriptionId,
      mqtt::const_message_ptr message
    ) = 0;
    virtual void onMuted(MQTT::Subscription::Id_t subscriptionId) = 0;
    virtual void onSolo(MQTT::Subscription::Id_t subscriptionId) = 0;
    virtual void onUnmuted(MQTT::Subscription::Id_t subscriptionId) = 0;
    virtual void onUnsubscribed(MQTT::Subscription::Id_t subscriptionId) = 0;
    virtual void onCleared(MQTT::Subscription::Id_t subscriptionId) = 0;
  };

  enum class Column : unsigned
  {
    Icon,
    Qos,
    Topic,
    Max
  };

  explicit Subscriptions(std::shared_ptr<MQTT::Client> client);

  size_t attachObserver(Observer *observer);
  bool detachObserver(size_t id);

  void mute(wxDataViewItem item);
  void setColor(wxDataViewItem item, const wxColor &color);
  void solo(wxDataViewItem item);
  void subscribe(const std::string &topic, MQTT::QoS qos);
  void unmute(wxDataViewItem item);
  void unsubscribe(wxDataViewItem item);
  void clear(wxDataViewItem item);

  bool getMuted(MQTT::Subscription::Id_t subscriptionId) const;
  bool getMuted(wxDataViewItem item) const;
  std::string getFilter(MQTT::Subscription::Id_t subscriptionId) const;
  std::string getFilter(wxDataViewItem item) const;
  wxColor getColor(MQTT::Subscription::Id_t subscriptionId) const;

private:

  std::shared_ptr<MQTT::Client> mClient;
  std::map<MQTT::Subscription::Id_t, std::unique_ptr<Types::Subscription>> mSubscriptions;
  std::vector<MQTT::Subscription::Id_t> mRemap;
  std::map<size_t, Observer *> mObservers;

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

  void onSubscribed(Events::Subscription &e);
  void onUnsubscribed(Events::Subscription &e);
  void onMessage(Events::Subscription &e);
};

}

#endif // TRANSMITRON_MODELS_SUBSCRIPTIONS_HPP

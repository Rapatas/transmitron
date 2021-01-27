#ifndef MODELS_SUBSCRIPTIONS_H
#define MODELS_SUBSCRIPTIONS_H

#include <memory>
#include <wx/dataview.h>

#include "History.hpp"
#include "MQTT/Client.hpp"
#include "SubscriptionData.hpp"
#include "Events/SubscriptionEvent.hpp"

class Subscriptions :
  public wxDataViewModel,
  public SubscriptionData::Observer
{
public:

  enum class Column : unsigned
  {
    Icon,
    Qos,
    Topic,
    Max
  };

  explicit Subscriptions(
    std::shared_ptr<MQTT::Client> client,
    wxObjectDataPtr<History> history
  );
  virtual ~Subscriptions();

  void subscribe(const std::string &topic, MQTT::QoS qos);

  void unsubscribe(const wxDataViewItem &item);
  void setColor(const wxDataViewItem &item, const wxColor &color);
  void mute(const wxDataViewItem &item);
  void unmute(const wxDataViewItem &item);
  void solo(const wxDataViewItem &item);

  std::string getFilter(const wxDataViewItem &item) const;
  bool getMuted(const wxDataViewItem &item) const;

private:

  std::shared_ptr<MQTT::Client> mClient;
  wxObjectDataPtr<History> mHistory;
  std::vector<SubscriptionData*> mSubscriptions;

  // wxDataViewModel interface.
  virtual unsigned GetColumnCount() const override;
  virtual wxString GetColumnType(unsigned int col) const override;
  virtual void GetValue(
    wxVariant &variant,
    const wxDataViewItem &item,
    unsigned int col
  ) const override;
  virtual bool SetValue(
    const wxVariant &variant,
    const wxDataViewItem &item,
    unsigned int col
  ) override;
  virtual bool IsEnabled(
    const wxDataViewItem &item,
    unsigned int col
  ) const override;
  virtual wxDataViewItem GetParent(
    const wxDataViewItem &item
  ) const override;
  virtual bool IsContainer(
    const wxDataViewItem &item
  ) const override;
  virtual unsigned int GetChildren(
    const wxDataViewItem &parent,
    wxDataViewItemArray &array
  ) const override;

  // SubscriptionData::Observer interface.
  void onMessage(
    SubscriptionData *subscriptionData,
    mqtt::const_message_ptr msg
  ) override;

  void onSubscribed(SubscriptionEvent &e);
  void onUnsubscribed(SubscriptionEvent &e);
  // void onMessage(SubscriptionEvent &e);
};

#endif // MODELS_SUBSCRIPTIONS_H

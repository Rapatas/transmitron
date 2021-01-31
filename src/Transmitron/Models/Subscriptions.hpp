#ifndef TRANSMITRON_MODELS_SUBSCRIPTIONS_HPP
#define TRANSMITRON_MODELS_SUBSCRIPTIONS_HPP

#include <memory>
#include <wx/dataview.h>

#include "History.hpp"
#include "MQTT/Client.hpp"
#include "Transmitron/Types/SubscriptionData.hpp"
#include "Transmitron/Events/Subscription.hpp"

namespace Transmitron::Models
{

class Subscriptions :
  public wxDataViewVirtualListModel,
  public Types::SubscriptionData::Observer
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
  std::vector<Types::SubscriptionData*> mSubscriptions;

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

  // SubscriptionData::Observer interface.
  void onMessage(
    Types::SubscriptionData *subscriptionData,
    mqtt::const_message_ptr msg
  ) override;

  void onSubscribed(Events::Subscription&e);
  void onUnsubscribed(Events::Subscription&e);
};

}

#endif // TRANSMITRON_MODELS_SUBSCRIPTIONS_HPP

#ifndef TRANSMITRON_MODELS_SUBSCRIPTIONS_HPP
#define TRANSMITRON_MODELS_SUBSCRIPTIONS_HPP

#include <memory>
#include <wx/dataview.h>

#include "MQTT/Client.hpp"
#include "Transmitron/Types/SubscriptionData.hpp"
#include "Transmitron/Events/Subscription.hpp"

namespace Transmitron::Models
{

class Subscriptions :
  public wxDataViewVirtualListModel
{
public:

  struct Observer
  {
    virtual void onMuted(wxDataViewItem subscription) {};
    virtual void onUnmuted(wxDataViewItem subscription) {};
    virtual void onSolo(wxDataViewItem subscription) {};
    virtual void onColorSet(wxDataViewItem subscription, wxColor color) {};
    virtual void onUnsubscribed(wxDataViewItem subscription) {};
    virtual void onMessage(
      wxDataViewItem subscription,
      mqtt::const_message_ptr message
    ) {}
  };

  enum class Column : unsigned
  {
    Icon,
    Qos,
    Topic,
    Max
  };

  explicit Subscriptions(std::shared_ptr<MQTT::Client> client);
  virtual ~Subscriptions();

  size_t attachObserver(Observer *observer);
  bool detachObserver(size_t id);

  void subscribe(const std::string &topic, MQTT::QoS qos);

  void unsubscribe(const wxDataViewItem &item);
  void setColor(const wxDataViewItem &item, const wxColor &color);
  void mute(const wxDataViewItem &item);
  void unmute(const wxDataViewItem &item);
  void solo(const wxDataViewItem &item);

  std::string getFilter(const wxDataViewItem &item) const;
  bool getMuted(const wxDataViewItem &item) const;
  wxColor getColor(const wxDataViewItem &item) const;

private:

  std::shared_ptr<MQTT::Client> mClient;
  std::vector<Types::SubscriptionData*> mSubscriptions;
  std::map<size_t, Observer *> mObservers;

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

  void onSubscribed(Events::Subscription &e);
  void onUnsubscribed(Events::Subscription &e);
  void onMessage(Events::Subscription &e);
};

}

#endif // TRANSMITRON_MODELS_SUBSCRIPTIONS_HPP

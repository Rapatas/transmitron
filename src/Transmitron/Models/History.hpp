#ifndef TRANSMITRON_MODELS_HISTORY_HPP
#define TRANSMITRON_MODELS_HISTORY_HPP

#include <mutex>
#include <wx/dataview.h>
#include <mqtt/message.h>
#include "MQTT/Client.hpp"
#include "Transmitron/Types/SubscriptionData.hpp"

namespace Transmitron::Models
{

class History :
  public wxEvtHandler,
  public wxDataViewVirtualListModel
{
public:

  struct Observer
  {
    virtual void onMessage(wxDataViewItem item) {}
  };

  struct Message
  {
    Types::SubscriptionData *sub;
    mqtt::const_message_ptr msg;
  };

  enum class Column : unsigned
  {
    Icon,
    Qos,
    Retained,
    Topic,
    Max
  };

  explicit History();
  virtual ~History();

  size_t attachObserver(Observer *observer);

  void insert(
    Types::SubscriptionData *sub,
    mqtt::const_message_ptr msg
  );
  void remove(Types::SubscriptionData *sub);
  void remap();
  void refresh(Types::SubscriptionData *sub);

  std::string getPayload(const wxDataViewItem &item) const;
  std::string getTopic(const wxDataViewItem &item) const;
  MQTT::QoS getQos(const wxDataViewItem &item) const;
  bool getRetained(const wxDataViewItem &item) const;

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

private:

  std::vector<Message> mMessages;
  std::mutex mRemapMtx;
  std::vector<size_t> mRemap;
  std::map<size_t, Observer *> mObservers;

};

}

#endif // TRANSMITRON_MODELS_HISTORY_HPP

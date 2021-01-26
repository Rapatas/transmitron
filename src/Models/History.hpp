#ifndef HISTORY_H
#define HISTORY_H

#include <mutex>
#include <wx/dataview.h>
#include <mqtt/message.h>
#include "MQTT/Client.hpp"
#include "SubscriptionData.hpp"

class History :
  public wxEvtHandler,
  public wxDataViewVirtualListModel
{
public:

  struct Message
  {
    SubscriptionData *sub;
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

  void insert(
    SubscriptionData *sub,
    mqtt::const_message_ptr msg
  );
  void remove(SubscriptionData *sub);
  void remap();
  void refresh(SubscriptionData *sub);

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

};

#endif // HISTORY_H

#ifndef TRANSMITRON_TABS_CLIENT_HPP
#define TRANSMITRON_TABS_CLIENT_HPP

#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/aui/aui.h>
#include <wx/tglbtn.h>

#include "MQTT/Client.hpp"
#include "Transmitron/Events/Message.hpp"
#include "Transmitron/Models/History.hpp"
#include "Transmitron/Models/Subscriptions.hpp"
#include "Transmitron/Types/Connection.hpp"
#include "Transmitron/Widgets/Edit.hpp"

#ifndef BUILD_DOCKING
#define BUILD_DOCKING false
#endif

namespace Transmitron::Tabs
{

class Client :
  public wxPanel,
  public MQTT::Client::Observer
{
public:

  Client(
    wxWindow* parent = nullptr,
    const Types::Connection &connection = {}
  );
  ~Client();

  void resize() const;

  // MQTT::Client::Observer:
  void onConnected() override;
  void onDisconnected() override;

private:

  enum class ContextIDs : unsigned
  {
    SubscriptionsUnsubscribe,
    SubscriptionsChangeColor,
    SubscriptionsMute,
    SubscriptionsUnmute,
    SubscriptionsSolo,
    HistoryRetainedClear,
    HistoryResend,
    HistoryEdit,
  };

  const Types::Connection mConnectionInfo;

  // Connection:
  wxPanel *mConnection;
  wxButton *mConnect;
  wxStatusBar *mStatusBar;

  // History:
  wxPanel *mHistory;
  wxObjectDataPtr<Models::History> mHistoryModel;
  wxDataViewCtrl *mHistoryCtrl;

  // Preview:
  Widgets::Edit *mPreview;

  // Publish:
  Widgets::Edit *mPublish;

  // Subscriptions:
  wxBitmapButton *mSubscribe;
  wxTextCtrl *mFilter;
  wxPanel *mSubscriptions;
  Models::Subscriptions *mSubscriptionsModel;
  wxDataViewCtrl *mSubscriptionsCtrl;

  std::shared_ptr<MQTT::Client> mClient;

  void setupPanelConnect(wxWindow *parent);
  void setupPanelSubscriptions(wxWindow *parent);
  void setupPanelHistory(wxWindow *parent);
  void setupPanelPreview(wxWindow *parent);
  void setupPanelPublish(wxWindow *parent);

  void onClose(wxCloseEvent &event);
  void onConnectClicked(wxCommandEvent &event);
  void onHistorySelected(wxDataViewEvent& event);
  void onPublishClicked(wxCommandEvent &event);
  void onSubscribeClicked(wxCommandEvent &event);
  void onSubscribeEnter(wxKeyEvent &event);
  void onSubscriptionContext(wxDataViewEvent& event);
  void onHistoryContext(wxDataViewEvent& event);
  void onContextSelected(wxCommandEvent& event);
  void onSubscriptionSelected(wxDataViewEvent &event);
  void onMessageAddedSync(Events::Message &event);
  void onMessageAdded(Events::Message &event);

  wxSplitterWindow *mSplitLeft;
  wxSplitterWindow *mSplitRight;
  wxSplitterWindow *mSplitCenter;

#if BUILD_DOCKING

  wxAuiManager mAuiMan;

#endif

};

}

#endif // TRANSMITRON_TABS_CLIENT_HPP

#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/aui/aui.h>
#include <wx/tglbtn.h>

#include "MQTT/Client.hpp"
#include "Widgets/Edit.hpp"
#include "Models/History.hpp"
#include "Models/Subscriptions.hpp"
#include "Connection.hpp"
#include "Events/MessageEvent.hpp"

#ifndef BUILD_DOCKING
#define BUILD_DOCKING false
#endif

class Client :
  public wxPanel,
  public MQTT::Client::Observer
{
public:

  Client(
    wxWindow* parent = nullptr,
    const Connection &connection = Connection{}
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
  };

  const Connection mConnectionInfo;

  // Connection:
  wxPanel *mConnection;
  wxButton *mConnect;
  wxStatusBar *mStatusBar;

  // History:
  wxPanel *mHistory;
  wxObjectDataPtr<History> mHistoryModel;
  wxDataViewCtrl *mHistoryCtrl;

  // Preview:
  Edit *mPreview;

  // Publish:
  Edit *mPublish;

  // Subscriptions:
  wxBitmapButton *mSubscribe;
  wxTextCtrl *mFilter;
  wxPanel *mSubscriptions;
  Subscriptions *mSubscriptionsModel;
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
  void onMessageAddedSync(MessageEvent &event);
  void onMessageAdded(MessageEvent &event);

  wxSplitterWindow *mSplitLeft;
  wxSplitterWindow *mSplitRight;
  wxSplitterWindow *mSplitCenter;

#if BUILD_DOCKING

  wxAuiManager mAuiMan;

#endif

};


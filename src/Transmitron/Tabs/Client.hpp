#ifndef TRANSMITRON_TABS_CLIENT_HPP
#define TRANSMITRON_TABS_CLIENT_HPP

#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/aui/aui.h>
#include <wx/tglbtn.h>

#include "MQTT/Client.hpp"
#include "Transmitron/Events/Connection.hpp"
#include "Transmitron/Widgets/TopicCtrl.hpp"
#include "Transmitron/Models/History.hpp"
#include "Transmitron/Models/Subscriptions.hpp"
#include "Transmitron/Models/Snippets.hpp"
#include "Transmitron/Types/Connection.hpp"
#include "Transmitron/Widgets/Edit.hpp"

#ifndef BUILD_DOCKING
#define BUILD_DOCKING false
#endif

namespace Transmitron::Tabs
{

class Client :
  public wxPanel,
  public Models::History::Observer,
  public MQTT::Client::Observer
{
public:

  Client(
    wxWindow* parent = nullptr,
    std::shared_ptr<Types::Connection> connection = nullptr
  );
  ~Client();

  void resize() const;

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

  static const size_t OptionsHeight = 26;

  std::shared_ptr<Types::Connection> mConnection;

  // Connection:
  wxPanel *mConnectionBar;
  wxButton *mConnect;
  wxStatusBar *mStatusBar;

  // History:
  wxPanel *mHistory;
  wxObjectDataPtr<Models::History> mHistoryModel;
  wxDataViewCtrl *mHistoryCtrl;
  wxCheckBox *mAutoScroll;

  // Preview:
  Widgets::Edit *mPreview;

  // Publish:
  Widgets::Edit *mPublish;

  // Subscriptions:
  wxBitmapButton *mSubscribe;
  Widgets::TopicCtrl *mFilter;
  wxPanel *mSubscriptions;
  Models::Subscriptions *mSubscriptionsModel;
  wxDataViewCtrl *mSubscriptionsCtrl;

  // Snippets:
  wxPanel *mSnippets;
  wxObjectDataPtr<Models::Snippets> mSnippetsModel;
  wxDataViewCtrl *mSnippetsCtrl;

  std::shared_ptr<MQTT::Client> mClient;

  void setupPanelConnect(wxWindow *parent);
  void setupPanelSubscriptions(wxWindow *parent);
  void setupPanelHistory(wxWindow *parent);
  void setupPanelPreview(wxWindow *parent);
  void setupPanelPublish(wxWindow *parent);
  void setupPanelSnippets(wxWindow *parent);

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

  void onConnectedSync(Events::Connection &e);
  void onDisconnectedSync(Events::Connection &e);

  wxSplitterWindow *mSplitTop;
  wxSplitterWindow *mSplitCenter;
  wxSplitterWindow *mSplitBottom;
  wxSplitterWindow *mSplitHistory;

  // MQTT::Client::Observer interface.
  void onConnected() override;
  void onDisconnected() override;

  // Models::History::Observer interface.
  void onMessage(wxDataViewItem item) override;

#if BUILD_DOCKING

  wxAuiManager mAuiMan;

#endif

};

}

#endif // TRANSMITRON_TABS_CLIENT_HPP

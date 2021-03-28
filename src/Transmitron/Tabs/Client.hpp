#ifndef TRANSMITRON_TABS_CLIENT_HPP
#define TRANSMITRON_TABS_CLIENT_HPP

#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/aui/aui.h>
#include <wx/tglbtn.h>

#include "MQTT/Client.hpp"
#include "MQTT/BrokerOptions.hpp"
#include "Transmitron/Events/Connection.hpp"
#include "Transmitron/Widgets/TopicCtrl.hpp"
#include "Transmitron/Models/History.hpp"
#include "Transmitron/Models/Subscriptions.hpp"
#include "Transmitron/Models/Snippets.hpp"
#include "Transmitron/Widgets/Edit.hpp"

namespace Transmitron::Tabs
{

class Client :
  public wxPanel,
  public Models::History::Observer,
  public MQTT::Client::Observer
{
public:

  Client(
    wxWindow* parent,
    const MQTT::BrokerOptions &brokerOptions,
    const wxObjectDataPtr<Models::Snippets> &snippetsModel
  );
  Client(const Client &other) = delete;
  Client(Client &&other) = delete;
  Client &operator=(const Client &other) = delete;
  Client &operator=(Client &&other) = delete;
  ~Client();

private:

  enum class ContextIDs : unsigned
  {
    SubscriptionsUnsubscribe,
    SubscriptionsChangeColor,
    SubscriptionsMute,
    SubscriptionsUnmute,
    SubscriptionsSolo,
    SubscriptionsClear,
    HistoryRetainedClear,
    HistoryResend,
    HistoryEdit,
    HistorySaveSnippet,
    SnippetRename,
    SnippetDelete,
    SnippetNewSnippet,
    SnippetNewFolder,
  };

  enum class Panes : unsigned
  {
    History = 0,
    Preview = 4,
    Publish = 3,
    Snippets = 2,
    Subscriptions = 1,
  };

  struct Pane
  {
    std::string name;
    wxAuiPaneInfo info;
    wxPanel *panel = nullptr;
    const wxBitmap *icon18x18 = nullptr;
    const wxBitmap *icon18x14 = nullptr;
    wxButton *toggle = nullptr;
  };

  std::map<Panes, Pane> mPanes;

  static constexpr size_t OptionsHeight = 26;

  const MQTT::BrokerOptions mBrokerOptions;
  const wxFont mFont;

  wxBoxSizer *mMasterSizer = nullptr;

  // Profile:
  wxPanel *mProfileBar = nullptr;
  wxBoxSizer *mProfileSizer = nullptr;
  wxButton *mConnect = nullptr;
  wxButton *mDisconnect = nullptr;
  wxButton *mCancel = nullptr;

  // History:
  wxObjectDataPtr<Models::History> mHistoryModel;
  wxDataViewCtrl *mHistoryCtrl = nullptr;
  wxCheckBox *mAutoScroll = nullptr;
  wxButton *mHistoryClear = nullptr;

  // Subscriptions:
  wxBitmapButton *mSubscribe = nullptr;
  Widgets::TopicCtrl *mFilter = nullptr;
  wxObjectDataPtr<Models::Subscriptions> mSubscriptionsModel;
  wxDataViewCtrl *mSubscriptionsCtrl = nullptr;

  // Snippets:
  wxObjectDataPtr<Models::Snippets> mSnippetsModel;
  wxDataViewCtrl *mSnippetsCtrl = nullptr;
  std::array<wxDataViewColumn*, Models::Snippets::Column::Max> mSnippetColumns {};
  bool mSnippetExplicitEditRequest = false;
  bool mSnippetsWasExpanded = false;
  std::pair<bool, wxDataViewItem> mSnippetsPossible;

  std::shared_ptr<MQTT::Client> mClient;
  size_t mMqttObserverId = 0;

  void setupPanelConnect(wxWindow *parent);
  void setupPanelSubscriptions(wxWindow *parent);
  void setupPanelHistory(wxWindow *parent);
  void setupPanelPreview(wxWindow *parent);
  void setupPanelPublish(wxWindow *parent);
  void setupPanelSnippets(wxWindow *parent);

  void onClose(wxCloseEvent &event);
  void onConnectClicked(wxCommandEvent &event);
  void onDisconnectClicked(wxCommandEvent &event);
  void onCancelClicked(wxCommandEvent &event);
  void onHistorySelected(wxDataViewEvent &event);
  void onPublishClicked(wxCommandEvent &event);
  void onPublishSaveSnippet(Events::Edit &e);
  void onPreviewSaveSnippet(Events::Edit &e);
  void onSubscribeClicked(wxCommandEvent &event);
  void onSubscribeEnter(wxKeyEvent &event);
  void onSubscriptionContext(wxDataViewEvent &event);
  void onHistoryContext(wxDataViewEvent &event);
  void onContextSelected(wxCommandEvent &event);
  void onSubscriptionSelected(wxDataViewEvent &event);
  void onSnippetsEdit(wxDataViewEvent &e);
  void onSnippetsContext(wxDataViewEvent &e);
  void onSnippetsSelected(wxDataViewEvent &e);
  void onSnippetsActivated(wxDataViewEvent &e);
  void onSnippetsDrag(wxDataViewEvent &e);
  void onSnippetsDrop(wxDataViewEvent &e);
  void onSnippetsDropPossible(wxDataViewEvent &e);

  void handleInserted(wxDataViewItem &inserted);
  void allowConnect();
  void allowDisconnect();
  void allowCancel();

  // MQTT::Client::Observer interface.
  void onConnected() override;
  void onDisconnected() override;
  void onConnectionLost() override;
  void onConnectionFailure() override;

  void onConnectedSync(Events::Connection &e);
  void onDisconnectedSync(Events::Connection &e);
  void onConnectionLostSync(Events::Connection &e);
  void onConnectionFailureSync(Events::Connection &e);

  // Models::History::Observer interface.
  void onMessage(wxDataViewItem item) override;

  wxAuiManager mAuiMan;

};

}

#endif // TRANSMITRON_TABS_CLIENT_HPP

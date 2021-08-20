#ifndef TRANSMITRON_TABS_CLIENT_HPP
#define TRANSMITRON_TABS_CLIENT_HPP

#include <wx/event.h>
#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/aui/aui.h>
#include <wx/tglbtn.h>
#include <wx/combobox.h>

#include "MQTT/Client.hpp"
#include "MQTT/BrokerOptions.hpp"
#include "Transmitron/Events/Connection.hpp"
#include "Transmitron/Events/Layout.hpp"
#include "Transmitron/Widgets/TopicCtrl.hpp"
#include "Transmitron/Widgets/Layouts.hpp"
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
    const wxObjectDataPtr<Models::Snippets> &snippetsModel,
    const wxObjectDataPtr<Models::Layouts> &layoutsModel,
    bool darkMode
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
    HistoryCopyTopic,
    HistoryCopyPayload,
    SnippetRename,
    SnippetDelete,
    SnippetNewSnippet,
    SnippetNewFolder,
    SnippetPublish,
    SnippetOverwrite,
  };

  enum class Panes : unsigned
  {
    History = 0,
    Subscriptions = 1,
    Snippets = 2,
    Publish = 3,
    Preview = 4,
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

  static constexpr size_t PaneMinWidth = 200;
  static constexpr size_t PaneMinHeight = 100;
  static constexpr size_t EditorMinHeight = 200;

  std::map<Panes, Pane> mPanes;

  static constexpr size_t OptionsHeight = 26;

  const MQTT::BrokerOptions mBrokerOptions;
  const wxFont mFont;
  const bool mDarkMode;

  wxBoxSizer *mMasterSizer = nullptr;

  Widgets::Layouts *mLayouts = nullptr;

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
  Widgets::TopicCtrl *mHistorySearchFilter = nullptr;
  wxButton *mHistorySearchButton = nullptr;

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

  void onClose(wxCloseEvent &event);

  // Connection.
  void allowCancel();
  void allowConnect();
  void allowDisconnect();
  void onCancelClicked(wxCommandEvent &event);
  void onConnectClicked(wxCommandEvent &event);
  void onDisconnectClicked(wxCommandEvent &event);

  // Context.
  void onContextSelected(wxCommandEvent &event);
  void onContextSelectedHistoryEdit(wxCommandEvent &event);
  void onContextSelectedHistoryResend(wxCommandEvent &event);
  void onContextSelectedHistoryRetainedClear(wxCommandEvent &event);
  void onContextSelectedHistorySaveSnippet(wxCommandEvent &event);
  void onContextSelectedHistoryCopyTopic(wxCommandEvent &event);
  void onContextSelectedHistoryCopyPayload(wxCommandEvent &event);
  void onContextSelectedSnippetDelete(wxCommandEvent &event);
  void onContextSelectedSnippetNewFolder(wxCommandEvent &event);
  void onContextSelectedSnippetNewSnippet(wxCommandEvent &event);
  void onContextSelectedSnippetRename(wxCommandEvent &event);
  void onContextSelectedSnippetPublish(wxCommandEvent &event);
  void onContextSelectedSnippetOverwrite(wxCommandEvent &event);
  void onContextSelectedSubscriptionsChangeColor(wxCommandEvent &event);
  void onContextSelectedSubscriptionsClear(wxCommandEvent &event);
  void onContextSelectedSubscriptionsMute(wxCommandEvent &event);
  void onContextSelectedSubscriptionsSolo(wxCommandEvent &event);
  void onContextSelectedSubscriptionsUnmute(wxCommandEvent &event);
  void onContextSelectedSubscriptionsUnsubscribe(wxCommandEvent &event);
  void onHistoryContext(wxDataViewEvent &event);
  void onSnippetsContext(wxDataViewEvent &e);
  void onSubscriptionContext(wxDataViewEvent &event);

  // History.
  void onHistoryClearClicked(wxCommandEvent &event);
  void onHistorySelected(wxDataViewEvent &event);
  void onHistoryDoubleClicked(wxDataViewEvent &event);
  void onHistorySearchKey(wxKeyEvent &event);
  void onHistorySearchButton(wxCommandEvent &event);

  // Preview.
  void onPreviewSaveSnippet(Events::Edit &e);

  // Publish.
  void onPublishClicked(wxCommandEvent &event);
  void onPublishSaveSnippet(Events::Edit &e);

  // Setup.
  void setupPanelConnect(
    wxWindow *parent,
    const wxObjectDataPtr<Models::Layouts> &layoutsModel
  );
  void setupPanelHistory(wxWindow *parent);
  void setupPanelPreview(wxWindow *parent);
  void setupPanelPublish(wxWindow *parent);
  void setupPanelSnippets(wxWindow *parent);
  void setupPanelSubscriptions(wxWindow *parent);

  // Snippets.
  void onSnippetsActivated(wxDataViewEvent &e);
  void onSnippetsDrag(wxDataViewEvent &e);
  void onSnippetsDrop(wxDataViewEvent &e);
  void onSnippetsDropPossible(wxDataViewEvent &e);
  void onSnippetsEdit(wxDataViewEvent &e);
  void onSnippetsSelected(wxDataViewEvent &e);

  // Subscriptions.
  void onSubscribeClicked(wxCommandEvent &event);
  void onSubscribeEnter(wxKeyEvent &event);
  void onSubscriptionSelected(wxDataViewEvent &event);

  // Layouts.
  void onLayoutSelected(Events::Layout &event);
  void onLayoutResized(Events::Layout &event);

  // MQTT::Client::Observer interface.
  void onConnected() override;
  void onDisconnected() override;
  void onConnectionLost() override;
  void onConnectionFailure() override;

  // MQTT::Client::Observer on GUI thread.
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

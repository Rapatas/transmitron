#include <chrono>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/wx.h>
#include <wx/artprov.h>
#include <wx/propgrid/propgrid.h>

#include "Homepage.hpp"
#include "Common/Log.hpp"
#include "Transmitron/Events/Edit.hpp"
#include "Transmitron/Events/Recording.hpp"
#include "Transmitron/Events/Profile.hpp"
#include "Transmitron/Models/Layouts.hpp"
#include "Transmitron/Types/ClientOptions.hpp"
#include "Transmitron/Widgets/Container.hpp"

using namespace Transmitron::Tabs;
using namespace Transmitron;
using namespace Transmitron::Events;

wxDEFINE_EVENT(Events::CONNECTION, Events::Connection); // NOLINT
wxDEFINE_EVENT(Events::RECORDING_OPEN, Events::Recording); // NOLINT
wxDEFINE_EVENT(Events::PROFILE_EDIT, Events::Profile); // NOLINT
wxDEFINE_EVENT(Events::PROFILE_CREATE, Events::Profile); // NOLINT

Homepage::Homepage(
  wxWindow *parent,
  wxFontInfo labelFont,
  int optionsHeight,
  const wxObjectDataPtr<Models::Profiles> &profilesModel,
  const wxObjectDataPtr<Models::Layouts> &layoutsModel
) :
  wxPanel(
    parent,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxTAB_TRAVERSAL,
    "Homepage"
  ),
  mLabelFont(std::move(labelFont)),
  mOptionsHeight(optionsHeight),
  mProfilesModel(profilesModel),
  mLayoutsModel(layoutsModel)
{
  mLogger = Common::Log::create("Transmitron::Homepage");

  auto *container = new Widgets::Container(this);
  auto *master = new wxPanel(container);
  container->contain(master);

  setupProfiles(master);
  setupQuickConnect(master);
  setupRecordings(master);

  auto *vsizer = new wxBoxSizer(wxVERTICAL);
  vsizer->Add(mQuickConnect, 0, wxEXPAND);
  vsizer->AddSpacer(10);
  vsizer->Add(mProfiles, 1, wxEXPAND);
  vsizer->AddSpacer(10);
  vsizer->Add(mRecordings, 0, wxEXPAND);
  master->SetSizer(vsizer);

  auto *sizer = new wxBoxSizer(wxVERTICAL);
  container->SetMinSize(wxSize(0, 400));
  sizer->Add(container, 0, wxEXPAND);
  SetSizer(sizer);

  Bind(wxEVT_COMMAND_MENU_SELECTED, &Homepage::onContextSelected, this);
}

void Homepage::focus()
{
  wxDataViewItemArray children;
  mProfilesModel->GetChildren(wxDataViewItem(nullptr), children);
  if (children.empty()) { return; }

  const auto first = children.front();
  mProfilesCtrl->Select(first);
  mProfilesCtrl->SetFocus();
}

void Homepage::setupProfiles(wxPanel *parent)
{
  wxDataViewColumn* const name = new wxDataViewColumn(
    "Name",
    new wxDataViewTextRenderer(),
    (unsigned)Models::Profiles::Column::Name,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );
  wxDataViewColumn* const url = new wxDataViewColumn(
    "Address",
    new wxDataViewTextRenderer(),
    (unsigned)Models::Profiles::Column::URL,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );

  mProfiles = new wxPanel(parent, -1);
  mProfiles->SetMinSize(wxSize(0, 400));

  auto *label = new wxStaticText(mProfiles, -1, "Profiles");
  label->SetFont(mLabelFont);

  mProfilesCtrl = new wxDataViewCtrl(
    mProfiles,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxDV_NO_HEADER
  );
  mProfilesCtrl->AssociateModel(mProfilesModel.get());
  mProfilesCtrl->AppendColumn(name);
  mProfilesCtrl->AppendColumn(url);
  mProfilesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_ACTIVATED,
    &Homepage::onProfileActivated,
    this
   );
  mProfilesCtrl->Bind(
    wxEVT_DATAVIEW_SELECTION_CHANGED,
    &Homepage::onProfileSelected,
    this
   );
  mProfilesCtrl->Bind(
    wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
    &Homepage::onProfileContext,
    this
  );

  auto *vsizer = new wxBoxSizer(wxVERTICAL);
  vsizer->Add(label,         0, wxEXPAND);
  vsizer->Add(mProfilesCtrl, 1, wxEXPAND);
  mProfiles->SetSizer(vsizer);
}

void Homepage::setupQuickConnect(wxPanel *parent)
{
  mQuickConnect = new wxPanel(parent);

  auto *panel = new wxPanel(mQuickConnect);

  mQuickConnectUrl = new wxTextCtrl(panel, wxID_ANY);
  mQuickConnectUrl->SetHint("localhost:1883");
  mQuickConnectUrl->Bind(wxEVT_KEY_DOWN, [&](wxKeyEvent &event){
    const auto isEnter = event.GetKeyCode() == WXK_RETURN;
    if (isEnter)
    {
      onQuickConnect();
    }
    event.Skip();
  });
  mQuickConnectBtn = new wxButton(panel, wxID_ANY, "Connect");
  mQuickConnectBtn->Bind(wxEVT_BUTTON, [&](wxCommandEvent &event){
    onQuickConnect();
    event.Skip();
  });

  auto *label = new wxStaticText(mQuickConnect, -1, "Quick Connect");
  label->SetFont(mLabelFont);

  auto *hsizer = new wxBoxSizer(wxHORIZONTAL);
  hsizer->Add(mQuickConnectBtn, 0, wxEXPAND);
  hsizer->Add(mQuickConnectUrl, 1, wxEXPAND);
  panel->SetSizer(hsizer);

  auto *vsizer = new wxBoxSizer(wxVERTICAL);
  vsizer->Add(label, 0, wxEXPAND);
  vsizer->Add(panel, 0, wxEXPAND);
  mQuickConnect->SetSizer(vsizer);
}

void Homepage::setupRecordings(wxPanel *parent)
{
  mRecordings = new wxPanel(parent, -1);

  auto *label = new wxStaticText(mRecordings, -1, "Recordings");
  label->SetFont(mLabelFont);

  auto *recordingOpen = new wxButton(
    mRecordings,
    -1,
    "Browse...",
    wxDefaultPosition,
    wxSize(-1, mOptionsHeight)
  );
  recordingOpen->Bind(wxEVT_BUTTON, &Homepage::onRecordingOpen, this);
  recordingOpen->SetBitmap(wxArtProvider::GetBitmap(wxART_MENU));

  auto *vsizer = new wxBoxSizer(wxVERTICAL);
  vsizer->Add(label,         0, wxEXPAND);
  vsizer->Add(recordingOpen, 0);
  mRecordings->SetSizer(vsizer);
}

void Homepage::onProfileActivated(wxDataViewEvent &event)
{
  const auto &profileItem = event.GetItem();
  connectTo(profileItem);
  event.Skip();
}

void Homepage::onProfileSelected(wxDataViewEvent &event)
{
  const auto item = event.GetItem();
  if (!item.IsOk())
  {
    event.Skip();
    return;
  }

  event.Skip();
}

void Homepage::onRecordingOpen(wxCommandEvent &event)
{
  auto *recordingEvent = new Events::Recording(RECORDING_OPEN);
  wxQueueEvent(this, recordingEvent);
  event.Skip();
}

void Homepage::onConnectClicked(wxCommandEvent &/* event */)
{
  const auto item = mProfilesCtrl->GetSelection();
  if (!item.IsOk())
  {
    return;
  }

  const auto &brokerOptions = mProfilesModel->getBrokerOptions(item);

  mLogger->info(
    "Queueing event for profile at {}:{}",
    brokerOptions.getHostname(),
    brokerOptions.getPort()
  );

  auto *connectionEvent = new Events::Connection();
  connectionEvent->setProfile(item);
  wxQueueEvent(this, connectionEvent);
}

void Homepage::onNewProfileClicked(wxCommandEvent &/* event */)
{
  const auto item = mProfilesModel->createProfile();
  mProfilesCtrl->Select(item);
  mProfilesCtrl->EnsureVisible(item);
}

void Homepage::onProfileContext(wxDataViewEvent &event)
{
  wxMenu menu;

  if (event.GetItem().IsOk())
  {

    auto *connect = new wxMenuItem(
      nullptr,
      (unsigned)ContextIDs::ProfilesConnect,
      "Connect"
    );
    connect->SetBitmap(wxArtProvider::GetBitmap(wxART_TICK_MARK));
    menu.Append(connect);

    auto *edit = new wxMenuItem(
      nullptr,
      (unsigned)ContextIDs::ProfilesEdit,
      "Edit"
    );
    edit->SetBitmap(wxArtProvider::GetBitmap(wxART_EDIT));
    menu.Append(edit);

  }
  else
  {

    auto *create = new wxMenuItem(
      nullptr,
      (unsigned)ContextIDs::ProfilesCreate,
      "Create profile"
    );
    create->SetBitmap(wxArtProvider::GetBitmap(wxART_NEW));
    menu.Append(create);

  }

  PopupMenu(&menu);
}

void Homepage::onContextSelected(wxCommandEvent &event)
{
  switch ((ContextIDs)event.GetId())
  {
    case ContextIDs::ProfilesConnect: onProfileConnect(event); break;
    case ContextIDs::ProfilesCreate: onProfileCreate(event); break;
    case ContextIDs::ProfilesEdit: onProfileEdit(event); break;
  }
  event.Skip();
}

void Homepage::onProfileCreate(wxCommandEvent & /* event */)
{
  auto *newEvent = new Events::Profile(PROFILE_CREATE);
  wxQueueEvent(this, newEvent);
}

void Homepage::onProfileEdit(wxCommandEvent &event)
{
  (void)event;
  auto *newEvent = new Events::Profile(PROFILE_EDIT);
  newEvent->setProfile(mProfilesCtrl->GetSelection());
  wxQueueEvent(this, newEvent);
}

void Homepage::onProfileConnect(wxCommandEvent & /* event */)
{
  connectTo(mProfilesCtrl->GetSelection());
}

void Homepage::onQuickConnect()
{
  const auto wxs = mQuickConnectUrl->GetValue();
  const auto utf8 = wxs.ToUTF8();
  const std::string url(utf8.data(), utf8.length());

  mProfilesModel->updateQuickConnect(url);

  const auto &profileItem = mProfilesModel->getQuickConnect();
  const auto &brokerOptions = mProfilesModel->getBrokerOptions(profileItem);
  mLogger->info(
    "Queueing event for profile at {}:{}",
    brokerOptions.getHostname(),
    brokerOptions.getPort()
  );

  auto *connectionEvent = new Events::Connection();
  connectionEvent->setProfile(profileItem);
  wxQueueEvent(this, connectionEvent);
}

void Homepage::connectTo(wxDataViewItem profile)
{
  const auto &brokerOptions = mProfilesModel->getBrokerOptions(profile);
  mLogger->info(
    "Queueing event for profile at {}:{}",
    brokerOptions.getHostname(),
    brokerOptions.getPort()
  );

  auto *connectionEvent = new Events::Connection();
  connectionEvent->setProfile(profile);
  wxQueueEvent(this, connectionEvent);
}

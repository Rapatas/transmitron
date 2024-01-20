#include <chrono>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/wx.h>
#include <wx/artprov.h>
#include <wx/propgrid/propgrid.h>

#include "Homepage.hpp"
#include "Common/Log.hpp"
#include "Transmitron/Events/Recording.hpp"
#include "Transmitron/Models/Layouts.hpp"
#include "Transmitron/Types/ClientOptions.hpp"

using namespace Transmitron::Tabs;
using namespace Transmitron;
using namespace Transmitron::Events;

wxDEFINE_EVENT(Events::CONNECTION, Events::Connection); // NOLINT
wxDEFINE_EVENT(Events::RECORDING_OPEN, Events::Recording); // NOLINT

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

  auto *center = new wxPanel(this, wxID_ANY);

  setupRecordings(this);
  setupProfiles(center);
  setupProfileButtons(this);

  auto *line = new wxStaticLine(this);

  auto *hsizer = new wxBoxSizer(wxHORIZONTAL);
  hsizer->Add(mProfiles, 1, wxEXPAND);
  center->SetSizer(hsizer);

  auto *vsizer = new wxBoxSizer(wxVERTICAL);
  vsizer->Add(center, 1, wxEXPAND);
  vsizer->Add(mProfileButtons, 0, wxEXPAND);
  vsizer->Add(line, 0, wxEXPAND);
  vsizer->Add(mRecordings, 0, wxEXPAND);
  this->SetSizer(vsizer);
  vsizer->Layout();

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

  auto *label = new wxStaticText(mProfiles, -1, "Profiles");
  label->SetFont(mLabelFont);

  mProfilesCtrl = new wxDataViewCtrl(mProfiles, -1);
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

void Homepage::setupRecordings(wxPanel *parent)
{
  mRecordings = new wxPanel(parent, -1);

  auto *label = new wxStaticText(mRecordings, -1, "Recordings");
  label->SetFont(mLabelFont);

  auto *recordingOpen = new wxButton(
    mRecordings,
    -1,
    "Open Recording",
    wxDefaultPosition,
    wxSize(-1, mOptionsHeight)
  );
  recordingOpen->Bind(wxEVT_BUTTON, &Homepage::onRecordingOpen, this);
  recordingOpen->SetBitmap(wxArtProvider::GetBitmap(wxART_MENU));

  auto *vsizer = new wxBoxSizer(wxVERTICAL);
  vsizer->Add(label,         0, wxEXPAND);
  vsizer->Add(recordingOpen, 0, wxEXPAND);
  mRecordings->SetSizer(vsizer);
}

void Homepage::setupProfileButtons(wxPanel *parent)
{
  mProfileButtons = new wxPanel(parent);

  auto *newProfile = new wxButton(
    mProfileButtons,
    -1,
    "New Profile",
    wxDefaultPosition,
    wxSize(-1, mOptionsHeight)
  );
  newProfile->SetBitmap(wxArtProvider::GetBitmap(wxART_NEW));
  newProfile->Bind(wxEVT_BUTTON, &Homepage::onNewProfileClicked, this);


  mConnect = new wxButton(
    mProfileButtons,
    -1,
    "Connect",
    wxDefaultPosition,
    wxSize(-1, mOptionsHeight)
  );
  mConnect->Enable(false);
  mConnect->Bind(wxEVT_BUTTON, &Homepage::onConnectClicked, this);
  mConnect->SetBitmap(wxArtProvider::GetBitmap(wxART_TICK_MARK));

  mProfileButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
  mProfileButtonsSizer->SetMinSize(0, mOptionsHeight);
  mProfileButtonsSizer->Add(newProfile, 0, wxEXPAND);
  mProfileButtonsSizer->AddStretchSpacer(1);
  mProfileButtonsSizer->Add(mConnect, 0, wxEXPAND);
  mProfileButtons->SetSizer(mProfileButtonsSizer);
}

void Homepage::onProfileActivated(wxDataViewEvent &e)
{
  const auto &profileItem = e.GetItem();
  const auto &brokerOptions = mProfilesModel->getBrokerOptions(profileItem);
  mLogger->info(
    "Queueing event for profile at {}:{}",
    brokerOptions.getHostname(),
    brokerOptions.getPort()
  );

  auto *connectionEvent = new Events::Connection();
  connectionEvent->setProfile(profileItem);
  wxQueueEvent(this, connectionEvent);

  e.Skip();
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

void Homepage::onProfileContext(wxDataViewEvent &e)
{
  if (!e.GetItem().IsOk())
  {
    e.Skip();
    return;
  }

  wxMenu menu;

  auto *del = new wxMenuItem(
    nullptr,
    (unsigned)ContextIDs::ProfilesDelete,
    "Delete"
  );
  del->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE));
  menu.Append(del);

  PopupMenu(&menu);
}

void Homepage::onContextSelected(wxCommandEvent &e)
{
  switch ((ContextIDs)e.GetId())
  {
    case ContextIDs::ProfilesDelete: onProfileDelete(e); break;
  }
  e.Skip();
}

void Homepage::onProfileDelete(wxCommandEvent & /* event */)
{
  const auto item = mProfilesCtrl->GetSelection();
  mProfilesModel->remove(item);
}

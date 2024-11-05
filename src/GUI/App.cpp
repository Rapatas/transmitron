#include "App.hpp"

#include <fstream>

#include <fmt/core.h>
#include <wx/artprov.h>
#include <wx/aui/auibook.h>
#include <wx/image.h>
#include <wx/notebook.h>
#include <wx/settings.h>
#include <wx/window.h>

#include "Common/Filesystem.hpp"
#include "Common/Helpers.hpp"
#include "Common/Info.hpp"
#include "Common/Log.hpp"
#include "Common/Url.hpp"
#include "Common/XdgBaseDir.hpp"
#include "Events/Connection.hpp"
#include "GUI/Events/Profile.hpp"
#include "GUI/Events/Recording.hpp"
#include "GUI/Models/History.hpp"
#include "GUI/Models/Layouts.hpp"
#include "GUI/Models/Subscriptions.hpp"
#include "Tabs/Client.hpp"
#include "Tabs/Homepage.hpp"
#include "Tabs/Settings.hpp"

using namespace Rapatas::Transmitron;
using namespace GUI;
using namespace Common;

constexpr size_t DefaultWindowHeight = 576;
constexpr size_t DefaultWindowWidth = 1024;
constexpr size_t MinWindowWidth = 550;
constexpr size_t MinWindowHeight = 400;
constexpr size_t LabelFontSize = 15;

App::App(bool verbose) :
  LabelFontInfo(LabelFontSize) //
{
  Common::Log::instance().initialize(verbose);
  mLogger = Common::Log::create("GUI::App");
}

bool App::OnInit() {
  wxImage::AddHandler(new wxPNGHandler);
  wxImage::AddHandler(new wxICOHandler);

  mFrame = new wxFrame(
    nullptr,
    -1,
    "Homepage - Transmitron",
    wxDefaultPosition,
    wxSize(DefaultWindowWidth, DefaultWindowHeight)
  );
  mFrame->SetMinSize(wxSize(MinWindowWidth, MinWindowHeight));
  calculateOptions();

  auto prefix = getInstallPrefix();
  const auto base = prefix.append("share/transmitron");
  mArtProvider.initialize(base, {mIconHeight, mIconHeight}, mDarkMode);

  setupIcon();

  const int noteStyle = wxAUI_NB_DEFAULT_STYLE
    & ~(0U | wxAUI_NB_TAB_MOVE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxAUI_NB_TAB_SPLIT
    );

  mNote = new wxAuiNotebook(
    mFrame,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    noteStyle
  );

  const auto configDir = createConfigDir().string();
  const auto cacheDir = createCacheDir().string();

  mLayoutsModel = new Models::Layouts();
  mLayoutsModel->load(configDir);

  mProfilesModel = new Models::Profiles(mLayoutsModel, mArtProvider);
  mProfilesModel->load(configDir, cacheDir);

  createSettingsTab();

  createHomepageTab(1);

  auto *newProfilesTab = new wxPanel(mNote);
  mNote->AddPage(newProfilesTab, "", false, mArtProvider.bitmap(Icon::Add));
  ++mCount;

  mNote->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGING, &App::onPageSelected, this);
  mNote->Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &App::onPageClosing, this);

  mFrame->Show();
  return true;
}

int App::FilterEvent(wxEvent &event) {
  if (event.GetEventType() != wxEVT_KEY_DOWN) {
    return wxEventFilter::Event_Skip;
  }

  const auto keyEvent = dynamic_cast<wxKeyEvent &>(event);

  if (keyEvent.GetKeyCode() == 'W' && keyEvent.ControlDown()) {
    onKeyDownControlW();
    return wxEventFilter::Event_Processed;
  }

  if (keyEvent.GetKeyCode() == 'T' && keyEvent.ControlDown()) {
    onKeyDownControlT();
    return wxEventFilter::Event_Processed;
  }

  return wxEventFilter::Event_Skip;
}

bool App::openProfile(const std::string &profileName) {
  const auto profileItem = mProfilesModel->getItemFromName(profileName);
  if (!profileItem.IsOk()) {
    mLogger->warn("Profile '{}' not found", profileName);
    return false;
  }

  openProfile(profileItem);
  return true;
}

void App::onPageSelected(wxBookCtrlEvent &event) {
  const auto selection = static_cast<size_t>(event.GetSelection());

  if (event.GetSelection() == wxNOT_FOUND) {
    event.Skip();
    return;
  }

  if (selection == mCount - 1) {
    createHomepageTab(mCount - 1);
    event.Veto();
    return;
  }

  const auto style = mNote->GetWindowStyle();
  constexpr int Closable = 0U | wxAUI_NB_CLOSE_BUTTON
    | wxAUI_NB_CLOSE_ON_ACTIVE_TAB;

  // NOLINTBEGIN(hicpp-signed-bitwise)
  if (selection == 0) {
    if ((style & Closable) != 0) { mNote->SetWindowStyle(style & ~Closable); }
  } else {
    if ((style & Closable) != Closable) {
      mNote->SetWindowStyle(style | Closable);
    }
    auto *page = mNote->GetPage(selection);
    auto *home = dynamic_cast<Tabs::Homepage *>(page);
    if (home != nullptr) { home->focus(); }
  }
  // NOLINTEND(hicpp-signed-bitwise)

  const auto windowName = mNote->GetPage(selection)->GetName();
  const auto title = wxString::FromUTF8(windowName).Append(" - Transmitron");
  mFrame->SetTitle(title);

  event.Skip();
}

void App::onPageClosing(wxBookCtrlEvent &event) {
  const auto selection = static_cast<size_t>(event.GetSelection());
  const auto previous = static_cast<size_t>(event.GetOldSelection());

  // Settings and Plus.
  if (selection == 0 || selection == mCount - 1) {
    event.Veto();
    return;
  }

  --mCount;

  if (mCount == 2) { createHomepageTab(mCount - 1); }

  if (previous == mCount - 1) { mNote->ChangeSelection(mCount - 2); }

  event.Skip();
}

void App::onKeyDownControlW() {
  const auto selection = static_cast<size_t>(mNote->GetSelection());

  // Settings and Plus.
  if (selection == 0 || selection == mCount - 1) { return; }

  mNote->RemovePage(selection);

  --mCount;

  if (mCount == 2) { createHomepageTab(mCount - 1); }

  if (selection == mCount - 1) { mNote->ChangeSelection(mCount - 2); }
}

void App::onKeyDownControlT() { createHomepageTab(mCount - 1); }

void App::onRecordingSave(Events::Recording &event) {
  const auto name = event.getName();
  const auto nameUtf8 = name.ToUTF8();
  const std::string nameStr(nameUtf8.data(), nameUtf8.length());
  mLogger->info("Storing recording for {}", nameStr);
  const std::string encoded = Url::encode(nameStr);

  const auto now = std::chrono::system_clock::now();
  const auto timestamp = Helpers::timeToFilename(now);
  const auto filename = encoded + "-" + timestamp + ".tmrc";

  wxFileDialog saveFileDialog(
    mFrame,
    _("Save TMRC file"),
    "",
    filename,
    "TMRC files (*.tmrc)|*.tmrc",
    wxFD_SAVE | wxFD_OVERWRITE_PROMPT
  );

  if (saveFileDialog.ShowModal() == wxID_CANCEL) { return; }

  const auto filepath = saveFileDialog.GetPath().ToStdString();
  std::ofstream out(filepath);
  if (!out.is_open()) {
    mLogger->error("Cannot save current contents in file '{}'.", filepath);
    return;
  }

  out << event.getContents();
}

void App::onProfileCreate(Events::Profile &event) {
  (void)event;
  mNote->ChangeSelection(0);
  mSettingsTab->createProfile();
}

void App::onProfileEdit(Events::Profile &event) {
  mNote->ChangeSelection(0);
  mSettingsTab->selectProfile(event.getProfile());
}

void App::onRecordingOpen(Events::Recording & /* event */) {
  wxFileDialog openFileDialog(
    mFrame,
    _("Open tmrc file"),
    "",
    "",
    "TMRC files (*.tmrc)|*.tmrc",
    wxFD_OPEN | wxFD_FILE_MUST_EXIST
  );

  if (openFileDialog.ShowModal() == wxID_CANCEL) { return; }

  const auto filename = openFileDialog.GetPath();
  const auto utf8 = filename.ToUTF8();
  const std::string encodedStr(utf8.data(), utf8.length());
  openRecording(encodedStr);
}

void App::createHomepageTab(size_t index) {
  auto *homepage = new Tabs::Homepage(
    mNote,
    mArtProvider,
    LabelFontInfo,
    mOptionsHeight,
    mProfilesModel,
    mLayoutsModel
  );
  mNote->InsertPage(index, homepage, "Homepage");
  ++mCount;
  mNote->SetSelection(index);
  homepage->focus();

  homepage->Bind(Events::RECORDING_OPEN, &App::onRecordingOpen, this);
  homepage->Bind(Events::PROFILE_CREATE, &App::onProfileCreate, this);
  homepage->Bind(Events::PROFILE_EDIT, &App::onProfileEdit, this);

  homepage->Bind(
    Events::CONNECTION_REQUESTED,
    [this](Events::Connection event) {
      if (event.GetSelection() == wxNOT_FOUND) {
        event.Skip();
        return;
      }

      const auto profileItem = event.getProfile();
      openProfile(profileItem);
    }
  );
}

void App::createSettingsTab() {
  mSettingsTab = new Tabs::Settings(
    mNote,
    mArtProvider,
    LabelFontInfo,
    mOptionsHeight,
    mProfilesModel,
    mLayoutsModel
  );
  mSettingsTab->Bind(
    Events::CONNECTION_REQUESTED,
    [this](Events::Connection event) {
      if (event.GetSelection() == wxNOT_FOUND) {
        event.Skip();
        return;
      }

      const auto profileItem = event.getProfile();
      openProfile(profileItem);
    }
  );
  mNote->AddPage(mSettingsTab, "", false, mArtProvider.bitmap(Icon::Settings));
  ++mCount;
}

Common::fs::path App::createConfigDir() {
  const auto configHome = Common::XdgBaseDir::configHome();
  const auto config = fmt::format(
    "{}/{}",
    configHome.string(),
    Info::getProjectName()
  );

  if (!fs::is_directory(config) || !fs::exists(config)) {
    if (!fs::create_directory(config)) {
      mLogger->warn("Could not create directory '%s'", config);
      return {};
    }
  }

  mLogger->info("Config dir: {}", config);

  return config;
}

Common::fs::path App::createCacheDir() {
  const auto configHome = Common::XdgBaseDir::cacheHome();
  const auto config = fmt::format(
    "{}/{}",
    configHome.string(),
    Info::getProjectName()
  );

  if (!fs::is_directory(config) || !fs::exists(config)) {
    if (!fs::create_directory(config)) {
      mLogger->warn("Could not create directory '%s'", config);
      return {};
    }
  }

  mLogger->info("Cache dir:  {}", config);

  return config;
}

void App::setupIcon() {
  Common::fs::path path = fmt::format(
    "{}/share/transmitron/transmitron.ico",
    getInstallPrefix().string()
  );
  path.make_preferred();

  const auto bundle = wxIconBundle(path.string());
  if (!bundle.IsOk()) {
    mLogger->debug("Could not load icon: {}", path.string());
    return;
  }
  mFrame->SetIcons(bundle);
}

Common::fs::path App::getExecutablePath() {
  const auto pathfinder = []() {
#if defined WIN32
    std::array<wchar_t, MAX_PATH> moduleFileName{};
    const auto r = GetModuleFileNameW(nullptr, moduleFileName.data(), MAX_PATH);
    if (r == 0) {
      throw std::runtime_error("Could not determine installation prefix");
    }

    return std::wstring(moduleFileName.data(), moduleFileName.size());
#else
    return Common::fs::read_symlink("/proc/self/exe");
#endif
  };

  static const Common::fs::path path = pathfinder();
  mLogger->info("Executable path: {}", path.string());
  return path;
}

Common::fs::path App::getInstallPrefix() {
  const Common::fs::path executable = getExecutablePath();
  const auto executableDir = executable.parent_path();
  auto prefix = executableDir.parent_path();
  return prefix;
}

void App::openProfile(wxDataViewItem item) {
  const auto options = mProfilesModel->getBrokerOptions(item);

  auto *client = new Tabs::Client(
    mNote,
    options,
    mProfilesModel->getClientOptions(item),
    mProfilesModel->getMessagesModel(item),
    mProfilesModel->getTopicsSubscribed(item),
    mProfilesModel->getTopicsPublished(item),
    mLayoutsModel,
    mProfilesModel->getName(item),
    mArtProvider,
    mDarkMode,
    mOptionsHeight
  );

  client->Bind(Events::RECORDING_SAVE, &App::onRecordingSave, this);

  const auto selection = static_cast<size_t>(mNote->GetSelection());
  const auto target = selection == 0 ? mCount - 1 : selection;

  mLogger->info("Target: {}", target);

  if (selection != 0) {
    // Remove homepage.
    mNote->RemovePage(selection);
  } else {
    ++mCount;
  }

  mNote->InsertPage(target, client, "");
  mNote->SetSelection(target);

  const auto name = mProfilesModel->getName(item);
  const auto utf8 = wxString::FromUTF8(name);
  mNote->SetPageText(target, utf8);

  client->focus();
}

void App::openRecording(const std::string &filename) {
  const Common::fs::path path(filename);
  const auto filenameStr = path.stem().string();
  const std::string decodedStr = Url::decode(filenameStr);

  wxObjectDataPtr<Models::Subscriptions> subscriptions;
  subscriptions = new Models::Subscriptions();
  const auto subscriptionsLoaded = subscriptions->load(filename);

  wxObjectDataPtr<Models::History> history;
  history = new Models::History(subscriptions);
  const auto historyLoaded = history->load(filename);

  if (!subscriptionsLoaded || !historyLoaded) {
    mLogger->warn("Could not load recording: '{}'", filename);
    return;
  }

  auto *client = new Tabs::Client(
    mNote,
    history,
    subscriptions,
    mLayoutsModel,
    decodedStr,
    mArtProvider,
    mDarkMode,
    mOptionsHeight
  );

  const auto selection = static_cast<size_t>(mNote->GetSelection());
  mNote->RemovePage(selection);
  mNote->InsertPage(selection, client, "");
  mNote->SetSelection(selection);
  mNote->SetPageText(selection, decodedStr);

  client->focus();
}

void App::calculateOptions() {
  auto *button = new wxButton(mFrame, wxID_ANY, "Hello");

  mOptionsHeight = button->GetBestSize().y;
  // mOptionsHeight = mFrame->FromDIP(wxSize(26, 26)).y;

#ifdef _WIN32
  // NOLINTNEXTLINE
  mIconHeight = mOptionsHeight - mFrame->FromDIP(wxSize(4, 4)).y;
#else
  // NOLINTNEXTLINE
  mIconHeight = mOptionsHeight - mFrame->FromDIP(wxSize(6, 6)).y;
#endif // _WIN32

  mLogger->info("button={} icon={}", mOptionsHeight, mIconHeight);

  mFrame->RemoveChild(button);
  button->Destroy();

  const auto appearance = wxSystemSettings::GetAppearance();
  mDarkMode = appearance.IsDark() || appearance.IsUsingDarkBackground();
}

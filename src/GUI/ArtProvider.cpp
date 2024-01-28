#include <wx/artprov.h>
#include <wx/rawbmp.h>

#include "GUI/ArtProvider.hpp"
#include "Common/Log.hpp"
#include "Common/Filesystem.hpp"

using namespace Rapatas::Transmitron::GUI;
using namespace Rapatas::Transmitron::Common;

constexpr uint8_t ByteMax = 255;

ArtProvider::ArtProvider() = default;

void ArtProvider::initialize(const fs::path &base, wxSize size, bool dark)
{
  mPlaceholder = wxArtProvider::GetBitmap(wxART_PLUS);
  mLogger = Log::create("ArtProvider");

  mLogger->info("Loading icons with dimensions: {}x{}", size.x, size.y);

  static const std::map<Icon, fs::path> paths
  {

    {Icon::Add,           "add.svg"},
    {Icon::Archive,       "inventory_2.svg"},
    {Icon::Cancel,        "cancel.svg"},
    {Icon::Clear,         "clear_all.svg"},
    {Icon::Connect,       "arrow_forward.svg"},
    {Icon::Connected,     "task_alt.svg"},
    {Icon::Connecting,    "pending.svg"},
    {Icon::Copy,          "content_copy.svg"},
    {Icon::Delete,        "delete.svg"},
    {Icon::Disconnected,  "hide_source.svg"},
    {Icon::Edit,          "edit.svg"},
    {Icon::File,          "draft.svg"},
    {Icon::FileFull,      "description.svg"},
    {Icon::Folder,        "folder.svg"},
    {Icon::History,       "history.svg"},
    {Icon::Home,          "home.svg"},
    {Icon::Mute,          "volume_off.svg"},
    {Icon::NewColor,      "palette.svg"},
    {Icon::NewDir,        "create_new_folder.svg"},
    {Icon::NewFile,       "note_add.svg"},
    {Icon::NewProfile,    "person_add.svg"},
    {Icon::Profile,       "person.svg"},
    {Icon::Publish,       "send.svg"},
    {Icon::Save,          "save.svg"},
    {Icon::SaveAs,        "save_as.svg"},
    {Icon::Search,        "search.svg"},
    {Icon::Settings,      "settings.svg"},
    {Icon::Solo,          "music_note.svg"},
    {Icon::Subscribe,     "notification_add.svg"},
    {Icon::Subscriptions, "notifications.svg"},
    {Icon::Unmute,        "volume_up.svg"},
    {Icon::Unsubscribe,   "notifications_off.svg"},

  };

  for (const auto &[icon, relative] : paths)
  {
    const auto fullpath = base / relative;
    if (!fs::exists(fullpath))
    {
      mLogger->warn("Could not load icon: {}", fullpath.string());
      continue;
    }
    auto bundle = wxBitmapBundle::FromSVGFile(fullpath.string(), size);
    if (!bundle.IsOk())
    {
      mLogger->warn("Could not load icon: {}", fullpath.string());
      continue;
    }
    auto bitmap = bundle.GetBitmap(size);

    if (dark)
    {
      wxAlphaPixelData bmdata(bitmap);
      wxAlphaPixelData::Iterator dst(bmdata);

      // NOLINTNEXTLINE(readability-identifier-length)
      for(int y = 0; y < bitmap.GetHeight(); ++y)
      {
        dst.MoveTo(bmdata, 0, y);
        // NOLINTNEXTLINE(readability-identifier-length)
        for(int x = 0; x < bitmap.GetWidth(); ++x)
        {
          const unsigned char alpha = dst.Alpha();
          const unsigned char blue  = dst.Blue() + 220;
          const unsigned char red   = dst.Red() + 220;
          const unsigned char green = dst.Green() + 220;
          dst.Blue()  = static_cast<uint8_t>(blue  * alpha / ByteMax);
          dst.Green() = static_cast<uint8_t>(red   * alpha / ByteMax);
          dst.Red()   = static_cast<uint8_t>(green * alpha / ByteMax);
          dst++;
        }
      }
    }

    mIcons.insert({icon, bitmap});
  }
}

const wxBitmap &ArtProvider::bitmap(Icon icon) const
{
  const auto it = mIcons.find(icon);
  if (it == mIcons.end()) { return mPlaceholder; }
  return it->second;
}

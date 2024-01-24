#include <wx/artprov.h>
#include <wx/rawbmp.h>

#define NANOSVG_ALL_COLOR_KEYWORDS
#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#include <nanosvg/nanosvg.h>
#include <nanosvg/nanosvgrast.h>

#include "GUI/ArtProvider.hpp"
#include "Common/Log.hpp"

using namespace Rapatas::Transmitron::GUI;
using namespace Rapatas::Transmitron::Common;

constexpr uint8_t ByteMax = 255;
constexpr float DPI = 96.0F;

ArtProvider::ArtProvider() = default;

void ArtProvider::initialize(const fs::path &base, wxSize size, bool dark)
{
  mPlaceholder = wxArtProvider::GetBitmap(wxART_EDIT);
  mLogger = Log::create("ArtProvider");

  mLogger->info("Loading icons with dimensions: {}x{}", size.x, size.y);

  static const std::map<Icon, fs::path> paths
  {

    {Icon::Add,           "add.svg"},
    {Icon::Archive,       "archive.svg"},
    {Icon::Clear,         "clear_all.svg"},
    {Icon::File,          "draft.svg"},
    {Icon::Folder,        "folder.svg"},
    {Icon::FileFull,      "description.svg"},
    {Icon::Profile,       "person.svg"},
    {Icon::NewProfile,    "person_add.svg"},
    {Icon::Cancel,        "cancel.svg"},
    {Icon::Connect,       "cable.svg"},
    {Icon::Copy,          "content_copy.svg"},
    {Icon::Delete,        "delete.svg"},
    {Icon::Edit,          "edit.svg"},
    {Icon::History,       "history.svg"},
    {Icon::Home,          "home.svg"},
    {Icon::Mute,          "volume_off.svg"},
    {Icon::Unmute,        "volume_up.svg"},
    {Icon::NewColor,      "palette.svg"},
    {Icon::NewDir,        "create_new_folder.svg"},
    {Icon::NewFile,       "note_add.svg"},
    {Icon::Preview,       "note_add.svg"},
    {Icon::Publish,       "send.svg"},
    {Icon::Save,          "save.svg"},
    {Icon::SaveAs,        "save_as.svg"},
    {Icon::Search,        "search.svg"},
    {Icon::Settings,      "settings.svg"},
    {Icon::Solo,          "music_note.svg"},
    {Icon::Subscribe,     "notification_add.svg"},
    {Icon::Subscriptions, "notifications.svg"},
    {Icon::Unsubscribe,   "notifications_off.svg"},

  };

  for (const auto &[icon, relative] : paths)
  {
    const auto fullpath = base / relative;
    auto bitmap = parseSVG(fullpath, size, dark);
    if (bitmap)
    {
      mIcons.insert({icon, std::move(bitmap.value())});
    }
  }
}

const wxBitmap &ArtProvider::bitmap(Icon icon) const
{
  const auto it = mIcons.find(icon);
  if (it == mIcons.end()) { return mPlaceholder; }
  return it->second;
}

std::optional<wxBitmap> ArtProvider::parseSVG(
  const Common::fs::path &filename,
  wxSize size,
  bool dark
) {
  if (!std::filesystem::exists(filename))
  {
    mLogger->warn("Could not load Icon {}: not found", filename.string());
    return std::nullopt;
  }

  auto *svg = nsvgParseFromFile(filename.string().data(), "px", DPI);
  if (svg == nullptr)
  {
    mLogger->warn("Could not load Icon {}: malformed SVG", filename.string());
    return std::nullopt;;
  }

  const double ratioSvg = svg->width / svg->height;
  const double ratioReq = size.x / static_cast<double>(size.y);

  const auto renderWidth = (ratioSvg > ratioReq)
    ? size.x
    : size.y * ratioSvg;
  const auto renderHeight = (ratioSvg > ratioReq)
    ? size.x * ratioSvg
    : size.y;

  const auto scale = renderWidth / svg->width;

  NSVGrasterizer *rast = nsvgCreateRasterizer();
  const auto len = 4 * static_cast<size_t>(renderWidth * renderHeight);
  std::vector<uint8_t> buffer(len);

  nsvgRasterize(
    rast,
    svg,
    0,
    0,
    static_cast<float>(scale),
    buffer.data(),
    static_cast<int>(renderWidth),
    static_cast<int>(renderHeight),
    static_cast<int>(renderWidth * 4)
  );

  wxBitmap result;
  const auto created = result.Create(
    static_cast<int>(renderWidth),
    static_cast<int>(renderHeight),
    32
  );
  if (!created || !result.IsOk())
  {
    mLogger->warn(
      "Could not load Icon {}: bitmap not allocated (height={} width={})",
      filename.string(),
      renderWidth,
      renderHeight
    );
    return std::nullopt;;
  }

  wxAlphaPixelData bmdata(result);
  wxAlphaPixelData::Iterator dst(bmdata);

  size_t index = 0;
  // NOLINTNEXTLINE(readability-identifier-length)
  for(int y = 0; y < result.GetHeight(); ++y)
  {
    dst.MoveTo(bmdata, 0, y);
    // NOLINTNEXTLINE(readability-identifier-length)
    for(int x = 0; x < result.GetWidth(); ++x)
    {
      const unsigned char alpha = buffer[index + 3];
      const unsigned char blue  = dark ? buffer[index + 2] + 220 : buffer[index + 2];
      const unsigned char red   = dark ? buffer[index + 1] + 220 : buffer[index + 1];
      const unsigned char green = dark ? buffer[index + 0] + 220 : buffer[index + 0];
      dst.Blue()  = static_cast<uint8_t>(blue  * alpha / ByteMax);
      dst.Green() = static_cast<uint8_t>(red   * alpha / ByteMax);
      dst.Red()   = static_cast<uint8_t>(green * alpha / ByteMax);
      dst.Alpha() = alpha;
      dst++;
      index += 4;
    }
  }

  nsvgDeleteRasterizer(rast);
  nsvgDelete(svg);

  return result;
}

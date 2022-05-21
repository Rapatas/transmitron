#include <chrono>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <sstream>
#include <tinyxml2.h>
#include <wx/clipbrd.h>
#include <wx/artprov.h>
#include <wx/sizer.h>
#include <wx/stc/stc.h>
#include "Edit.hpp"
#include "Transmitron/Events/Edit.hpp"
#include "Transmitron/Resources/send/send-18x18.hpp"
#include "Transmitron/Resources/pin/pinned-18x18.hpp"
#include "Transmitron/Resources/pin/not-pinned-18x18.hpp"
#include "Transmitron/Resources/qos/qos-0.hpp"
#include "Transmitron/Resources/qos/qos-1.hpp"
#include "Transmitron/Resources/qos/qos-2.hpp"

using namespace nlohmann;
using namespace Transmitron;
using namespace Transmitron::Widgets;

wxDEFINE_EVENT(Events::EDIT_PUBLISH, Events::Edit); // NOLINT
wxDEFINE_EVENT(Events::EDIT_SAVE_SNIPPET, Events::Edit); // NOLINT

constexpr uint8_t ByteSize = std::numeric_limits<uint8_t>::digits;

Edit::Edit(
  wxWindow* parent,
  wxWindowID id,
  int optionsHeight,
  bool darkMode
) :
  wxPanel(parent, id),
  mStyles(initThemes()),
  mTheme(darkMode ? Theme::Dark : Theme::Light),
  mOptionsHeight(optionsHeight),
  mTop(new wxBoxSizer(wxOrientation::wxHORIZONTAL)),
  mVsizer(new wxBoxSizer(wxOrientation::wxVERTICAL)),
  mBottom(new wxBoxSizer(wxOrientation::wxHORIZONTAL)),
  mTopic(new TopicCtrl(this, -1)),
  mRetained(false),
  mQoS(MQTT::QoS::AtLeastOnce)
{
  constexpr size_t FontSize = 9;
  mFont = wxFont(wxFontInfo(FontSize).FaceName("Consolas"));

  const int timestampBorderPx = 5;
  mInfoLine = new wxStaticText(this, -1, "0000-00-00 00:00:00.000");
  mInfoLine->SetFont(mFont);

  mTopic->Bind(Events::TOPICCTRL_RETURN, &Edit::onTopicCtrlReturn, this);

  mPublish = new wxBitmapButton(
    this,
    -1,
    *bin2cSend18x18(),
    wxDefaultPosition,
    wxSize(mOptionsHeight, mOptionsHeight)
  );
  mPublish->Bind(wxEVT_BUTTON, [this](wxCommandEvent &/* event */){
    auto *e = new Events::Edit(Events::EDIT_PUBLISH);
    wxQueueEvent(this, e);
  });

  setupScintilla();

  mSaveSnippet = new wxButton(
    this,
    -1,
    "",
    wxDefaultPosition,
    wxSize(mOptionsHeight, mOptionsHeight)
  );
  mSaveSnippet->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE));
  mSaveSnippet->Bind(wxEVT_BUTTON, [this](wxCommandEvent &/* event */){
    auto *e = new Events::Edit(Events::EDIT_SAVE_SNIPPET);
    wxQueueEvent(this, e);
  });

  auto *formatLabel = new wxStaticText(
    this,
    -1,
    "Format: ",
    wxDefaultPosition
  );

  constexpr size_t FormatButtonWidth = 100;

  mFormatSelect = new wxComboBox(
    this,
    -1,
    mFormats.begin()->first,
    wxDefaultPosition,
    wxSize(FormatButtonWidth, mOptionsHeight)
  );
  for (const auto &format : mFormats)
  {
    mFormatSelect->Insert(format.first, 0);
  }

  mTop->SetMinSize(0, mOptionsHeight);
  mBottom->SetMinSize(0, mOptionsHeight);

  mRetainedFalse = new wxStaticBitmap(this, -1, *bin2cNotPinned18x18());
  mRetainedFalse->SetToolTip("Not retained");
  mRetainedFalse->Bind(wxEVT_LEFT_UP, &Edit::onRetainedClicked, this);
  mRetainedTrue = new wxStaticBitmap(this, -1, *bin2cPinned18x18());
  mRetainedTrue->SetToolTip("Retained");
  mRetainedTrue->Bind(wxEVT_LEFT_UP, &Edit::onRetainedClicked, this);

  mRetainedTrue->Hide();

  mQos0 = new wxStaticBitmap(this, -1, *bin2cQos0());
  mQos0->SetToolTip("QoS: 0");
  mQos0->Bind(wxEVT_LEFT_UP, &Edit::onQosClicked, this);

  mQos1 = new wxStaticBitmap(this, -1, *bin2cQos1());
  mQos1->SetToolTip("QoS: 1");
  mQos1->Bind(wxEVT_LEFT_UP, &Edit::onQosClicked, this);

  mQos2 = new wxStaticBitmap(this, -1, *bin2cQos2());
  mQos2->SetToolTip("QoS: 2");
  mQos2->Bind(wxEVT_LEFT_UP, &Edit::onQosClicked, this);

  mQos1->Hide();
  mQos2->Hide();

  mFormatSelect->Bind(wxEVT_COMBOBOX, &Edit::onFormatSelected, this);

  mTop->Add(mTopic, 1, wxEXPAND);
  mTop->AddSpacer(2);
  mTop->Add(mRetainedTrue, 0, wxEXPAND);
  mTop->Add(mRetainedFalse, 0, wxEXPAND);
  mTop->AddSpacer(2);
  mTop->Add(mQos0, 0, wxEXPAND);
  mTop->Add(mQos1, 0, wxEXPAND);
  mTop->Add(mQos2, 0, wxEXPAND);
  mTop->AddSpacer(2);
  mTop->Add(mPublish, 0, wxEXPAND);
  mBottom->Add(mSaveSnippet, 0, wxEXPAND);
  mBottom->AddStretchSpacer(1);
  mBottom->Add(formatLabel, 0, wxALIGN_CENTER_VERTICAL);
  mBottom->Add(mFormatSelect, 0, wxEXPAND);
  mVsizer->Add(mTop, 0, wxEXPAND);
  mVsizer->Add(mInfoLine, 0, (int)wxEXPAND | (int)wxLEFT, timestampBorderPx);
  mVsizer->Add(mText, 1, wxEXPAND);
  mVsizer->Add(mBottom, 0, wxEXPAND);

  SetSizer(mVsizer);
}

void Edit::setupScintilla()
{
  mText = new wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition);

  mText->StyleSetFont(wxSTC_STYLE_DEFAULT, mFont);
  mText->SetWrapMode(wxSTC_WRAP_WORD);

  constexpr int ScrollWidth = 50;
  mText->SetScrollWidth(ScrollWidth);
  mText->SetScrollWidthTracking(true);

  // Folding margins.
  constexpr int MarginFoldIndex = 1;
  constexpr int MarginFoldWidth = 20;
  mText->SetMarginType(MarginFoldIndex, wxSTC_MARGIN_SYMBOL);
  mText->SetMarginMask(MarginFoldIndex, (int)wxSTC_MASK_FOLDERS);
  mText->SetMarginWidth(MarginFoldIndex, MarginFoldWidth);
  mText->SetMarginSensitive(MarginFoldIndex, true);
  mText->SetAutomaticFold(wxSTC_AUTOMATICFOLD_CLICK);
  mText->SetFoldFlags(wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);

  // Line number margins.
  constexpr int MarginLineNumberIndex = 0;
  constexpr int MarginLineNumberWidth = 30;
  mText->SetMarginType(MarginLineNumberIndex, wxSTC_MARGIN_NUMBER);
  mText->SetMarginWidth(MarginLineNumberIndex, MarginLineNumberWidth);

  // Folding markers.
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDER,        wxSTC_MARK_PLUS);
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN,    wxSTC_MARK_MINUS);
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDEREND,     wxSTC_MARK_EMPTY);
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY);
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_EMPTY);
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB,     wxSTC_MARK_EMPTY);
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL,    wxSTC_MARK_EMPTY);

  // Constant styles.
  mText->SetCaretForeground(mStyles.at(mTheme).at(Style::Normal).first);
  mText->StyleSetForeground(wxSTC_STYLE_DEFAULT, mStyles.at(mTheme).at(Style::Normal).first);
  mText->StyleSetBackground(wxSTC_STYLE_DEFAULT, mStyles.at(mTheme).at(Style::Editor).second);
  mText->StyleClearAll();
}

void Edit::setStyle(Format format)
{
  if (mCurrentFormat == format)
  {
    return;
  }

  const auto &style = mStyles.at(mTheme);

  switch (format)
  {
    case Format::Json:
    {
      mText->SetLexer(wxSTC_LEX_JSON);
      mText->SetKeyWords(0, "true false null");

      mText->StyleSetForeground(wxSTC_JSON_OPERATOR,     style.at(Style::Normal).first);
      mText->StyleSetForeground(wxSTC_JSON_ERROR,        style.at(Style::Error).first);
      mText->StyleSetForeground(wxSTC_JSON_KEYWORD,      style.at(Style::Keyword).first);
      mText->StyleSetForeground(wxSTC_JSON_NUMBER,       style.at(Style::Number).first);
      mText->StyleSetForeground(wxSTC_JSON_PROPERTYNAME, style.at(Style::Key).first);
      mText->StyleSetForeground(wxSTC_JSON_STRING,       style.at(Style::String).first);
      mText->StyleSetForeground(wxSTC_JSON_URI,          style.at(Style::Uri).first);

      mText->StyleSetBackground(wxSTC_JSON_OPERATOR,     style.at(Style::Normal).second);
      mText->StyleSetBackground(wxSTC_JSON_ERROR,        style.at(Style::Error).second);
      mText->StyleSetBackground(wxSTC_JSON_KEYWORD,      style.at(Style::Keyword).second);
      mText->StyleSetBackground(wxSTC_JSON_NUMBER,       style.at(Style::Number).second);
      mText->StyleSetBackground(wxSTC_JSON_PROPERTYNAME, style.at(Style::Key).second);
      mText->StyleSetBackground(wxSTC_JSON_STRING,       style.at(Style::String).second);
      mText->StyleSetBackground(wxSTC_JSON_URI,          style.at(Style::Uri).second);
    }
    break;

    case Format::Xml:
    {
      mText->SetLexer(wxSTC_LEX_XML);

      mText->StyleSetForeground(wxSTC_H_ATTRIBUTE,        style.at(Style::Key).first);
      mText->StyleSetForeground(wxSTC_H_ATTRIBUTEUNKNOWN, style.at(Style::Error).first);
      mText->StyleSetForeground(wxSTC_H_COMMENT,          style.at(Style::Comment).first);
      mText->StyleSetForeground(wxSTC_H_DOUBLESTRING,     style.at(Style::String).first);
      mText->StyleSetForeground(wxSTC_H_NUMBER,           style.at(Style::Number).first);
      mText->StyleSetForeground(wxSTC_H_OTHER,            style.at(Style::Key).first);
      mText->StyleSetForeground(wxSTC_H_SINGLESTRING,     style.at(Style::String).first);
      mText->StyleSetForeground(wxSTC_H_TAG,              style.at(Style::Keyword).first);
      mText->StyleSetForeground(wxSTC_H_TAGEND,           style.at(Style::Keyword).first);
      mText->StyleSetForeground(wxSTC_H_TAGUNKNOWN,       style.at(Style::Error).first);
      mText->StyleSetForeground(wxSTC_H_XMLEND,           style.at(Style::Special).first);
      mText->StyleSetForeground(wxSTC_H_XMLSTART,         style.at(Style::Special).first);

      mText->StyleSetBackground(wxSTC_H_ATTRIBUTE,        style.at(Style::Key).second);
      mText->StyleSetBackground(wxSTC_H_ATTRIBUTEUNKNOWN, style.at(Style::Error).second);
      mText->StyleSetBackground(wxSTC_H_COMMENT,          style.at(Style::Comment).second);
      mText->StyleSetBackground(wxSTC_H_DOUBLESTRING,     style.at(Style::String).second);
      mText->StyleSetBackground(wxSTC_H_NUMBER,           style.at(Style::Number).second);
      mText->StyleSetBackground(wxSTC_H_OTHER,            style.at(Style::Key).second);
      mText->StyleSetBackground(wxSTC_H_SINGLESTRING,     style.at(Style::String).second);
      mText->StyleSetBackground(wxSTC_H_TAG,              style.at(Style::Keyword).second);
      mText->StyleSetBackground(wxSTC_H_TAGEND,           style.at(Style::Keyword).second);
      mText->StyleSetBackground(wxSTC_H_TAGUNKNOWN,       style.at(Style::Error).second);
      mText->StyleSetBackground(wxSTC_H_XMLEND,           style.at(Style::Special).second);
      mText->StyleSetBackground(wxSTC_H_XMLSTART,         style.at(Style::Special).second);
    }
    break;

    case Format::Text:
    default:
    {
      mText->SetLexer(wxSTC_LEX_NULL);
    }
    break;
  }

  // Setup folding.
  mText->SetProperty("fold", "1");
  mText->SetProperty("fold.html", "1");
  mText->SetProperty("fold.compact", "0");

  mCurrentFormat = format;
}

void Edit::setMessage(const MQTT::Message &message)
{
  setTopic(message.topic);
  setPayload(message.payload);
  setQos(message.qos);
  setRetained(message.retained);
  setTimestamp(message.timestamp);
}

void Edit::setPayload(const std::string &text)
{
  auto format = mFormatSelect->GetValue().ToStdString();
  if (mReadOnly)
  {
    mText->SetReadOnly(false);
  }
  auto payloadUtf8 = formatTry(text, mFormats.at(format));
  mText->SetText(
    wxString::FromUTF8(
      payloadUtf8.data(),
      payloadUtf8.length()
    )
  );
  if (mReadOnly)
  {
    mText->SetReadOnly(true);
  }
}

void Edit::setTimestamp(const std::chrono::system_clock::time_point &timestamp)
{
  const std::time_t timestampC = std::chrono::system_clock::to_time_t(timestamp);
  const std::tm timestampTm = *std::localtime(&timestampC);
  const std::string format = "%Y-%m-%d %H:%M:%S";
  std::stringstream ss;
  ss << std::put_time(&timestampTm, format.c_str());

  const auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(timestamp);
  const auto fraction = timestamp - seconds;
  const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(fraction).count();
  ss << "." << millis;

  mInfoLine->SetLabel(ss.str());
}

void Edit::setInfoLine(const std::string &info)
{
  mInfoLine->SetLabel(
    wxString::FromUTF8(
      info.data(),
      info.length()
    )
  );
}

void Edit::setReadOnly(bool readonly)
{
  mReadOnly = readonly;

  mText->SetReadOnly(readonly);
  mTopic->setReadOnly(readonly);

  mPublish->Show(!readonly);
  mTop->Layout();
}

void Edit::addKnownTopics(
  const wxObjectDataPtr<Models::KnownTopics> &knownTopicsModel
) {
  mTopic->addKnownTopics(knownTopicsModel);
}

MQTT::Message Edit::getMessage() const
{
  return {
    getTopic(),
    getPayload(),
    mQoS,
    mRetained,
    {}
  };
}

std::string Edit::getPayload() const
{
  const auto wxs = mText->GetValue();
  const auto utf8 = wxs.ToUTF8();
  return {utf8.data(), utf8.length()};
}

bool Edit::getReadOnly() const
{
  return mReadOnly;
}

void Edit::format()
{
  auto text = getPayload();
  auto format = mFormatSelect->GetValue().ToStdString();
  if (mReadOnly)
  {
    mText->SetReadOnly(false);
  }
  auto payloadUtf8 = formatTry(text, mFormats.at(format));
  mText->SetText(
    wxString::FromUTF8(
      payloadUtf8.data(),
      payloadUtf8.length())
  );

  if (mReadOnly)
  {
    mText->SetReadOnly(true);
  }
}

std::string Edit::formatTry(
  const std::string &text,
  Format format
) {

  if (text.empty())
  {
    return "";
  }

  if (format == Format::Auto)
  {
    return formatTry(text, formatGuess(text));
  }

  setStyle(format);

  if (format == Format::Json)
  {
    try
    {
      auto j = json::parse(text);
      return j.dump(2);
    }
    catch (std::exception &e)
    {}
  }

  if (format == Format::Xml)
  {
    tinyxml2::XMLDocument doc;
    auto res = doc.Parse(text.c_str());
    if (res == tinyxml2::XMLError::XML_SUCCESS)
    {
      tinyxml2::XMLPrinter p;
      doc.Print(&p);
      return p.CStr();
    }
  }

  return text;
}

Edit::Format Edit::formatGuess(const std::string &text)
{
  if (text.empty())
  {
    return Format::Text;
  }

  const char c = text[0];

  if (c == '<')
  {
    return Format::Xml;
  }

  if (
    (bool)isdigit(c)
    || c == '"'
    || c == '{'
    || c == '['
    || c == 't'
    || c == 'f'
    || c == 'n'
  ) {
    return Format::Json;
  }

  return Format::Text;
}

void Edit::onFormatSelected(wxCommandEvent &/* event */)
{
  format();
}

void Edit::onTopicCtrlReturn(Events::TopicCtrl &/* event */)
{
  auto *e = new Events::Edit(Events::EDIT_PUBLISH);
  wxQueueEvent(this, e);
}

std::string Edit::getTopic() const
{
  const auto wxs = mTopic->GetValue();
  const auto utf8 = wxs.ToUTF8();
  return {utf8.data(), utf8.length()};
}

MQTT::QoS Edit::getQos() const
{
  return mQoS;
}

bool Edit::getRetained() const
{
  return mRetained;
}

void Edit::clear()
{
  setRetained(false);
  setTopic("");
  setPayload("");
  setQos(MQTT::QoS::AtLeastOnce);
}

void Edit::setRetained(bool retained)
{
  mRetained = retained;
  mRetainedTrue->Show(retained);
  mRetainedFalse->Show(!retained);
  mTop->Layout();
}

void Edit::setTopic(const std::string &topic)
{
  mTopic->SetValue(
    wxString::FromUTF8(
      topic.data(),
      topic.length()
    )
  );
}

void Edit::setQos(MQTT::QoS qos)
{
  if (mQoS == qos) { return; }

  mQos0->Hide();
  mQos1->Hide();
  mQos2->Hide();

  switch (qos)
  {
    case MQTT::QoS::AtLeastOnce: { mQos0->Show(); } break;
    case MQTT::QoS::AtMostOnce:  { mQos1->Show(); } break;
    case MQTT::QoS::ExactlyOnce: { mQos2->Show(); } break;
  }

  mTop->Layout();

  mQoS = qos;
}

void Edit::onQosClicked(wxMouseEvent &e)
{
  e.Skip();
  if (mReadOnly) { return; }

  switch (mQoS)
  {
    case MQTT::QoS::AtLeastOnce: { setQos(MQTT::QoS::AtMostOnce); } break;
    case MQTT::QoS::AtMostOnce:  { setQos(MQTT::QoS::ExactlyOnce); } break;
    case MQTT::QoS::ExactlyOnce: { setQos(MQTT::QoS::AtLeastOnce); } break;
  }
}

void Edit::onRetainedClicked(wxMouseEvent &e)
{
  e.Skip();
  if (mReadOnly) { return; }

  setRetained(!mRetained);
}

std::map<Edit::Theme, Edit::ThemeStyles> Edit::initThemes()
{
  auto color = [](uint8_t red, uint8_t green, uint8_t blue)
  {
    return 0
      | (uint32_t)(red   << (ByteSize * 0))
      | (uint32_t)(green << (ByteSize * 1))
      | (uint32_t)(blue  << (ByteSize * 2))
    ;
  };

  constexpr uint32_t BrightnessBump = color(80, 80, 80); // NOLINT
  constexpr uint32_t Black  = color(30,  30,  30);  // NOLINT
  constexpr uint32_t White  = color(250, 250, 250); // NOLINT
  constexpr uint32_t Red    = color(180, 0,   0);   // NOLINT
  constexpr uint32_t Orange = color(150, 120, 0);   // NOLINT
  constexpr uint32_t Green  = color(0,   150, 0);   // NOLINT
  constexpr uint32_t Pink   = color(200, 0,   150); // NOLINT
  constexpr uint32_t Cyan   = color(0,   120, 150); // NOLINT

  return {
    {Theme::Light, {
      {Style::Comment, {Black,  Black}},
      {Style::Editor,  {Black,  White}},
      {Style::Error,   {Black,  Red}},
      {Style::Normal,  {Black,  White}},
      {Style::Key,     {Green,  White}},
      {Style::Keyword, {Cyan,   White}},
      {Style::Number,  {Orange, White}},
      {Style::String,  {Orange, White}},
      {Style::Uri,     {Cyan,   White}},
      {Style::Special, {Pink,   White}},
    }},
    {Theme::Dark, {
      {Style::Comment, {White, Black}},
      {Style::Editor,  {White, Black}},
      {Style::Error,   {Black, Red}},
      {Style::Normal,  {White, Black}},
      {Style::Key,     {Green  | BrightnessBump, Black}},
      {Style::Keyword, {Cyan   | BrightnessBump, Black}},
      {Style::Number,  {Orange | BrightnessBump, Black}},
      {Style::String,  {Orange | BrightnessBump, Black}},
      {Style::Uri,     {Cyan   | BrightnessBump, Black}},
      {Style::Special, {Pink   | BrightnessBump, Black}},
    }}
  };
}

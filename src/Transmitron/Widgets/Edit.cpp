#include <chrono>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <sstream>
#include <tinyxml2.h>
#include <wx/clipbrd.h>
#include <wx/artprov.h>
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

Edit::Edit(
  wxWindow* parent,
  wxWindowID id,
  size_t optionsHeight,
  bool darkMode
) :
  wxPanel(parent, id),
  mTheme(darkMode ? Theme::Dark : Theme::Light),
  mOptionsHeight(optionsHeight)
{
  constexpr size_t FontSize = 9;
  mFont = wxFont(wxFontInfo(FontSize).FaceName("Consolas"));

  setupScintilla();

  const int timestampBorderPx = 5;
  mTimestamp = new wxStaticText(this, -1, "0000-00-00 00:00:00.000");
  mTimestamp->SetFont(mFont);
  mTimestamp->Hide();

  mTopic = new TopicCtrl(this, -1);
  mTopic->Bind(wxEVT_KEY_UP, &Edit::onTopicKeyDown, this);

  mSaveSnippet = new wxButton(
    this,
    -1,
    "",
    wxDefaultPosition,
    wxSize((int)optionsHeight, (int)optionsHeight)
  );
  mSaveSnippet->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE));
  mSaveSnippet->Bind(wxEVT_BUTTON, [this](wxCommandEvent &/*event*/){
    auto *e = new Events::Edit(Events::EDIT_SAVE_SNIPPET);
    wxQueueEvent(this, e);
  });

  constexpr size_t FormatButtonWidth = 100;

  mFormatSelect = new wxComboBox(
    this,
    -1,
    mFormats.begin()->first,
    wxDefaultPosition,
    wxSize(FormatButtonWidth, (int)optionsHeight)
  );
  for (const auto &format : mFormats)
  {
    mFormatSelect->Insert(format.first, 0);
  }

  mTop = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  mVsizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  mBottom = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  mTop->SetMinSize(0, (int)mOptionsHeight);
  mBottom->SetMinSize(0, (int)mOptionsHeight);

  mRetainedFalse = new wxStaticBitmap(this, -1, *bin2cNotPinned18x18());
  mRetainedFalse->SetToolTip("Not retained");
  mRetainedFalse->Bind(wxEVT_LEFT_UP, &Edit::onRetainedClicked, this);
  mRetainedTrue = new wxStaticBitmap(this, -1, *bin2cPinned18x18());
  mRetainedTrue->SetToolTip("Retained");
  mRetainedTrue->Bind(wxEVT_LEFT_UP, &Edit::onRetainedClicked, this);

  mRetainedTrue->Hide();
  mRetained = false;

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
  mQoS = MQTT::QoS::AtLeastOnce;

  mPublish = new wxBitmapButton(
    this,
    -1,
    *bin2cSend18x18(),
    wxDefaultPosition,
    wxSize((int)mOptionsHeight, (int)mOptionsHeight)
  );
  mPublish->Bind(wxEVT_BUTTON, [this](wxCommandEvent &/*event*/){
    auto *e = new Events::Edit(Events::EDIT_PUBLISH);
    wxQueueEvent(this, e);
  });

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
  mBottom->Add(mFormatSelect, 0, wxEXPAND);
  mVsizer->Add(mTop, 0, wxEXPAND);
  mVsizer->Add(mTimestamp, 0, (int)wxEXPAND | (int)wxLEFT, timestampBorderPx);
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
  constexpr int MarginScriptFoldIndex = 1;
  constexpr int FoldMarginWidth = 20;
  mText->SetMarginWidth(MarginScriptFoldIndex, 0);
  mText->SetMarginType(MarginScriptFoldIndex, wxSTC_MARGIN_SYMBOL);
  mText->SetMarginMask(MarginScriptFoldIndex, (int)wxSTC_MASK_FOLDERS);
  mText->SetMarginWidth(MarginScriptFoldIndex, FoldMarginWidth);
  mText->SetMarginSensitive(MarginScriptFoldIndex, true);
  mText->SetAutomaticFold(wxSTC_AUTOMATICFOLD_CLICK);
  mText->SetFoldFlags(wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);

  // Folding markers.
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDER,        wxSTC_MARK_PLUS);
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN,    wxSTC_MARK_MINUS);
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDEREND,     wxSTC_MARK_EMPTY);
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY);
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_EMPTY);
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB,     wxSTC_MARK_EMPTY);
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL,    wxSTC_MARK_EMPTY);

  mText->StyleSetForeground(wxSTC_STYLE_DEFAULT, mStyles.at(mTheme).at(Style::Normal).first);
  mText->StyleSetBackground(wxSTC_STYLE_DEFAULT, mStyles.at(mTheme).at(Style::Normal).second);
  mText->StyleClearAll();
  mText->StyleSetForeground(wxSTC_STYLE_LINENUMBER, mStyles.at(mTheme).at(Style::Normal).second);
  mText->StyleSetBackground(wxSTC_STYLE_LINENUMBER, mStyles.at(mTheme).at(Style::Normal).first);
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
      mText->StyleSetBackground(wxSTC_JSON_ERROR,        style.at(Style::Error).first);
      mText->StyleSetForeground(wxSTC_JSON_KEYWORD,      style.at(Style::Keyword).first);
      mText->StyleSetForeground(wxSTC_JSON_NUMBER,       style.at(Style::Number).first);
      mText->StyleSetForeground(wxSTC_JSON_PROPERTYNAME, style.at(Style::Key).first);
      mText->StyleSetForeground(wxSTC_JSON_STRING,       style.at(Style::String).first);
      mText->StyleSetForeground(wxSTC_JSON_URI,          style.at(Style::Uri).first);

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

  mTimestamp->SetLabel(ss.str());
}

void Edit::setReadOnly(bool readonly)
{
  mReadOnly = readonly;

  mText->SetReadOnly(readonly);
  mTopic->setReadOnly(readonly);
  if (readonly)
  {
    mTimestamp->Show(readonly);
    mVsizer->Layout();
  }

  mPublish->Show(!readonly);
  mTop->Layout();
}

MQTT::Message Edit::getMessage() const
{
  return {
    getTopic(),
    getPayload(),
    mQoS,
    mRetained,
    0,
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

void Edit::onTopicKeyDown(wxKeyEvent &e)
{
  if (
    e.GetKeyCode() == WXK_RETURN
    && !mTopic->IsEmpty()
  ) {
    auto *e = new Events::Edit(Events::EDIT_PUBLISH);
    wxQueueEvent(this, e);
  }
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


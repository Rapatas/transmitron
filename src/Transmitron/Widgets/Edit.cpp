#include <nlohmann/json.hpp>
#include <tinyxml2.h>
#include "Edit.hpp"
#include "Transmitron/Resources/send/send-18x18.hpp"
#include "Transmitron/Resources/pin/pinned-18x18.hpp"
#include "Transmitron/Resources/pin/not-pinned-18x18.hpp"
#include "Transmitron/Resources/qos/qos-0.hpp"
#include "Transmitron/Resources/qos/qos-1.hpp"
#include "Transmitron/Resources/qos/qos-2.hpp"

using namespace tinyxml2;
using namespace nlohmann;
using namespace Transmitron::Widgets;

Edit::Edit(wxWindow* parent) :
  wxPanel(parent)
{
  mFont = wxFont(wxFontInfo(9).FaceName("Consolas"));
  setupScintilla();

  mFormatSelect = new wxComboBox(this, -1, "TEXT", wxDefaultPosition, wxSize(100, 25));
  mFormatSelect->Insert("TEXT", 0);
  mFormatSelect->Insert("XML", 0);
  mFormatSelect->Insert("JSON", 0);

  mTopic = new wxTextCtrl(this, -1);
  mTopic->SetHint("topic");
  mTopic->SetFont(mFont);

  mTop = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  mVsizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  mBottom = new wxBoxSizer(wxOrientation::wxHORIZONTAL);

  mRetainedFalse = new wxStaticBitmap(this, -1, *bin2c_not_pinned_18x18_png);
  mRetainedFalse->SetToolTip("Not retained");
  mRetainedFalse->Bind(wxEVT_LEFT_UP, &Edit::onRetainedClicked, this);

  mRetainedTrue = new wxStaticBitmap(this, -1, *bin2c_pinned_18x18_png);
  mRetainedTrue->SetToolTip("Retained");
  mRetainedTrue->Bind(wxEVT_LEFT_UP, &Edit::onRetainedClicked, this);

  mRetainedTrue->Hide();
  mRetained = false;

  mQos0 = new wxStaticBitmap(this, -1, *bin2c_qos_0_png);
  mQos0->SetToolTip("QoS: 0");
  mQos0->Bind(wxEVT_LEFT_UP, &Edit::onQosClicked, this);

  mQos1 = new wxStaticBitmap(this, -1, *bin2c_qos_1_png);
  mQos1->SetToolTip("QoS: 1");
  mQos1->Bind(wxEVT_LEFT_UP, &Edit::onQosClicked, this);

  mQos2 = new wxStaticBitmap(this, -1, *bin2c_qos_2_png);
  mQos2->SetToolTip("QoS: 2");
  mQos2->Bind(wxEVT_LEFT_UP, &Edit::onQosClicked, this);

  mQos1->Hide();
  mQos2->Hide();
  mQoS = MQTT::QoS::AtLeastOnce;

  mPublish = new wxBitmapButton(this, -1, *bin2c_send_18x18_png);

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
  mBottom->Add(mFormatSelect, 0, wxEXPAND);
  mVsizer->Add(mTop, 0, wxEXPAND);
  mVsizer->Add(mText, 1, wxEXPAND);
  mVsizer->Add(mBottom, 0, wxALIGN_RIGHT);

  SetSizer(mVsizer);
}

Edit::~Edit() {}

void Edit::setupScintilla()
{
  mText = new wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition);
  mText->SetLexer(wxSTC_LEX_JSON);

  auto error      = (180 << 0) | (0   << 8) | (0   << 16);
  auto backgound  = (0   << 0) | (0   << 8) | (0   << 16);
  auto foreground = (200 << 0) | (200 << 8) | (200 << 16);
  auto number     = (200 << 0) | (150 << 8) | (0   << 16);
  auto green      = (0   << 0) | (200 << 8) | (0   << 16);

  mText->SetWrapMode(wxSTC_WRAP_WORD);
  mText->SetProperty("fold", "1");
  mText->SetProperty("fold.compact", "0");
  mText->SetProperty("fold.comment", "1");
  mText->SetProperty("fold.preprocessor", "1");

  // For JSON only.
  mText->SetKeyWords(0, "true false null");

  static const int MARGIN_SCRIPT_FOLD_INDEX = 1;
  mText->SetMarginType(MARGIN_SCRIPT_FOLD_INDEX, wxSTC_MARGIN_SYMBOL);
  mText->SetMarginMask(MARGIN_SCRIPT_FOLD_INDEX, wxSTC_MASK_FOLDERS);
  mText->SetMarginWidth(MARGIN_SCRIPT_FOLD_INDEX, 20);
  mText->SetMarginSensitive(MARGIN_SCRIPT_FOLD_INDEX, 1);

  mText->MarkerDefine(wxSTC_MARKNUM_FOLDER,        wxSTC_MARK_PLUS);
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN,    wxSTC_MARK_MINUS);
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDEREND,     wxSTC_MARK_EMPTY);
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY);
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_EMPTY);
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB,     wxSTC_MARK_EMPTY);
  mText->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL,    wxSTC_MARK_EMPTY);

  mText->SetScrollWidth(50);
  mText->SetScrollWidthTracking(true);

  mText->SetAutomaticFold(wxSTC_AUTOMATICFOLD_CLICK);

  mText->StyleSetFont(wxSTC_STYLE_DEFAULT, mFont);

  mText->StyleSetBackground(wxSTC_STYLE_DEFAULT, backgound);
  mText->StyleSetForeground(wxSTC_STYLE_DEFAULT, foreground);

  // Apply the defaults we just set.
  mText->StyleClearAll();

  mText->StyleSetForeground(wxSTC_STYLE_LINENUMBER, backgound);
  mText->StyleSetBackground(wxSTC_STYLE_LINENUMBER, foreground);

  mText->StyleSetForeground(wxSTC_JSON_NUMBER,         number);
  mText->StyleSetForeground(wxSTC_JSON_PROPERTYNAME,   green);
  mText->StyleSetForeground(wxSTC_JSON_STRING,         number);
  mText->StyleSetBackground(wxSTC_JSON_ERROR, error);
}

void Edit::setPayload(const std::string &text)
{
  auto format = mFormatSelect->GetValue();
  if (mReadOnly)
  {
    mText->SetReadOnly(false);
  }
  mText->SetText(formatTry(text, format));
  if (mReadOnly)
  {
    mText->SetReadOnly(true);
  }
}

void Edit::setReadOnly(bool readonly)
{
  mReadOnly = readonly;

  mText->SetReadOnly(readonly);

  auto newTopic= new wxTextCtrl(this,
    -1,
    "",
    wxDefaultPosition,
    wxDefaultSize,
    readonly ? wxTE_READONLY : 0
  );
  mTop->Replace(mTopic, newTopic);
  delete mTopic;
  mTopic = newTopic;
  mTopic->SetHint("topic");
  mTopic->SetFont(mFont);

  mPublish->Show(!readonly);
  mTop->Layout();
}

std::string Edit::getPayload() const
{
  return mText->GetText().ToStdString();
}

bool Edit::getReadOnly() const
{
  return mReadOnly;
}

void Edit::format()
{
  auto text = mText->GetText().ToStdString();
  auto format = mFormatSelect->GetValue().ToStdString();
  if (mReadOnly)
  {
    mText->SetReadOnly(false);
  }
  mText->SetText(formatTry(text, format));
  if (mReadOnly)
  {
    mText->SetReadOnly(true);
  }
}

std::string Edit::formatTry(
  const std::string &text,
  const wxString &format
) {
  XMLDocument doc;

  if (format == "JSON" && json::accept(text))
  {
    auto j = json::parse(text);
    return j.dump(4);
  }
  else if (format == "XML" && doc.Parse(text.c_str()) == XMLError::XML_SUCCESS)
  {
    XMLPrinter p;
    doc.Print(&p);
    return p.CStr();
  }
  else
  {
    return text;
  }
}

void Edit::onFormatSelected(wxCommandEvent &e)
{
  format();
}

std::string Edit::getTopic() const
{
  return mTopic->GetValue().ToStdString();
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
  mTopic->SetValue(topic);
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

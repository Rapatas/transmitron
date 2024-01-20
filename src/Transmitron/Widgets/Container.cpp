#include "Transmitron/Widgets/Container.hpp"
#include "Common/Log.hpp"

using namespace Transmitron::Widgets;

Container::Container(wxPanel *parent) :
  wxPanel(parent, wxID_ANY),
  mSizer(new wxBoxSizer(wxHORIZONTAL))
{
  mSizer->AddStretchSpacer(1);
  mSizer->AddStretchSpacer(1);
  SetSizer(mSizer);
  this->Bind(wxEVT_SIZE , &Container::onResized, this, wxID_ANY);
}

void Container::contain(wxPanel *contained)
{
  if (mContained != nullptr)
  {
    mSizer->Remove(1);
  }

  mContained = contained;
  mSizer->Insert(1, mContained, 0, wxEXPAND);

  onResized(GetSize());
}

void Container::onResized(wxSizeEvent& event)
{
  onResized(event.GetSize());
  event.Skip(true);
}

void Container::onResized(wxSize size)
{
  const auto width = size.x;
  const auto newHeight = size.y;
  const auto newWidth = [&]()
  {
    // NOLINTBEGIN(readability-magic-numbers)
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
    if (width >= 1400) { return 1320; }
    if (width >= 1200) { return 1140; }
    if (width >= 992) { return 960; }
    if (width >= 768) { return 720; }
    return 540;
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
    // NOLINTEND(readability-magic-numbers)
  }();

  mContained->SetMinSize(wxSize{
    newWidth,
    newHeight
  });
}

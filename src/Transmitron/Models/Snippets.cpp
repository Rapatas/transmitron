#include "Snippets.hpp"

#include <wx/log.h>

#define wxLOG_COMPONENT "models/snippets"

using namespace Transmitron::Models;

Snippets::Snippets()
{
  for (size_t i = 0; i != 10; ++i)
  {
    mSnippets.push_back(new std::string("lorem ipsum"));
  }
}

Snippets::~Snippets()
{
  for (const auto &s : mSnippets)
  {
    delete s;
  }
}

unsigned Snippets::GetColumnCount() const
{
  return mSnippets.size();
}

wxString Snippets::GetColumnType(unsigned int col) const
{
  switch ((Column)col)
  {
    case Column::Name: { return wxDataViewTextRenderer::GetDefaultType(); } break;
    default: { return "string"; }
  }
}

void Snippets::GetValue(
  wxVariant &variant,
  const wxDataViewItem &item,
  unsigned int col
) const {
  auto s = static_cast<std::string*>(item.GetID());
  wxLogMessage("Giving %s", *s);
  variant = *s;
}

bool Snippets::SetValue(
  const wxVariant &variant,
  const wxDataViewItem &item,
  unsigned int col
) {
  return false;
}

bool Snippets::IsEnabled(
  const wxDataViewItem &item,
  unsigned int col
) const {
  return true;
}

wxDataViewItem Snippets::GetParent(
  const wxDataViewItem &item
) const {
  return wxDataViewItem(nullptr);
}

bool Snippets::IsContainer(
  const wxDataViewItem &item
) const {
  return false;
}

unsigned int Snippets::GetChildren(
  const wxDataViewItem &parent,
  wxDataViewItemArray &array
) const {
  for (const auto &s : mSnippets)
  {
    array.Add(wxDataViewItem(static_cast<void*>(s)));
  }
  return mSnippets.size();
}



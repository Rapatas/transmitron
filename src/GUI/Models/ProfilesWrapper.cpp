#include "ProfilesWrapper.hpp"

using namespace Rapatas::Transmitron::GUI::Models;
using Item = wxDataViewItem;

ProfilesWrapper::ProfilesWrapper(const wxObjectDataPtr<Profiles> &profiles) :
  mProfiles(profiles) //
{}

unsigned ProfilesWrapper::GetColumnCount() const {
  return mProfiles->GetColumnCount();
}

wxString ProfilesWrapper::GetColumnType(unsigned int col) const {
  return mProfiles->GetColumnType(col);
}

void ProfilesWrapper::GetValue(
  wxVariant &variant,
  const Item &item,
  unsigned int col //
) const {
  mProfiles->GetValue(variant, item, col);
}

bool ProfilesWrapper::SetValue(
  const wxVariant &value,
  const Item &item,
  unsigned int col //
) {
  (void)value;
  (void)item;
  (void)col;
  (void)this;
  return false;
}

bool ProfilesWrapper::IsEnabled(
  const Item &item,
  unsigned int col //
) const {
  (void)item;
  (void)col;
  (void)this;
  return true;
}

Item ProfilesWrapper::GetParent(const Item &item) const {
  return mProfiles->GetParent(item);
}

bool ProfilesWrapper::IsContainer(const Item &item) const {
  return mProfiles->IsContainer(item);
}

unsigned int ProfilesWrapper::GetChildren(
  const Item &parent,
  wxDataViewItemArray &array //
) const {
  return mProfiles->GetChildren(parent, array);
}

#include "Common/Log.hpp"
#include "KnownTopics.hpp"
#include <cstring>
#include <fstream>

namespace fs = Common::fs;
using namespace Transmitron::Models;

KnownTopics::KnownTopics()
{
  mLogger = Common::Log::create("Models::KnownTopics");
  remap();
}

KnownTopics::~KnownTopics()
{
  save();
}

bool KnownTopics::load(Common::fs::path filepath)
{
  mLogger->info("Loading {}", filepath.string());
  mFilepath = std::move(filepath);
  const bool exists = fs::exists(mFilepath);
  if (!exists)
  {
    mLogger->warn("Could not load file '{}': Not found", mFilepath.string());
    return false;
  }

  std::ifstream file(mFilepath);
  if (!file.is_open())
  {
    mLogger->warn(
      "Could not load file '{}': {}",
      mFilepath.string(),
      std::strerror(errno)
    );
    return false;
  }

  std::string line;
  while (std::getline(file, line))
  {
    if (line.empty()) { continue; }
    mTopics.insert(line);
  }

  remap();

  return true;
}

void KnownTopics::clear()
{
  mTopics.clear();
  remap();
}

void KnownTopics::setFilter(std::string filter)
{
  mFilter = std::move(filter);
  remap();
}

void KnownTopics::append(std::string topic)
{
  if (topic == "#") { return; }
  if (topic.empty()) { return; }

  const auto it = std::find(
    std::begin(mTopics),
    std::end(mTopics),
    topic
  );
  if (it != std::end(mTopics))
  {
    return;
  }

  mTopics.insert(std::move(topic));
  remap();
}

void KnownTopics::append(std::set<std::string> topics)
{
  topics.erase("");
  mTopics.merge(topics);
}

const std::string &KnownTopics::getTopic(const wxDataViewItem &item) const
{
  return *(mRemap.at(GetRow(item)));
}

const std::string &KnownTopics::getFilter() const
{
  return mFilter;
}

void KnownTopics::remap()
{
  const size_t before = mRemap.size();
  mRemap.clear();
  mRemap.reserve(mTopics.size());

  for (auto it = std::begin(mTopics); it != std::end(mTopics); ++it)
  {
    const auto &topic = *it;
    const auto pos = topic.find(mFilter);
    if (mFilter.empty() || pos != std::string::npos)
    {
      mRemap.push_back(it);
    }
  }

  mRemap.shrink_to_fit();
  const size_t after = mRemap.size();

  const auto common = std::min(before, after);
  for (size_t i = 0; i != common; ++i)
  {
    RowChanged((unsigned)i);
  }

  if (after > before)
  {
    size_t diff = after - common;
    for (size_t i = 0; i < diff; ++i)
    {
      RowAppended();
    }
  }
  else if (after < before)
  {
    wxArrayInt rows;
    size_t diff = before - common;
    for (size_t i = 0; i < diff; ++i)
    {
      rows.Add((int)(after + i));
    }
    RowsDeleted(rows);
  }
}

void KnownTopics::save()
{
  if (mFilepath.empty()) { return; }

  mLogger->info("Saving to {}", mFilepath.string());
  std::ofstream file(mFilepath);
  if (!file.is_open())
  {
    mLogger->warn(
      "Could not save file '{}': {}",
      mFilepath.string(),
      std::strerror(errno)
    );
    return;
  }

  for (const auto &topic : mTopics)
  {
    file << topic << "\n";
  }
}

// wxDataViewVirtualListModel interface {

unsigned KnownTopics::GetColumnCount() const
{
  return (unsigned)Column::Max;
}

wxString KnownTopics::GetColumnType(unsigned int col) const
{
  if (static_cast<Column>(col) != Column::Topic)
  {
    return "string";
  }

  return wxDataViewTextRenderer::GetDefaultType();
}

unsigned KnownTopics::GetCount() const
{
  return (unsigned)mRemap.size();
}

void KnownTopics::GetValueByRow(
  wxVariant &variant,
  unsigned int row,
  unsigned int col
) const {

  if (static_cast<Column>(col) != Column::Topic)
  {
    return;
  }

  const auto &topic = *(mRemap.at(row));
  const auto wxs = wxString::FromUTF8(topic.data(), topic.length());
  variant = wxs;
}

bool KnownTopics::GetAttrByRow(
  unsigned int /* row */,
  unsigned int /* col */,
  wxDataViewItemAttr &/* attr */
) const {
  return false;
}

bool KnownTopics::SetValueByRow(
  const wxVariant &/* variant */,
  unsigned int /* row */,
  unsigned int /* col */
) {
  return false;
}

// wxDataViewVirtualListModel interface }

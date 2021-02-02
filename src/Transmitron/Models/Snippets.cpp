#include "Snippets.hpp"

#include <wx/log.h>
#include <fmt/format.h>

#define wxLOG_COMPONENT "models/snippets"

using namespace Transmitron::Models;

Snippets::Snippets()
{
  Node root {
    0,
    "_",
    Node::Type::Folder,
    {},
    nullptr
  };

  mNodes.push_back(std::move(root));

  for (size_t p = 0; p != 10; ++p)
  {
    Node parent {
      0,
      fmt::format("Parent - {}", p),
      Node::Type::Folder,
      {},
      nullptr
    };
    mNodes.push_back(std::move(parent));
    Node::Index_t currentParent = mNodes.size() - 1;
    mNodes.at(0).children.push_back(currentParent);

    for (size_t i = 0; i != 10; ++i)
    {
      auto message = std::make_unique<MQTT::Message>();
      message->topic = "sample/topic/here";
      message->payload = R"({"lorem":{"key":"value"}})";
      message->qos = MQTT::QoS::AtLeastOnce;
      message->retained = false;

      Node snip {
        currentParent,
        fmt::format("Snippet - {}", i),
        Node::Type::Snippet,
        {},
        std::move(message)
      };

      mNodes.push_back(std::move(snip));
      mNodes.at(currentParent).children.push_back(mNodes.size() - 1);
    }
  }
}

unsigned Snippets::GetColumnCount() const
{
  return static_cast<unsigned>(Column::Max);
}

wxString Snippets::GetColumnType(unsigned int col) const
{
  switch ((Column)col)
  {
    case Column::Name: {
      return wxDataViewTextRenderer::GetDefaultType();
    } break;
    default: { return "string"; }
  }
}

void Snippets::GetValue(
  wxVariant &variant,
  const wxDataViewItem &item,
  unsigned int col
) const {
  auto index = toIndex(item);
  variant = mNodes.at(index).name;
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

  if (!item.IsOk())
  {
    return wxDataViewItem(nullptr);
  }

  auto index = toIndex(item);

  if (index == 0)
  {
    return wxDataViewItem(nullptr);
  }

  auto parent = mNodes.at(index).parent;

  return toItem(parent);
}

bool Snippets::IsContainer(
  const wxDataViewItem &item
) const {

  if (!item.IsOk())
  {
    return true;
  }

  const auto &node = mNodes.at(toIndex(item));
  return node.type == Node::Type::Folder;
}

unsigned int Snippets::GetChildren(
  const wxDataViewItem &parent,
  wxDataViewItemArray &array
) const {

  Node::Index_t index = 0;

  if (parent.IsOk())
  {
    index = toIndex(parent);
  }

  const auto &node = mNodes.at(index);

  for (auto i : node.children)
  {
    array.Add(toItem(i));
  }
  return node.children.size();
}

Snippets::Node::Index_t Snippets::toIndex(const wxDataViewItem &item) const
{
  return reinterpret_cast<Node::Index_t>(item.GetID());
}

wxDataViewItem Snippets::toItem(Node::Index_t index) const
{
  return wxDataViewItem(reinterpret_cast<void*>(index));
}


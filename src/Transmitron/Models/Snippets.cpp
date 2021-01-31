#include "Snippets.hpp"

#include <wx/log.h>

#define wxLOG_COMPONENT "models/snippets"

using namespace Transmitron::Models;

Snippets::Snippets()
{
  mRoot = new Types::SnippetNode(nullptr, "Root this is");
  for (size_t p = 0; p != 10; ++p)
  {
    auto f = new Types::SnippetNode(mRoot, "Parent - " + std::to_string(p));

    for (size_t i = 0; i != 10; ++i)
    {
      auto message = std::make_unique<MQTT::Message>();
      message->topic = "sample/topic/here";
      message->payload = R"({"lorem":{"key":"value"}})";
      message->qos = MQTT::QoS::AtLeastOnce;
      message->retained = false;

      auto s = new Types::SnippetNode(
        f,
        "some name - " + std::to_string(i),
        std::move(message)
      );
    }
  }

}

Snippets::~Snippets()
{
  if (mRoot != nullptr)
  {
    delete mRoot;
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
  auto s = static_cast<Types::SnippetNode*>(item.GetID());
  variant = s->getName();
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

  auto node = static_cast<Types::SnippetNode*>(item.GetID());

  if (node == mRoot)
  {
    return wxDataViewItem(nullptr);
  }

  auto parent = node->getParent();
  if (parent == mRoot)
  {
    parent = nullptr;
  }
  return wxDataViewItem(static_cast<void*>(parent));
}

bool Snippets::IsContainer(
  const wxDataViewItem &item
) const {

  if (!item.IsOk())
  {
    return true;
  }

  auto node = static_cast<Types::SnippetNode*>(item.GetID());
  return node->getType() == Types::SnippetNode::Type::Folder;
}

unsigned int Snippets::GetChildren(
  const wxDataViewItem &parent,
  wxDataViewItemArray &array
) const {

  Types::SnippetNode *current = mRoot;

  if (parent.IsOk())
  {
    current = static_cast<Types::SnippetNode*>(parent.GetID());
  }

  for (const auto &c : current->getChildren())
  {
    array.Add(wxDataViewItem(static_cast<void*>(c)));
  }
  return current->getChildren().size();
}


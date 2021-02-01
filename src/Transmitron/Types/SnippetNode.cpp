#include "SnippetNode.hpp"

using namespace Transmitron::Types;

SnippetNode::SnippetNode(
  SnippetNode *parent,
  std::string name,
  std::unique_ptr<MQTT::Message> message
) :
  mType(Type::Snippet),
  mParent(parent),
  mName(name),
  mMessage(std::move(message))
{
  if (mParent != nullptr)
  {
    mParent->addChild(this);
  }
}

SnippetNode::SnippetNode(SnippetNode *parent, std::string name) :
  mType(Type::Folder),
  mParent(parent),
  mName(name)
{
  if (mParent != nullptr)
  {
    mParent->addChild(this);
  }
}

SnippetNode::~SnippetNode()
{
  for (auto &c : mChildren)
  {
    delete c;
  }
}

const MQTT::Message &SnippetNode::getMessage() const
{
  if (mType != Type::Snippet)
  {
    throw std::runtime_error("Can not get snippet from folder");
  }

  return mMessage.operator*();
}

std::string SnippetNode::getName() const
{
  return mName;
}

SnippetNode::Type SnippetNode::getType() const
{
  return mType;
}

bool SnippetNode::addChild(SnippetNode *child)
{
  if (mType != Type::Folder) { return false; }
  mChildren.push_back(child);
  return true;
}


SnippetNode *SnippetNode::getParent() const
{
  return mParent;
}

std::vector<SnippetNode*> SnippetNode::getChildren() const
{
  return mChildren;
}

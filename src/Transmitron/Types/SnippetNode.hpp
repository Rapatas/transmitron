#ifndef TRANSMITRON_TYPES_SNIPPETNODE_HPP
#define TRANSMITRON_TYPES_SNIPPETNODE_HPP

#include <memory>
#include <vector>
#include <string>
#include "MQTT/Message.hpp"
#include "MQTT/QualityOfService.hpp"

namespace Transmitron::Types
{

class SnippetNode
{
public:

  enum class Type
  {
    Folder,
    Snippet,
  };

  explicit SnippetNode(
    SnippetNode *parent,
    std::string name,
    std::unique_ptr<MQTT::Message> message
  );
  explicit SnippetNode(SnippetNode *parent, std::string name);
  virtual ~SnippetNode();

  const MQTT::Message &getMessage() const;
  std::string getName() const;
  Type getType() const;

  SnippetNode *getParent() const;
  std::vector<SnippetNode*> getChildren() const;

  bool addChild(SnippetNode *child);

private:

  Type mType;
  std::string mName;
  SnippetNode *mParent;

  // When type = snippet.
  std::unique_ptr<MQTT::Message> mMessage;

  // When type = folder.
  std::vector<SnippetNode*> mChildren;
};

}

#endif // TRANSMITRON_TYPES_SNIPPETNODE_HPP

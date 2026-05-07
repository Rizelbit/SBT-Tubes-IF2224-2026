#include "ParseNode.hpp"

ParseNode::ParseNode(const std::string& name) : name(name) {}

ParseNode::~ParseNode() {
    for (auto child : children) {
        delete child;
    }
}

void ParseNode::addChild(ParseNode* child) {
    if (child) {
        children.push_back(child);
    }
}

void ParseNode::printTree(std::ostream& out, const std::string& prefix, bool isLast, bool isRoot) const {
    if (!isRoot) {
        out << prefix << (isLast ? "└── " : "├── ");
    } else {
        out << prefix;
    }

    out << name << "\n";

    std::string childPrefix = prefix;
    if (!isRoot) {
        childPrefix += isLast ? "    " : "│   ";
    }

    for (size_t i = 0;i < children.size();++i) {
        children[i]->printTree(out, childPrefix, (i == children.size() - 1), false);
    }
}
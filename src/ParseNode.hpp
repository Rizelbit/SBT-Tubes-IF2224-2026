#pragma once

#include <string>
#include <vector>
#include <iostream>

class ParseNode {
public:
    std::string name;
    std::vector<ParseNode*> children;

    ParseNode(const std::string& name);
    ~ParseNode();

    void addChild(ParseNode* child);
    void printTree(std::ostream& out, const std::string& prefix = "", bool isLast = true, bool isRoot = true) const;
};
#include "ASTBuilder.hpp"

#include <algorithm>

std::string ASTBuilder::extractTokenKind(const std::string& s) const {
    size_t pos = s.find('(');
    return (pos == std::string::npos) ? s : s.substr(0, pos);
}

std::string ASTBuilder::extractTokenValue(const std::string& s) const {
    size_t pos1 = s.find('(');
    size_t pos2 = s.rfind(')');
    if (pos1 != std::string::npos && pos2 != std::string::npos && pos2 > pos1) {
        return s.substr(pos1 + 1, pos2 - pos1 - 1);
    }
    return "";
}

bool ASTBuilder::isToken(ParseNode* node, const std::string& tokenKind) const {
    return node && extractTokenKind(node->name) == tokenKind;
}

bool ASTBuilder::isOperatorWrapper(ParseNode* node) const {
    return node && (node->name == "<relational-operator>" || node->name == "<additive-operator>" || node->name == "<multiplicative-operator>");
}

std::string ASTBuilder::operatorText(ParseNode* node) const {
    if (!node) return "";
    ParseNode* op = isOperatorWrapper(node) && !node->children.empty() ? node->children[0] : node;
    std::string value = extractTokenValue(op->name);
    return value.empty() ? extractTokenKind(op->name) : value;
}

ASTNode* ASTBuilder::clone(ASTNode* node) {
    if (!node) return nullptr;

    ASTNode* copy = new ASTNode(node->kind, node->value);
    copy->inferredType = node->inferredType;
    copy->tabIndex = node->tabIndex;
    copy->blockIndex = node->blockIndex;
    copy->lexicalLevel = node->lexicalLevel;
    for (ASTNode* child : node->children) {
        copy->addChild(clone(child));
    }
    return copy;
}

std::vector<std::string> ASTBuilder::collectIdentifiers(ParseNode* node) {
    std::vector<std::string> identifiers;
    if (!node) return identifiers;

    if (isToken(node, "IDENT")) {
        identifiers.push_back(extractTokenValue(node->name));
    }

    for (ParseNode* child : node->children) {
        std::vector<std::string> childIdentifiers = collectIdentifiers(child);
        identifiers.insert(identifiers.end(), childIdentifiers.begin(), childIdentifiers.end());
    }
    return identifiers;
}

ASTNode* ASTBuilder::build(ParseNode* parseRoot) {
    if (!parseRoot) return nullptr;
    if (parseRoot->name == "<program>") return buildProgram(parseRoot);
    return nullptr;
}

ASTNode* ASTBuilder::buildProgram(ParseNode* node) {
    ASTNode* ast = new ASTNode(ASTKind::Program);
    if (!node->children.empty()) {
        ParseNode* header = node->children[0];
        if (header->children.size() > 1) {
            ast->value = extractTokenValue(header->children[1]->name);
        }
    }

    for (ParseNode* child : node->children) {
        if (child->name == "<declaration-part>") {
            ast->addChild(buildDeclarationPart(child));
        } else if (child->name == "<compound-statement>") {
            ast->addChild(buildBlock(child));
        }
    }
    return ast;
}

ASTNode* ASTBuilder::buildDeclarationPart(ParseNode* node) {
    ASTNode* declarations = new ASTNode(ASTKind::DeclarationPart);
    for (ParseNode* child : node->children) {
        ASTNode* declaration = nullptr;
        if (child->name == "<const-declaration>") {
            declaration = buildConstDeclaration(child);
        } else if (child->name == "<type-declaration>") {
            declaration = buildTypeDeclaration(child);
        } else if (child->name == "<var-declaration>") {
            declaration = buildVarDeclaration(child);
        } else if (child->name == "<subprogram-declaration>") {
            declaration = buildSubprogramDeclaration(child);
        }

        if (!declaration) continue;
        if (declaration->kind == ASTKind::DeclarationPart) {
            for (ASTNode* grandChild : declaration->children) {
                declarations->addChild(grandChild);
            }
            declaration->children.clear();
            delete declaration;
        } else {
            declarations->addChild(declaration);
        }
    }
    return declarations;
}

ASTNode* ASTBuilder::buildConstDeclaration(ParseNode* node) {
    ASTNode* declarations = new ASTNode(ASTKind::DeclarationPart);
    for (size_t i = 0; i + 2 < node->children.size(); ++i) {
        if (!isToken(node->children[i], "IDENT")) continue;

        ASTNode* constDecl = new ASTNode(ASTKind::ConstDecl, extractTokenValue(node->children[i]->name));
        constDecl->addChild(buildConstant(node->children[i + 2]));
        declarations->addChild(constDecl);
    }
    return declarations;
}

ASTNode* ASTBuilder::buildTypeDeclaration(ParseNode* node) {
    ASTNode* declarations = new ASTNode(ASTKind::DeclarationPart);
    for (size_t i = 0; i + 2 < node->children.size(); ++i) {
        if (!isToken(node->children[i], "IDENT")) continue;

        ASTNode* typeDecl = new ASTNode(ASTKind::TypeDecl, extractTokenValue(node->children[i]->name));
        typeDecl->addChild(buildType(node->children[i + 2]));
        declarations->addChild(typeDecl);
    }
    return declarations;
}

ASTNode* ASTBuilder::buildVarDeclaration(ParseNode* node) {
    ASTNode* declarations = new ASTNode(ASTKind::DeclarationPart);
    for (size_t i = 0; i + 2 < node->children.size(); ++i) {
        if (node->children[i]->name != "<identifier-list>") continue;

        std::vector<std::string> identifiers = collectIdentifiers(node->children[i]);
        ASTNode* typeNode = buildType(node->children[i + 2]);
        for (const std::string& identifier : identifiers) {
            ASTNode* varDecl = new ASTNode(ASTKind::VarDecl, identifier);
            varDecl->addChild(clone(typeNode));
            declarations->addChild(varDecl);
        }
        delete typeNode;
    }
    return declarations;
}

ASTNode* ASTBuilder::buildSubprogramDeclaration(ParseNode* node) {
    if (node->children.empty()) return new ASTNode(ASTKind::Empty);

    ParseNode* declaration = node->children[0];
    bool isProcedure = declaration->name == "<procedure-declaration>";
    bool isFunction = declaration->name == "<function-declaration>";
    if (!isProcedure && !isFunction) return new ASTNode(ASTKind::Empty);

    ASTNode* subprogram = new ASTNode(isProcedure ? ASTKind::ProcDecl : ASTKind::FuncDecl);
    for (ParseNode* child : declaration->children) {
        if (isToken(child, "IDENT") && subprogram->value.empty()) {
            subprogram->value = extractTokenValue(child->name);
        } else if (child->name == "<formal-parameter-list>") {
            subprogram->addChild(buildParameterList(child));
        } else if (child->name == "<block>") {
            subprogram->addChild(buildBlock(child));
        }
    }

    if (isFunction) {
        for (size_t i = 0; i + 1 < declaration->children.size(); ++i) {
            if (isToken(declaration->children[i], "COLON") && isToken(declaration->children[i + 1], "IDENT")) {
                subprogram->addChild(new ASTNode(ASTKind::Type, extractTokenValue(declaration->children[i + 1]->name)));
                break;
            }
        }
    }

    return subprogram;
}

ASTNode* ASTBuilder::buildParameterList(ParseNode* node) {
    ASTNode* parameters = new ASTNode(ASTKind::DeclarationPart, "parameters");
    for (ParseNode* group : node->children) {
        if (group->name != "<parameter-group>") continue;

        std::vector<std::string> identifiers;
        ASTNode* typeNode = nullptr;
        for (ParseNode* child : group->children) {
            if (child->name == "<identifier-list>") {
                identifiers = collectIdentifiers(child);
            } else if (child->name == "<array-type>") {
                typeNode = buildType(child);
            } else if (isToken(child, "IDENT")) {
                typeNode = new ASTNode(ASTKind::Type, extractTokenValue(child->name));
            }
        }

        for (const std::string& identifier : identifiers) {
            ASTNode* param = new ASTNode(ASTKind::Param, identifier);
            param->addChild(clone(typeNode));
            parameters->addChild(param);
        }
        delete typeNode;
    }
    return parameters;
}

ASTNode* ASTBuilder::buildType(ParseNode* node) {
    if (!node) return nullptr;
    if (node->name == "<type>" && !node->children.empty()) return buildType(node->children[0]);

    if (isToken(node, "IDENT")) return new ASTNode(ASTKind::Type, extractTokenValue(node->name));

    if (node->name == "<range>") {
        ASTNode* range = new ASTNode(ASTKind::RangeType);
        if (node->children.size() >= 4) {
            range->addChild(buildConstant(node->children[0]));
            range->addChild(buildConstant(node->children[3]));
        }
        return range;
    }

    if (node->name == "<enumerated>") {
        ASTNode* enumerated = new ASTNode(ASTKind::EnumType);
        for (ParseNode* child : node->children) {
            if (isToken(child, "IDENT")) {
                enumerated->addChild(new ASTNode(ASTKind::Literal, extractTokenValue(child->name)));
            }
        }
        return enumerated;
    }

    if (node->name == "<array-type>") {
        ASTNode* arrayType = new ASTNode(ASTKind::ArrayType);
        for (ParseNode* child : node->children) {
            if (child->name == "<range>" || child->name == "<type>" || isToken(child, "IDENT")) {
                arrayType->addChild(buildType(child));
            }
        }
        return arrayType;
    }

    if (node->name == "<record-type>") {
        ASTNode* recordType = new ASTNode(ASTKind::RecordType);
        for (ParseNode* child : node->children) {
            if (child->name == "<field-list>") {
                for (ParseNode* fieldChild : child->children) {
                    if (fieldChild->name == "<field-part>") {
                        ASTNode* fields = buildRecordFieldPart(fieldChild);
                        for (ASTNode* field : fields->children) {
                            recordType->addChild(field);
                        }
                        fields->children.clear();
                        delete fields;
                    }
                }
            }
        }
        return recordType;
    }

    return new ASTNode(ASTKind::Type, node->name);
}

ASTNode* ASTBuilder::buildRecordFieldPart(ParseNode* node) {
    ASTNode* fields = new ASTNode(ASTKind::DeclarationPart, "fields");
    std::vector<std::string> identifiers;
    ASTNode* typeNode = nullptr;

    for (ParseNode* child : node->children) {
        if (child->name == "<identifier-list>") {
            identifiers = collectIdentifiers(child);
        } else if (child->name == "<type>") {
            typeNode = buildType(child);
        }
    }

    for (const std::string& identifier : identifiers) {
        ASTNode* field = new ASTNode(ASTKind::FieldDecl, identifier);
        field->addChild(clone(typeNode));
        fields->addChild(field);
    }
    delete typeNode;
    return fields;
}

ASTNode* ASTBuilder::buildBlock(ParseNode* node) {
    ASTNode* block = new ASTNode(ASTKind::Block);
    for (ParseNode* child : node->children) {
        if (child->name == "<declaration-part>") {
            ASTNode* declarations = buildDeclarationPart(child);
            if (!declarations->children.empty()) block->addChild(declarations);
            else delete declarations;
        } else if (child->name == "<statement-list>") {
            for (ParseNode* stmtChild : child->children) {
                if (stmtChild->name == "<statement>") {
                    block->addChild(buildStatement(stmtChild));
                }
            }
        }
    }
    return block;
}

ASTNode* ASTBuilder::buildStatement(ParseNode* node) {
    if (!node || node->children.empty()) return new ASTNode(ASTKind::Empty);

    ParseNode* firstChild = node->children[0];
    if (firstChild->name == "<assignment-statement>") {
        ASTNode* assignNode = new ASTNode(ASTKind::Assign);
        if (firstChild->children.size() >= 3) {
            assignNode->addChild(buildVariable(firstChild->children[0]));
            assignNode->addChild(buildExpression(firstChild->children[2]));
        }
        return assignNode;
    }

    if (firstChild->name == "<procedure/function-call>") return buildProcedureCall(firstChild);

    if (firstChild->name == "<if-statement>") {
        ASTNode* ifNode = new ASTNode(ASTKind::If);
        for (ParseNode* child : firstChild->children) {
            if (child->name == "<expression>") ifNode->addChild(buildExpression(child));
            else if (child->name == "<statement>") ifNode->addChild(buildStatement(child));
        }
        return ifNode;
    }

    if (firstChild->name == "<while-statement>") {
        ASTNode* whileNode = new ASTNode(ASTKind::While);
        for (ParseNode* child : firstChild->children) {
            if (child->name == "<expression>") whileNode->addChild(buildExpression(child));
            else if (child->name == "<compound-statement>") whileNode->addChild(buildBlock(child));
        }
        return whileNode;
    }

    if (firstChild->name == "<repeat-statement>") {
        ASTNode* repeatNode = new ASTNode(ASTKind::Repeat);
        for (ParseNode* child : firstChild->children) {
            if (child->name == "<statement-list>") {
                ASTNode* body = new ASTNode(ASTKind::Block);
                for (ParseNode* stmtChild : child->children) {
                    if (stmtChild->name == "<statement>") body->addChild(buildStatement(stmtChild));
                }
                repeatNode->addChild(body);
            } else if (child->name == "<expression>") {
                repeatNode->addChild(buildExpression(child));
            }
        }
        return repeatNode;
    }

    if (firstChild->name == "<for-statement>") {
        ASTNode* forNode = new ASTNode(ASTKind::For);
        for (ParseNode* child : firstChild->children) {
            if (isToken(child, "IDENT") && forNode->value.empty()) forNode->value = extractTokenValue(child->name);
            else if (isToken(child, "TOSY") || isToken(child, "DOWNTOSY")) forNode->value += " " + extractTokenKind(child->name);
            else if (child->name == "<expression>") forNode->addChild(buildExpression(child));
            else if (child->name == "<compound-statement>") forNode->addChild(buildBlock(child));
        }
        return forNode;
    }

    if (firstChild->name == "<case-statement>") {
        ASTNode* caseNode = new ASTNode(ASTKind::Case);
        for (ParseNode* child : firstChild->children) {
            if (child->name == "<expression>") caseNode->addChild(buildExpression(child));
            else if (child->name == "<case-block>") caseNode->addChild(buildStatement(child));
        }
        return caseNode;
    }

    if (firstChild->name == "<case-block>") {
        ASTNode* caseNode = new ASTNode(ASTKind::Case);
        for (ParseNode* child : firstChild->children) {
            if (child->name == "<constant>") caseNode->addChild(buildConstant(child));
            else if (child->name == "<statement>") caseNode->addChild(buildStatement(child));
            else if (child->name == "<case-block>") caseNode->addChild(buildStatement(child));
        }
        return caseNode;
    }

    return new ASTNode(ASTKind::Empty);
}

ASTNode* ASTBuilder::buildExpression(ParseNode* node) {
    if (!node) return nullptr;

    if (node->name == "<variable>") return buildVariable(node);
    if (node->name == "<procedure/function-call>") {
        ASTNode* funcCall = buildProcedureCall(node);
        funcCall->kind = ASTKind::FunctionCall;
        return funcCall;
    }
    if (node->name == "<constant>") return buildConstant(node);

    std::string kind = extractTokenKind(node->name);
    if (kind == "IDENT") return new ASTNode(ASTKind::Var, extractTokenValue(node->name));
    if (kind == "INTCON" || kind == "REALCON" || kind == "STRING" || kind == "CHARCON") {
        return new ASTNode(ASTKind::Literal, extractTokenValue(node->name));
    }

    if (!node->children.empty() && (isToken(node->children[0], "PLUS") || isToken(node->children[0], "MINUS") || isToken(node->children[0], "NOTSY"))) {
        ASTNode* unary = new ASTNode(ASTKind::UnaryOp, operatorText(node->children[0]));
        for (size_t i = 1; i < node->children.size(); ++i) {
            ASTNode* child = buildExpression(node->children[i]);
            if (child && child->kind != ASTKind::Empty) {
                unary->addChild(child);
                return unary;
            }
            delete child;
        }
        return unary;
    }

    ASTNode* current = nullptr;
    for (ParseNode* child : node->children) {
        if (isOperatorWrapper(child)) {
            continue;
        }

        ASTNode* operand = buildExpression(child);
        if (!operand || operand->kind == ASTKind::Empty) {
            delete operand;
            continue;
        }

        if (!current) {
            current = operand;
            continue;
        }

        auto opIt = std::find_if(node->children.begin(), node->children.end(), [child](ParseNode* candidate) {
            return candidate == child;
        });
        std::string op = "";
        if (opIt != node->children.begin()) {
            for (auto it = opIt; it != node->children.begin();) {
                --it;
                if ((*it)->name == "<relational-operator>" ||
                    (*it)->name == "<additive-operator>" ||
                    (*it)->name == "<multiplicative-operator>") {
                    op = operatorText(*it);
                    break;
                }
            }
        }

        ASTNode* binOp = new ASTNode(ASTKind::BinOp, op.empty() ? "?" : op);
        binOp->addChild(current);
        binOp->addChild(operand);
        current = binOp;
    }

    if (current) return current;
    return new ASTNode(ASTKind::Empty);
}

ASTNode* ASTBuilder::buildVariable(ParseNode* node) {
    if (!node) return nullptr;

    ASTNode* current = nullptr;
    for (ParseNode* child : node->children) {
        if (isToken(child, "IDENT") && !current) {
            current = new ASTNode(ASTKind::Var, extractTokenValue(child->name));
        } else if (child->name == "<component-variable>" && current) {
            if (!child->children.empty() && isToken(child->children[0], "LBRACK")) {
                ASTNode* access = new ASTNode(ASTKind::ArrayAccess);
                access->addChild(current);
                for (ParseNode* componentChild : child->children) {
                    if (componentChild->name == "<index-list>") {
                        for (ParseNode* indexChild : componentChild->children) {
                            if (isToken(indexChild, "INTCON") || isToken(indexChild, "CHARCON") || isToken(indexChild, "IDENT")) {
                                access->addChild(buildExpression(indexChild));
                            }
                        }
                    }
                }
                current = access;
            } else if (child->children.size() >= 2 && isToken(child->children[0], "PERIOD")) {
                ASTNode* field = new ASTNode(ASTKind::FieldAccess, extractTokenValue(child->children[1]->name));
                field->addChild(current);
                current = field;
            }
        }
    }

    return current ? current : new ASTNode(ASTKind::Empty);
}

ASTNode* ASTBuilder::buildProcedureCall(ParseNode* node) {
    ASTNode* callNode = new ASTNode(ASTKind::ProcedureCall);
    if (!node->children.empty() && isToken(node->children[0], "IDENT")) {
        callNode->value = extractTokenValue(node->children[0]->name);
    }

    for (ParseNode* child : node->children) {
        if (child->name != "<parameter-list>") continue;
        for (ParseNode* param : child->children) {
            if (param->name == "<expression>") callNode->addChild(buildExpression(param));
        }
    }
    return callNode;
}

ASTNode* ASTBuilder::buildConstant(ParseNode* node) {
    if (!node) return nullptr;

    bool negative = false;
    for (ParseNode* child : node->children) {
        if (isToken(child, "MINUS")) {
            negative = true;
        } else if (isToken(child, "PLUS")) {
            negative = false;
        } else if (isToken(child, "IDENT")) {
            return new ASTNode(ASTKind::Var, extractTokenValue(child->name));
        } else if (isToken(child, "INTCON") || isToken(child, "REALCON") ||
                   isToken(child, "CHARCON") || isToken(child, "STRING")) {
            std::string value = extractTokenValue(child->name);
            if (negative) value = "-" + value;
            return new ASTNode(ASTKind::Literal, value);
        }
    }

    if (isToken(node, "INTCON") || isToken(node, "REALCON") ||
        isToken(node, "CHARCON") || isToken(node, "STRING")) {
        return new ASTNode(ASTKind::Literal, extractTokenValue(node->name));
    }
    if (isToken(node, "IDENT")) return new ASTNode(ASTKind::Var, extractTokenValue(node->name));

    return new ASTNode(ASTKind::Empty);
}

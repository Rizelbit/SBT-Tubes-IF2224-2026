#include "DecoratedASTReader.hpp"
#include "TypeSystem.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace {

std::string trim(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

bool startsWith(const std::string& s, const std::string& prefix) {
    return s.rfind(prefix, 0) == 0;
}

std::vector<std::string> splitWords(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> words;
    std::string word;
    while (iss >> word) words.push_back(word);
    return words;
}

TypeKind typeKindFromString(const std::string& s) {
    static const std::unordered_map<std::string, TypeKind> types = {
        {"Unknown", TypeKind::Unknown}, {"Void", TypeKind::Void},
        {"Integer", TypeKind::Integer}, {"Real", TypeKind::Real},
        {"Char", TypeKind::Char}, {"Boolean", TypeKind::Boolean},
        {"String", TypeKind::String}, {"Subrange", TypeKind::Subrange},
        {"Enumerated", TypeKind::Enumerated}, {"Array", TypeKind::Array},
        {"Record", TypeKind::Record}, {"Procedure", TypeKind::Procedure},
        {"Function", TypeKind::Function}
    };
    auto it = types.find(s);
    return (it == types.end()) ? TypeKind::Unknown : it->second;
}

bool objectKindFromString(const std::string& s, ObjectKind& out) {
    static const std::vector<std::pair<std::string, ObjectKind>> objects = {
        {"Program", ObjectKind::Program}, {"Constant", ObjectKind::Constant},
        {"Type", ObjectKind::Type}, {"Variable", ObjectKind::Variable},
        {"VarParam", ObjectKind::VarParam}, {"Parameter", ObjectKind::Parameter},
        {"Field", ObjectKind::Field}, {"Procedure", ObjectKind::Procedure},
        {"Function", ObjectKind::Function}
    };
    for (const auto& kv : objects) {
        if (s == kv.first) {
            out = kv.second;
            return true;
        }
    }
    return false;
}

bool splitIdentifierAndObject(const std::string& token, std::string& identifier, ObjectKind& object) {
    static const std::vector<std::pair<std::string, ObjectKind>> suffixes = {
        {"Procedure", ObjectKind::Procedure}, {"Function", ObjectKind::Function},
        {"Variable", ObjectKind::Variable}, {"Parameter", ObjectKind::Parameter},
        {"VarParam", ObjectKind::VarParam}, {"Constant", ObjectKind::Constant},
        {"Program", ObjectKind::Program}, {"Field", ObjectKind::Field},
        {"Type", ObjectKind::Type}
    };

    for (const auto& kv : suffixes) {
        const std::string& suffix = kv.first;
        if (token.size() > suffix.size()
            && token.compare(token.size() - suffix.size(), suffix.size(), suffix) == 0) {
            identifier = token.substr(0, token.size() - suffix.size());
            object = kv.second;
            return !identifier.empty();
        }
    }
    return false;
}

ASTKind astKindFromString(const std::string& s) {
    static const std::unordered_map<std::string, ASTKind> kinds = {
        {"Program", ASTKind::Program}, {"Declarations", ASTKind::DeclarationPart},
        {"Block", ASTKind::Block}, {"ConstDecl", ASTKind::ConstDecl},
        {"TypeDecl", ASTKind::TypeDecl}, {"VarDecl", ASTKind::VarDecl},
        {"FieldDecl", ASTKind::FieldDecl}, {"ProcedureDecl", ASTKind::ProcDecl},
        {"FunctionDecl", ASTKind::FuncDecl}, {"Param", ASTKind::Param},
        {"Type", ASTKind::Type}, {"ArrayType", ASTKind::ArrayType},
        {"RecordType", ASTKind::RecordType}, {"RangeType", ASTKind::RangeType},
        {"EnumType", ASTKind::EnumType}, {"Assign", ASTKind::Assign},
        {"If", ASTKind::If}, {"While", ASTKind::While},
        {"Repeat", ASTKind::Repeat}, {"For", ASTKind::For},
        {"Case", ASTKind::Case}, {"ProcedureCall", ASTKind::ProcedureCall},
        {"FunctionCall", ASTKind::FunctionCall}, {"BinOp", ASTKind::BinOp},
        {"UnaryOp", ASTKind::UnaryOp}, {"Literal", ASTKind::Literal},
        {"Var", ASTKind::Var}, {"ArrayAccess", ASTKind::ArrayAccess},
        {"FieldAccess", ASTKind::FieldAccess}, {"EmptyStatement", ASTKind::Empty}
    };
    auto it = kinds.find(s);
    if (it == kinds.end()) {
        throw std::runtime_error("Unknown Decorated AST node kind: " + s);
    }
    return it->second;
}

int parseIntField(const std::string& metadata, const std::string& key, int defaultValue) {
    std::string marker = key + "=";
    size_t start = metadata.find(marker);
    if (start == std::string::npos) return defaultValue;
    start += marker.size();
    size_t end = metadata.find_first_of(",]", start);
    return std::stoi(trim(metadata.substr(start, end - start)));
}

SemanticType parseTypeField(const std::string& metadata) {
    std::string marker = "type=";
    size_t start = metadata.find(marker);
    if (start == std::string::npos) return makeUnknownType();
    start += marker.size();
    size_t end = metadata.find_first_of(",]", start);
    return makeType(typeKindFromString(trim(metadata.substr(start, end - start))));
}

SemanticType parsePrintedTypeToken(const std::string& token, int& gluedRef, bool& hasGluedRef) {
    hasGluedRef = false;
    gluedRef = 0;

    size_t refStart = token.find("[ref=");
    if (refStart == std::string::npos) {
        return makeType(typeKindFromString(token));
    }

    std::string kind = token.substr(0, refStart);
    size_t refValueStart = refStart + std::string("[ref=").size();
    size_t refEnd = token.find(']', refValueStart);
    if (refEnd == std::string::npos) {
        return makeType(typeKindFromString(kind));
    }

    SemanticType type = makeType(typeKindFromString(kind));
    type.ref = std::stoi(token.substr(refValueStart, refEnd - refValueStart));

    std::string trailing = token.substr(refEnd + 1);
    if (!trailing.empty()) {
        gluedRef = std::stoi(trailing);
        hasGluedRef = true;
    }
    return type;
}

int treeDepthAndContent(const std::string& line, std::string& content) {
    size_t marker = line.find("── ");
    if (marker == std::string::npos) {
        content = trim(line);
        return 0;
    }

    std::string prefix = line.substr(0, marker);
    content = line.substr(marker + std::string("── ").size());

    int groups = 0;
    size_t pos = 0;
    const std::string pipeGroup = u8"│   ";
    while (pos < prefix.size()) {
        if (prefix.compare(pos, pipeGroup.size(), pipeGroup) == 0) {
            pos += pipeGroup.size();
            groups++;
        } else if (prefix.compare(pos, 4, "    ") == 0) {
            pos += 4;
            groups++;
        } else {
            break;
        }
    }
    return groups + 1;
}

ASTNode* parseASTLine(const std::string& line) {
    size_t metaStart = line.find(" [");
    std::string nodeText = (metaStart == std::string::npos) ? trim(line) : line.substr(0, metaStart);
    std::string metadata = (metaStart == std::string::npos) ? "" : line.substr(metaStart + 2);

    std::string kindText;
    std::string value;
    size_t open = nodeText.find('(');
    if (open == std::string::npos) {
        kindText = trim(nodeText);
    } else {
        kindText = trim(nodeText.substr(0, open));
        size_t close = nodeText.rfind(')');
        if (close == std::string::npos || close < open) {
            throw std::runtime_error("Malformed Decorated AST node: " + line);
        }
        value = nodeText.substr(open + 1, close - open - 1);
    }

    ASTNode* node = new ASTNode(astKindFromString(kindText));
    if (node->kind == ASTKind::For) {
        const std::string marker = ", direction=";
        size_t direction = value.find(marker);
        if (direction != std::string::npos) {
            node->value = trim(value.substr(0, direction));
            node->attribute = trim(value.substr(direction + marker.size()));
        } else {
            node->value = value;
        }
    } else if (node->kind == ASTKind::Empty && node->value.empty()) {
        node->value = "";
    } else {
        node->value = value;
    }

    node->inferredType = parseTypeField(metadata);
    node->tabIndex = parseIntField(metadata, "tab", -1);
    node->blockIndex = parseIntField(metadata, "block", -1);
    node->lexicalLevel = parseIntField(metadata, "level", 0);
    return node;
}

void ensureTabIndex(std::vector<TabEntry>& tab, int idx) {
    while (static_cast<int>(tab.size()) <= idx) {
        TabEntry dummy;
        dummy.identifier = "__reserved__";
        dummy.lev = -1;
        tab.push_back(dummy);
    }
}

} // namespace

bool DecoratedASTReader::canRead(const std::string& filename) const {
    std::ifstream in(filename);
    if (!in.is_open()) return false;

    bool hasAST = false;
    bool hasTab = false;
    std::string line;
    while (std::getline(in, line)) {
        if (trim(line) == "Decorated AST:") hasAST = true;
        if (trim(line) == "Symbol Table - tab:") hasTab = true;
        if (hasAST && hasTab) return true;
    }
    return false;
}

DecoratedASTInput DecoratedASTReader::read(const std::string& filename) const {
    std::ifstream in(filename);
    if (!in.is_open()) {
        throw std::runtime_error("Cannot open Decorated AST input file: " + filename);
    }

    enum class Section { None, AST, Tab, BTab, ATab };
    Section section = Section::None;

    DecoratedASTInput result;
    std::vector<ASTNode*> stack;
    std::vector<TabEntry> tab;
    std::vector<BTabEntry> btab;
    std::vector<ATabEntry> atab(1);

    std::string line;
    while (std::getline(in, line)) {
        std::string stripped = trim(line);
        if (stripped == "Decorated AST:") {
            section = Section::AST;
            continue;
        }
        if (stripped == "Symbol Table - tab:") {
            section = Section::Tab;
            continue;
        }
        if (stripped == "Symbol Table - btab:") {
            section = Section::BTab;
            continue;
        }
        if (stripped == "Symbol Table - atab:") {
            section = Section::ATab;
            continue;
        }
        if (stripped == "Intermediate Code:" || stripped == "Program Output:" || startsWith(stripped, "Runtime Status:")) {
            section = Section::None;
            continue;
        }
        if (stripped.empty() || startsWith(stripped, "---") || startsWith(stripped, "idx ")) {
            continue;
        }

        if (section == Section::AST) {
            std::string content;
            int depth = treeDepthAndContent(line, content);
            ASTNode* node = parseASTLine(content);

            if (depth == 0) {
                if (result.root != nullptr) {
                    delete node;
                    throw std::runtime_error("Decorated AST contains more than one root");
                }
                result.root = node;
            } else {
                if (depth > static_cast<int>(stack.size())) {
                    delete node;
                    throw std::runtime_error("Malformed Decorated AST indentation near: " + content);
                }
                stack[depth - 1]->addChild(node);
            }

            if (depth >= static_cast<int>(stack.size())) {
                stack.resize(depth + 1);
            }
            stack[depth] = node;
            stack.resize(depth + 1);
        } else if (section == Section::Tab) {
            std::vector<std::string> words = splitWords(line);
            if (words.size() < 9) continue;

            int idx = std::stoi(words[0]);
            std::string identifier = words[1];
            ObjectKind obj;
            size_t typePos = 3;
            if (objectKindFromString(words[2], obj)) {
                typePos = 3;
            } else if (splitIdentifierAndObject(words[1], identifier, obj)) {
                typePos = 2;
            } else {
                throw std::runtime_error("Malformed tab row: " + line);
            }

            int gluedRef = 0;
            bool hasGluedRef = false;
            SemanticType printedType = parsePrintedTypeToken(words[typePos], gluedRef, hasGluedRef);
            size_t refPos = typePos + 1;
            size_t nrmPos = hasGluedRef ? typePos + 1 : typePos + 2;
            if (words.size() <= nrmPos + 4) {
                throw std::runtime_error("Incomplete tab row: " + line);
            }

            TabEntry entry;
            entry.identifier = identifier;
            entry.obj = obj;
            entry.type = printedType;
            entry.ref = hasGluedRef ? gluedRef : std::stoi(words[refPos]);
            if (entry.type.ref == 0) {
                entry.type.ref = entry.ref;
            }
            entry.nrm = std::stoi(words[nrmPos]);
            entry.lev = std::stoi(words[nrmPos + 1]);
            entry.adr = std::stoi(words[nrmPos + 2]);
            entry.link = std::stoi(words[nrmPos + 3]);
            entry.initialized = words[nrmPos + 4] == "yes";

            ensureTabIndex(tab, idx);
            tab[idx] = entry;
        } else if (section == Section::BTab) {
            std::vector<std::string> words = splitWords(line);
            if (words.size() < 5) continue;
            int idx = std::stoi(words[0]);
            if (static_cast<int>(btab.size()) <= idx) btab.resize(idx + 1);
            btab[idx] = {std::stoi(words[1]), std::stoi(words[2]), std::stoi(words[3]), std::stoi(words[4])};
        } else if (section == Section::ATab) {
            std::vector<std::string> words = splitWords(line);
            if (words.size() < 8) continue;
            int idx = std::stoi(words[0]);
            if (static_cast<int>(atab.size()) <= idx) atab.resize(idx + 1);
            ATabEntry entry;
            entry.xtyp = makeType(typeKindFromString(words[1]));
            entry.etyp = makeType(typeKindFromString(words[2]));
            entry.eref = std::stoi(words[3]);
            entry.low = std::stoi(words[4]);
            entry.high = std::stoi(words[5]);
            entry.elsz = std::stoi(words[6]);
            entry.size = std::stoi(words[7]);
            atab[idx] = entry;
        }
    }

    if (result.root == nullptr) {
        throw std::runtime_error("Decorated AST input does not contain an AST root");
    }
    result.symbolTable.loadFromTables(tab, btab, atab);
    return result;
}

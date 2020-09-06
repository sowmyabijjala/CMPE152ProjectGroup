/**
 * Parse tree node class for a simple interpreter.
 *
 * (c) 2020 by Ronald Mak
 * Department of Computer Science
 * San Jose State University
 */
#ifndef NODE_H_
#define NODE_H_

#include <string>
#include <vector>

#include "../Object.h"
#include "SymtabEntry.h"

namespace intermediate {

using namespace std;

enum class NodeType
{
    PROGRAM, COMPOUND, ASSIGN, LOOP, TEST, WRITE, WRITELN,
    ADD, SUBTRACT, MULTIPLY, DIVIDE, EQ, LT,
    VARIABLE, INTEGER_CONSTANT, REAL_CONSTANT, STRING_CONSTANT
};

static const string NODE_TYPE_STRINGS[] =
{
    "PROGRAM", "COMPOUND", "ASSIGN", "LOOP", "TEST", "WRITE", "WRITELN",
    "ADD", "SUBTRACT", "MULTIPLY", "DIVIDE", "EQ", "LT",
    "VARIABLE", "INTEGER_CONSTANT", "REAL_CONSTANT", "STRING_CONSTANT"
};

constexpr NodeType PROGRAM          = NodeType::PROGRAM;
constexpr NodeType COMPOUND         = NodeType::COMPOUND;
constexpr NodeType ASSIGN           = NodeType::ASSIGN;
constexpr NodeType LOOP             = NodeType::LOOP;
constexpr NodeType TEST             = NodeType::TEST;
constexpr NodeType WRITE            = NodeType::WRITE;
constexpr NodeType WRITELN          = NodeType::WRITELN;
constexpr NodeType ADD              = NodeType::ADD;
constexpr NodeType SUBTRACT         = NodeType::SUBTRACT;
constexpr NodeType MULTIPLY         = NodeType::MULTIPLY;
constexpr NodeType DIVIDE           = NodeType::DIVIDE;
constexpr NodeType EQ               = NodeType::EQ;
constexpr NodeType LT               = NodeType::LT;
constexpr NodeType VARIABLE         = NodeType::VARIABLE;
constexpr NodeType INTEGER_CONSTANT = NodeType::INTEGER_CONSTANT;
constexpr NodeType REAL_CONSTANT    = NodeType::REAL_CONSTANT;
constexpr NodeType STRING_CONSTANT  = NodeType::STRING_CONSTANT;

class Node
{
public:
    NodeType type;
    int lineNumber;
    string text;
    SymtabEntry *entry;
    Object value;
    vector<Node *> children;

    Node(NodeType type)
        : type(type), lineNumber(0), entry(nullptr) {}

    void adopt(Node *child) { children.push_back(child); }
};

}  // namespace intermediate

#endif /* NODE_H_ */

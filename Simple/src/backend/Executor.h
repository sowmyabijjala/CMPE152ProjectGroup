/**
 * Executor class for a simple interpreter.
 *
 * (c) 2020 by Ronald Mak
 * Department of Computer Science
 * San Jose State University
 */
#ifndef EXECUTOR_H_
#define EXECUTOR_H_

#include <string>
#include <vector>
#include <set>

#include "../Object.h"
#include "../intermediate/Symtab.h"
#include "../intermediate/Node.h"

namespace backend {

using namespace std;
using namespace intermediate;

class Executor
{
private:
    int lineNumber;
    Symtab *symtab;

public:
    /**
     * Initialize the static sets.
     */
    static void initialize();

    Executor(Symtab *symtab) : lineNumber(0), symtab(symtab) {}

    Object visit(Node *node);

private:
    static set<NodeType> singletons;   // singleton factors
    static set<NodeType> relationals;  // relational operators

    Object visitProgram(Node *programNode);
    Object visitStatement(Node *statementNode);
    Object visitCompound(Node *compoundNode);
    Object visitAssign(Node *assignNode);
    Object visitLoop(Node *loopNode);
    Object visitTest(Node *testNode);
    Object visitWrite(Node *writeNode);
    Object visitWriteln(Node *writelnNode);
    Object visitExpression(Node *expressionNode);
    Object visitVariable(Node *variableNode);
    Object visitIntegerConstant(Node *integerConstantNode);
    Object visitRealConstant(Node *realConstantNode);
    Object visitStringConstant(Node *stringConstantNode);

    void printValue(vector<Node *> children);
    void runtimeError(Node *node, string message);
};

}  // namespace backend

#endif /* EXECUTOR_H_ */

/**
 * Parser class for a simple interpreter.
 *
 * (c) 2020 by Ronald Mak
 * Department of Computer Science
 * San Jose State University
 */
#ifndef PARSER_H_
#define PARSER_H_

#include <set>

#include "Scanner.h"
#include "Token.h"
#include "../intermediate/Symtab.h"
#include "../intermediate/Node.h"

namespace frontend {

using namespace std;
using namespace intermediate;

class Parser
{
private:
    Scanner *scanner;
    Symtab *symtab;
    Token *currentToken;
    int lineNumber;
    int errorCount;

    static set<TokenType> statementStarters;          // what starts a statement
    static set<TokenType> statementFollowers;         // what follows a statement
    static set<TokenType> relationalOperators;        // relational operators
    static set<TokenType> simpleExpressionOperators;  // simple expression operators
    static set<TokenType> termOperators;              // term operators
    static set<TokenType> factorOperators;			  // factor operators (but this is just NOT)

public:
    /**
     * Initialize the static sets.
     */
    static void initialize();

    Parser(Scanner *scanner, Symtab *symtab)
        : scanner(scanner), symtab(symtab), currentToken(nullptr),
          lineNumber(1), errorCount(0) {}

    int getErrorCount() const { return errorCount; }

    Node *parseProgram();

private:
    Node *parseStatement();
    Node *parseAssignmentStatement();
    Node *parseCompoundStatement();
    Node *parseRepeatStatement();
    Node *parseWhileStatement();
    Node *parseIfStatement();
    Node *parseWriteStatement();
    Node *parseWritelnStatement();
    Node *parseExpression();
    Node *parseSimpleExpression();
    Node *parseTerm();
    Node *parseFactor();
    Node *parseVariable();
    Node *parseIntegerConstant();
    Node *parseRealConstant();
    Node *parseStringConstant();

    void parseStatementList(Node *parentNode, TokenType terminalType);
    void parseWriteArguments(Node *node);

    void syntaxError(string message);
    void semanticError(string message);
};

}  // namespace frontend

#endif /* PARSER_H_ */

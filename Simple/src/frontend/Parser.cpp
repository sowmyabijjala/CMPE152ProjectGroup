/**
 * Parser class for a simple interpreter.
 *
 * (c) 2020 by Ronald Mak
 * Department of Computer Science
 * San Jose State University
 */
#include <string>
#include <map>

#include "Token.h"
#include "Parser.h"

namespace frontend {

using namespace std;

set<TokenType> Parser::statementStarters;
set<TokenType> Parser::statementFollowers;
set<TokenType> Parser::relationalOperators;
set<TokenType> Parser::simpleExpressionOperators;
set<TokenType> Parser::termOperators;
set<TokenType> Parser::factorOperators;

void Parser::initialize()
{
    statementStarters.insert(BEGIN);
    statementStarters.insert(IDENTIFIER);
    statementStarters.insert(REPEAT);
    statementStarters.insert(WHILE);
    statementStarters.insert(FOR);
    statementStarters.insert(TokenType::WRITE);
    statementStarters.insert(TokenType::WRITELN);

    statementFollowers.insert(SEMICOLON);
    statementFollowers.insert(END);
    statementFollowers.insert(UNTIL);
    statementFollowers.insert(DO);
    statementFollowers.insert(TO);
    statementFollowers.insert(DOWNTO);
    statementFollowers.insert(END_OF_FILE);

    //expression operators
    relationalOperators.insert(EQUALS);
    relationalOperators.insert(LESS_THAN);
    //LESS EQUALS, GREATER THAN, GREATER EQUALS, NOT EQUALS

    simpleExpressionOperators.insert(PLUS);
    simpleExpressionOperators.insert(MINUS);
    //OR

    termOperators.insert(STAR);
    termOperators.insert(SLASH);
    //DIV, MOD, AND

    //factorOperators.insert(NOT);
    factorOperators.insert(TokenType::NOT);
}

Node *Parser::parseProgram()
{
    Node *programNode = new Node(NodeType::PROGRAM);

    currentToken = scanner->nextToken();  // first token!

    if (currentToken->type == TokenType::PROGRAM)
    {
        currentToken = scanner->nextToken();  // consume PROGRAM
    }
    else syntaxError("Expecting PROGRAM");

    if (currentToken->type == IDENTIFIER)
    {
        string programName = currentToken->text;
        symtab->enter(programName);
        programNode->text = programName;

        currentToken = scanner->nextToken();  // consume program name
    }
    else syntaxError("Expecting program name");

    if (currentToken->type == SEMICOLON)
    {
        currentToken = scanner->nextToken();  // consume ;
    }
    else syntaxError("Missing ;");

    if (currentToken->type != BEGIN) syntaxError("Expecting BEGIN");

    // The PROGRAM node adopts the COMPOUND tree.
    programNode->adopt(parseCompoundStatement());

    if (currentToken->type == SEMICOLON) syntaxError("Expecting .");
    return programNode;
}

Node *Parser::parseStatement()
{
    Node *stmtNode = nullptr;
    int savedLineNumber = currentToken->lineNumber;
    lineNumber = savedLineNumber;

    switch (currentToken->type)
    {
        case IDENTIFIER : stmtNode = parseAssignmentStatement(); break;
        case BEGIN :      stmtNode = parseCompoundStatement();   break;
        case REPEAT :     stmtNode = parseRepeatStatement();     break;
        case WHILE:		  stmtNode = parseWhileStatement(); 	 break;
        case FOR:		  stmtNode = parseForStatement();		 break;
        case WRITE :      stmtNode = parseWriteStatement();      break;
        case WRITELN :    stmtNode = parseWritelnStatement();    break;
        case SEMICOLON :  stmtNode = nullptr; break;  // empty statement

        default : syntaxError("Unexpected token");
    }

    if (stmtNode != nullptr) stmtNode->lineNumber = savedLineNumber;
    return stmtNode;
}

Node *Parser::parseAssignmentStatement()
{
    // The current token should now be the left-hand-side variable name.

    Node *assignmentNode = new Node(ASSIGN);

    // The assignment Node *adopts the variable Node *as its first child.
    Node *lhsNode = new Node(VARIABLE);

    string variableName = currentToken->text;

    SymtabEntry *variableId = symtab->enter(toLowerCase(variableName));

    lhsNode->text  = variableName;
    lhsNode->entry = variableId;
    assignmentNode->adopt(lhsNode);

    currentToken = scanner->nextToken();  // consume the LHS variable;

    if (currentToken->type == COLON_EQUALS)
    {
        currentToken = scanner->nextToken();  // consume :=
    }
    else syntaxError("Missing :=");

    // The assignment Node *adopts the expression Node *as its second child.
    Node *rhsNode = parseExpression();
    assignmentNode->adopt(rhsNode);

    //for(int i=0; i < assignmentNode->children.size(); i++)

    return assignmentNode;
}

Node *Parser::parseForAssignmentStatement(string var)
{
    // The current token should now be the left-hand-side variable name.

    Node *assignmentNode = new Node(ASSIGN);

    // The assignment Node *adopts the variable Node *as its first child.
    Node *lhsNode = new Node(VARIABLE);

    string variableName = var;

    SymtabEntry *variableId = symtab->enter(toLowerCase(variableName));

    lhsNode->text  = variableName;
    lhsNode->entry = variableId;
    assignmentNode->adopt(lhsNode);

    // The assignment Node *adopts the expression Node *as its second child.
    Node *rhsNode = parseExpression();
    assignmentNode->adopt(rhsNode);


    //for(int i=0; i < assignmentNode->children.size(); i++)
    	   //printf("in for Size '%d' and text '%s'", assignmentNode->children.size(),assignmentNode->children.at(i)->text.c_str());
    return assignmentNode;
}

Node *Parser::parseCompoundStatement()
{
    Node *compoundNode = new Node(COMPOUND);
    compoundNode->lineNumber = currentToken->lineNumber;

    currentToken = scanner->nextToken();  // consume BEGIN
    parseStatementList(compoundNode, END);

    if (currentToken->type == END)
    {
        currentToken = scanner->nextToken();  // consume END
    }
    else syntaxError("Expecting END");

    return compoundNode;
}

void Parser::parseStatementList(Node *parentNode, TokenType terminalType)
{
    while (   (currentToken->type != terminalType)
           && (currentToken->type != END_OF_FILE))
    {
        Node *stmtNode = parseStatement();
        if (stmtNode != nullptr) parentNode->adopt(stmtNode);

        // A semicolon separates statements.
        if (currentToken->type == SEMICOLON)
        {
            while (currentToken->type == SEMICOLON)
            {
                currentToken = scanner->nextToken();  // consume ;
            }
        }
        else if (statementStarters.find(currentToken->type) !=
                                                        statementStarters.end())
        {
            syntaxError("Missing ;");
        }
    }
}

Node *Parser::parseRepeatStatement()
{
    // The current token should now be REPEAT.

    // Create a LOOP node->
    Node *loopNode = new Node(LOOP);
    currentToken = scanner->nextToken();  // consume REPEAT

    parseStatementList(loopNode, UNTIL);

    if (currentToken->type == UNTIL)
    {
        // Create a TEST node-> It adopts the test expression node->
        Node *testNode = new Node(TEST);
        lineNumber = currentToken->lineNumber;
        testNode->lineNumber = lineNumber;
        currentToken = scanner->nextToken();  // consume UNTIL

        testNode->adopt(parseExpression());

        // The LOOP Node *adopts the TEST Node *as its final child.
        loopNode->adopt(testNode);
    }
    else syntaxError("Expecting UNTIL");

    return loopNode;
}

Node *Parser::parseWhileStatement()
{
    // The current token should now be WHILE.

	// Create a LOOP node->
	Node *loopNode = new Node(LOOP);
	currentToken = scanner->nextToken(); // consume WHILE

	Node *testNode = new Node(TEST);
	Node *notNode = new Node(NodeType::NOT);

	notNode->adopt(parseExpression());
	testNode->adopt(notNode);
	loopNode->adopt(testNode);

	//missing some line # stuff still

	if (currentToken->type == DO) {
		currentToken = scanner->nextToken(); //consume DO?
		loopNode->adopt(parseStatement());
	}
	else
		syntaxError("Expecting DO");

    return loopNode;
}

Node *Parser::parseForStatement()
{
    // for i := 1 to 5 do begin

	// Create a LOOP node->
	bool to = false;
	Node *forNode = new Node(NodeType::FOR);
	currentToken = scanner->nextToken(); // consume FOR

	printf("finish FOR now: '%s' \n", currentToken->text.c_str());

	//FOR NODE currently has ASSIGN
	forNode->adopt(parseAssignmentStatement()); //assignment statement
	printf("finish assignment now: '%s' \n", currentToken->text.c_str());

	Node *tempNode = forNode->children[0]; //should technically carry the variable assignment
    tempNode = tempNode->children[0];

	Node *loopNode = new Node(LOOP);

	if (currentToken->type == TO)
	{
		to = true;
		currentToken = scanner->nextToken(); //consume TO
		Node *testNode = new Node(TEST);
		Node *opNode =  new Node(GT);
		opNode->adopt(parseForAssignmentStatement(tempNode->text)); // add assignment statement
		testNode->adopt(opNode);//add GT to parse tree for TO
		printf("temp node's string TO '%s' \n", tempNode->text.c_str());

		//LOOP NODE currently has TEST
		loopNode->adopt(testNode);

	}
	else if (currentToken->type == DOWNTO)
	{
		//should be the same as TO, will add code here after I fix the TO part
		currentToken = scanner->nextToken(); //consume TO
		Node *testNode = new Node(TEST);
		Node *opNode =  new Node(LT);
		testNode->adopt(opNode);//add GT to parse tree for TO
		testNode->adopt(parseForAssignmentStatement(tempNode->text)); // add assignment statement

		//LOOP NODE currently has TEST
		loopNode->adopt(testNode);
	}
	else syntaxError("Expecting TO or DOWNTO");

	//current token should be DO
	printf("Should be DO: '%s' \n", currentToken->text.c_str());
	if (currentToken->type == DO) {
			currentToken = scanner->nextToken(); //consume DO

			//LOOP NODE currently has TEST and ASSIGN
			loopNode->adopt(parseStatement());
		}
		else
			syntaxError("Expecting DO");

	Node *assignmentNode = new Node(ASSIGN);
	Node *lhsNode = new Node(VARIABLE);

	SymtabEntry *variableId = symtab->enter(toLowerCase(tempNode->text));
	Node *optoNode = to ? new Node(ADD): new Node(SUBTRACT);

	lhsNode->text  = tempNode->text;
	lhsNode->entry = variableId;
	optoNode->adopt(lhsNode); //add var child to optonode
	Node *integerNode = new Node(INTEGER_CONSTANT);
	integerNode->value.L = 1;
	optoNode->adopt(integerNode);

	assignmentNode->adopt(optoNode);

	//LOOP NODE currently has TEST, ASSIGN and ASSIGN
	loopNode->adopt(assignmentNode);

	//FOR NODE currently has ASSIGN AND LOOP
	forNode->adopt(loopNode); //assignment statement

    return forNode;
}

Node *Parser::parseWriteStatement()
{
    // The current token should now be WRITE.

    // Create a WRITE node-> It adopts the variable or string node->
    Node *writeNode = new Node(NodeType::WRITE);
    currentToken = scanner->nextToken();  // consume WRITE

    parseWriteArguments(writeNode);
    if (writeNode->children.size() == 0)
    {
        syntaxError("Invalid WRITE statement");
    }

    return writeNode;
}

Node *Parser::parseWritelnStatement()
{
    // The current token should now be WRITELN.

    // Create a WRITELN node-> It adopts the variable or string node->
    Node *writelnNode = new Node(NodeType::WRITELN);
    currentToken = scanner->nextToken();  // consume WRITELN

    if (currentToken->type == LPAREN) parseWriteArguments(writelnNode);
    return writelnNode;
}

void Parser::parseWriteArguments(Node *node)
{
    // The current token should now be (

    bool hasArgument = false;

    if (currentToken->type == LPAREN)
    {
        currentToken = scanner->nextToken();  // consume (
    }
    else syntaxError("Missing left parenthesis");

    if (currentToken->type == IDENTIFIER)
    {
        node->adopt(parseVariable());
        hasArgument = true;
    }
    else if (currentToken->type == STRING)
    {
        node->adopt(parseStringConstant());
        hasArgument = true;
    }
    else syntaxError("Invalid WRITE or WRITELN statement");

    // Look for a field width and a count of decimal places.
    if (hasArgument)
    {
        if (currentToken->type == COLON)
        {
            currentToken = scanner->nextToken();  // consume ,

            if (currentToken->type == INTEGER)
            {
                // Field width
                node->adopt(parseIntegerConstant());

                if (currentToken->type == COLON)
                {
                    currentToken = scanner->nextToken();  // consume ,

                    if (currentToken->type == INTEGER)
                    {
                        // Count of decimal places
                        node->adopt(parseIntegerConstant());
                    }
                    else syntaxError("Invalid count of decimal places");
                }
            }
            else syntaxError("Invalid field width");
        }
    }

    if (currentToken->type == RPAREN)
    {
        currentToken = scanner->nextToken();  // consume )
    }
    else syntaxError("Missing right parenthesis");
}

Node *Parser::parseExpression()
{
    // The current token should now be an identifier or a number.

    // The expression's root node->
    Node *exprNode = parseSimpleExpression();

    // The current token might now be a relational operator.
    if (relationalOperators.find(currentToken->type) != relationalOperators.end())
    {
        TokenType tokenType = currentToken->type;
        Node *opNode = tokenType == EQUALS    ? new Node(EQ)
                    : tokenType == LESS_THAN ? new Node(LT)
                    :                          nullptr;

        currentToken = scanner->nextToken();  // consume relational operator

        // The relational operator Node *adopts the first simple expression
        // Node *as its first child and the second simple expression node
        // as its second child. Then it becomes the expression's root node->
        if (opNode != nullptr)
        {
            opNode->adopt(exprNode);
            opNode->adopt(parseSimpleExpression());
            exprNode = opNode;
        }
    }

    return exprNode;
}

Node *Parser::parseSimpleExpression()
{
    // The current token should now be an identifier or a number.

    // The simple expression's root node->
    Node *simpExprNode = parseTerm();

    // Keep parsing more terms as long as the current token
    // is a + or - operator.
    while (simpleExpressionOperators.find(currentToken->type) !=
                                                simpleExpressionOperators.end())
    {
        Node *opNode = currentToken->type == PLUS ? new Node(ADD)
                                                : new Node(SUBTRACT);

        currentToken = scanner->nextToken();  // consume the operator

        // The add or subtract Node *adopts the first term Node *as its
        // first child and the next term Node *as its second child.
        // Then it becomes the simple expression's root node->
        opNode->adopt(simpExprNode);
        opNode->adopt(parseTerm());
        simpExprNode = opNode;
    }

    return simpExprNode;
}

Node *Parser::parseTerm()
{
    // The current token should now be an identifier or a number.

    // The term's root node->
    Node *termNode = parseFactor();

    // Keep parsing more factor as long as the current token
    // is a * or / operator.
    while (termOperators.find(currentToken->type) != termOperators.end())
    {
        Node *opNode = currentToken->type == STAR ? new Node(MULTIPLY)
                                                : new Node(DIVIDE);

        currentToken = scanner->nextToken();  // consume the operator

        // The multiply or dive Node *adopts the first factor Node *as its
        // as its first child and the next factor Node *as its second child.
        // Then it becomes the term's root node->
        opNode->adopt(termNode);
        opNode->adopt(parseFactor());
        termNode = opNode;
    }

    return termNode;
}

Node *Parser::parseFactor()
{
    // The current token should now be an identifier or a number or (

    if      (currentToken->type == IDENTIFIER) return parseVariable();
    else if (currentToken->type == INTEGER)    return parseIntegerConstant();
    else if (currentToken->type == REAL)       return parseRealConstant();

    else if (currentToken->type == LPAREN)
    {
        currentToken = scanner->nextToken();  // consume (
        Node *exprNode = parseExpression();

        if (currentToken->type == RPAREN)
        {
            currentToken = scanner->nextToken();  // consume )
        }
        else syntaxError("Expecting )");

        return exprNode;
    }

    else syntaxError("Unexpected token");
    return nullptr;
}

Node *Parser::parseVariable()
{
    // The current token should now be an identifier.

    string variableName = currentToken->text;
    SymtabEntry *variableId = symtab->lookup(toLowerCase(variableName));

    if (variableId == nullptr) semanticError("Undeclared identifier");

    Node *node = new Node(VARIABLE);
    node->text = variableName;

    currentToken = scanner->nextToken();  // consume the identifier
    return node;
}

Node *Parser::parseIntegerConstant()
{
    // The current token should now be a number.

    Node *integerNode = new Node(INTEGER_CONSTANT);
    integerNode->value = currentToken->value;

    currentToken = scanner->nextToken();  // consume the number
    return integerNode;
}

Node *Parser::parseRealConstant()
{
    // The current token should now be a number.

    Node *realNode = new Node(REAL_CONSTANT);
    realNode->value = currentToken->value;

    currentToken = scanner->nextToken();  // consume the number
    return realNode;
}

Node *Parser::parseStringConstant()
{
    // The current token should now be string.

    Node *stringNode = new Node(STRING_CONSTANT);
    stringNode->value = currentToken->value;

    currentToken = scanner->nextToken();  // consume the string
    return stringNode;
}

void Parser::syntaxError(string message)
{
    printf("SYNTAX ERROR at line %d: %s at '%s'\n",
           lineNumber, message.c_str(), currentToken->text.c_str());
    errorCount++;

    currentToken = scanner->nextToken();
    //printf("recovery attempt");
    // Recover by skipping the rest of the statement.
    // Skip to a statement follower token.
    while (statementFollowers.find(currentToken->type) ==
                                                    statementFollowers.end())
    {
        currentToken = scanner->nextToken();
    }
}

void Parser::semanticError(string message)
{
    printf("SEMANTIC ERROR at line %d: %s at '%s'\n",
           lineNumber, message.c_str(), currentToken->text.c_str());
    errorCount++;
}

}  // namespace frontend

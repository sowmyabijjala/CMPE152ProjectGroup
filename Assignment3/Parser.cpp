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

void Parser::initialize() {
	// Tokens that can start a statement.
	statementStarters.insert(BEGIN);
	statementStarters.insert(IDENTIFIER);
	statementStarters.insert(REPEAT);
	statementStarters.insert(WHILE);
	statementStarters.insert(DO);
	statementStarters.insert(IF);
	statementStarters.insert(TokenType::WRITE);
	statementStarters.insert(TokenType::WRITELN);

	// Tokens that can immediately follow a statement.
	statementFollowers.insert(SEMICOLON);
	statementFollowers.insert(END);
	statementFollowers.insert(UNTIL);
	statementFollowers.insert(END_OF_FILE);

	relationalOperators.insert(EQUALS);
	relationalOperators.insert(LESS_THAN);
	relationalOperators.insert(GREATER_THAN);
	relationalOperators.insert(LESS_EQUALS);

	simpleExpressionOperators.insert(PLUS);
	simpleExpressionOperators.insert(MINUS);

	termOperators.insert(STAR);
	termOperators.insert(SLASH);

	factorOperators.insert(TokenType::NOT);
	factorOperators.insert(TokenType::IF);
}

Node *Parser::parseProgram() {
	Node *programNode = new Node(NodeType::PROGRAM);

	currentToken = scanner->nextToken();  // first token!

	if (currentToken->type == TokenType::PROGRAM) {
		currentToken = scanner->nextToken();  // consume PROGRAM
	} else
		syntaxError("Expecting PROGRAM");

	if (currentToken->type == IDENTIFIER) {
		string programName = currentToken->text;
		symtab->enter(programName);
		programNode->text = programName;

		currentToken = scanner->nextToken();  // consume program name
	} else
		syntaxError("Expecting program name");

	if (currentToken->type == SEMICOLON) {
		currentToken = scanner->nextToken();  // consume ;
	} else
		syntaxError("Missing ;");

	if (currentToken->type != BEGIN)
		syntaxError("Expecting BEGIN");

	// The PROGRAM node adopts the COMPOUND tree.
	programNode->adopt(parseCompoundStatement());

	if (currentToken->type == SEMICOLON)
		syntaxError("Expecting .");
	return programNode;
}

Node *Parser::parseStatement() {
	Node *stmtNode = nullptr;
	int savedLineNumber = currentToken->lineNumber;
	lineNumber = savedLineNumber;

	switch (currentToken->type) {
	case IDENTIFIER:
		stmtNode = parseAssignmentStatement();
		break;
	case BEGIN:
		stmtNode = parseCompoundStatement();
		break;
	case REPEAT:
		stmtNode = parseRepeatStatement();
		break;
	case WHILE:
		stmtNode = parseWhileStatement();
		break;
	case IF:
		stmtNode = parseIfStatement();
		break;
	case WRITE:
		stmtNode = parseWriteStatement();
		break;
	case WRITELN:
		stmtNode = parseWritelnStatement();
		break;
	case SEMICOLON:
		stmtNode = nullptr;
		break;  // empty statement

	default:
		syntaxError("Unexpected token");
	}

	if (stmtNode != nullptr)
		stmtNode->lineNumber = savedLineNumber;
	return stmtNode;
}

Node *Parser::parseAssignmentStatement() {
	// The current token should now be the left-hand-side variable name.

	Node *assignmentNode = new Node(ASSIGN);

	// Enter the variable name into the symbol table
	// if it isn't already in there.
	string variableName = currentToken->text;
	SymtabEntry *variableId = symtab->lookup(toLowerCase(variableName));
	if (variableId == nullptr)
		variableId = symtab->enter(variableName);

	// The assignment node adopts the variable node as its first child.
	Node *lhsNode = new Node(VARIABLE);
	lhsNode->text = variableName;
	lhsNode->entry = variableId;
	assignmentNode->adopt(lhsNode);

	currentToken = scanner->nextToken();  // consume the LHS variable;

	if (currentToken->type == COLON_EQUALS) {
		currentToken = scanner->nextToken();  // consume :=
	} else
		syntaxError("Missing :=");

	// The assignment node adopts the expression node as its second child.
	Node *rhsNode = parseExpression();
	assignmentNode->adopt(rhsNode);

	return assignmentNode;
}

Node *Parser::parseCompoundStatement() {
	Node *compoundNode = new Node(COMPOUND);
	compoundNode->lineNumber = currentToken->lineNumber;

	currentToken = scanner->nextToken();  // consume BEGIN
	parseStatementList(compoundNode, END);

	if (currentToken->type == END) {
		currentToken = scanner->nextToken();  // consume END
	} else
		syntaxError("Expecting END");

	return compoundNode;
}

void Parser::parseStatementList(Node *parentNode, TokenType terminalType) {
	while ((currentToken->type != terminalType)
			&& (currentToken->type != END_OF_FILE)) {
		Node *stmtNode = parseStatement();
		if (stmtNode != nullptr)
			parentNode->adopt(stmtNode);

		// A semicolon separates statements.
		if (currentToken->type == SEMICOLON) {
			while (currentToken->type == SEMICOLON) {
				currentToken = scanner->nextToken();  // consume ;
			}
		} else if (statementStarters.find(currentToken->type)
				!= statementStarters.end()) {
			syntaxError("Missing ;");
		}
	}
}

Node *Parser::parseRepeatStatement() {
	// The current token should now be REPEAT.

	// Create a LOOP node.
	Node *loopNode = new Node(LOOP);
	currentToken = scanner->nextToken();  // consume REPEAT

	parseStatementList(loopNode, UNTIL);

	if (currentToken->type == UNTIL) {
		// Create a TEST node. It adopts the test expression node.
		Node *testNode = new Node(TEST);
		lineNumber = currentToken->lineNumber;
		testNode->lineNumber = lineNumber;
		currentToken = scanner->nextToken();  // consume UNTIL

		testNode->adopt(parseExpression());

		// The LOOP node adopts the TEST node as its final child.
		loopNode->adopt(testNode);
	} else
		syntaxError("Expecting UNTIL");

	return loopNode;
}

Node *Parser::parseWhileStatement() {
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
	} else
		syntaxError("Expecting DO");

	return loopNode;
}

Node *Parser::parseForStatement() {

	Node *compoundNode = new Node(COMPOUND);

	Node *newValue = new Node(INTEGER_CONSTANT);

	SymtabEntry *a;
	Node *oldVariable = new Node(VARIABLE);
	if (currentToken->type == FOR) {
		currentToken = scanner->nextToken(); // Consume FOR

		//Since For statement needs to keep track of previous variables, store them
		oldVariable->value = currentToken->value;
		string variablename = toLowerCase(currentToken->text);
		oldVariable->text = currentToken->text;

		compoundNode->adopt(parseAssignmentStatement());
		a = symtab->lookup(variablename);
		oldVariable->entry = a;
	} else
		syntaxError("Expecting FOR");
	if (currentToken->type == TO || currentToken->type == DOWNTO) {
		Node *gtltNode; //create node for greater than/less than
		if (currentToken->type == TO) { //check if it is TO
			gtltNode = new Node(GT);
		} else {
			gtltNode = new Node(LT);
		}

		Node *addNode; //handle cases for addition or subtraction
		if (currentToken->type == TO) { //check if it is TO
			addNode = new Node(ADD);
		} else {
			addNode = new Node(SUBTRACT);
		}

		currentToken = scanner->nextToken(); //Consumes TO/DOWNTO

		newValue->value = currentToken->value;
		currentToken = scanner->nextToken();

		gtltNode->adopt(oldVariable); //create assignment
		gtltNode->adopt(newValue);

		Node *testNode = new Node(TEST); //create a test node
		testNode->adopt(gtltNode);

		Node *loopNode = new Node(LOOP); //create loop in parse tree
		loopNode->adopt(testNode); //adopt the left hand side of tree (test)

		currentToken = scanner->nextToken();

		loopNode->adopt(parseStatement()); //adopt middle part of tree (DO statement)

		addNode->adopt(oldVariable);

		int constant = 1;
		Node *Constant = new Node(INTEGER_CONSTANT);
		Constant->value.D = constant;
		addNode->adopt(Constant);

		Node *assignNode = new Node(ASSIGN);
		compoundNode->adopt(loopNode); //adopt the loop to right side
		assignNode->adopt(oldVariable);
		loopNode->adopt(assignNode);
		assignNode->adopt(addNode);
	} else
		syntaxError("Expecting TO/DOWNTO");

	return compoundNode;
}

Node *Parser::parseIfStatement() {
	// The current token should now be IF.
	Node *ifNode = new Node(LOOP);
	currentToken = scanner->nextToken();  // consume IF

	//Parse the expression.
	// The IF node adopts the expression subtree as its first child.
	ifNode->adopt(parseExpression());

	//Find the THEN
	if (currentToken->type == THEN) {
		currentToken = scanner->nextToken(); //consume THEN
	} else
		syntaxError("Expecting THEN");

	printf("finish THEN now: '%s' \n", currentToken->text.c_str());

	//parse the THEN statement
	//IF node adopts the statement as the second child
	ifNode->adopt(parseExpression());
	currentToken = scanner->nextToken();  // consume
	printf("finish adding child now: '%s' \n", currentToken->text.c_str());

	//Look for else
	if (currentToken->type == ELSE) {
		currentToken = scanner->nextToken(); //consume ELSE

		//Parse the else Statement
		//IF node adopts the statement as the third child
		ifNode->adopt(parseExpression());
	}

	return ifNode;
}

Node *Parser::parseCaseStatement() {
	// The current token should now be CASE

	// Create LOOP, TEST, and NOT nodes.
//    Node *not_node = new Node (NOT);
	Node *case_node = new Node(LOOP);
	lineNumber = currentToken->lineNumber;
	case_node->lineNumber = lineNumber;

	currentToken = scanner->nextToken();  // consume the CASE

	// The TEST node adopts the expression of case as its first child
	case_node->adopt(parseExpression());

	//Find the Case option
	if (currentToken->type == OF) {
		currentToken = scanner->nextToken();  // consume the OF
	} else {
		syntaxError("Expecting OF");
	}
	//starts looping through each case line until case is finished
	while (currentToken->type != END) {
		//check to make sure it starts with a case label
		if (currentToken->type == COLON) {
			//missing case label error
			syntaxError("Expecting case before :");
		} else if (currentToken->type != SEMICOLON) {
			//create a child with the case label
			case_node->adopt(parseExpression());
		} else {
			syntaxError("missing case expression");
		}
		//Find the :
		if (currentToken->type == COLON) {
			currentToken = scanner->nextToken();   // consume the :
		} else {
			//missing : after case label
			//TODO
			syntaxError("Expecting :");
		}
		if (currentToken->type != SEMICOLON) {
			//create a child with the case expression
			case_node->adopt(parseExpression());
		} else {
			syntaxError("Missing case expression");
		}
		if (currentToken->type == SEMICOLON) {
			currentToken = scanner->nextToken();  // consume the ;
		} else {
			syntaxError("Expected ;");
		}
	}

	return case_node;
}

Node *Parser::parseWriteStatement() {
// The current token should now be WRITE.

// Create a WRITE node-> It adopts the variable or string node.
	Node *writeNode = new Node(NodeType::WRITE);
	currentToken = scanner->nextToken();  // consume WRITE

	parseWriteArguments(writeNode);
	if (writeNode->children.size() == 0) {
		syntaxError("Invalid WRITE statement");
	}

	return writeNode;
}

Node *Parser::parseWritelnStatement() {
// The current token should now be WRITELN.

// Create a WRITELN node. It adopts the variable or string node.
	Node *writelnNode = new Node(NodeType::WRITELN);
	currentToken = scanner->nextToken();  // consume WRITELN

	if (currentToken->type == LPAREN)
		parseWriteArguments(writelnNode);
	return writelnNode;
}

void Parser::parseWriteArguments(Node *node) {
// The current token should now be (

	bool hasArgument = false;

	if (currentToken->type == LPAREN) {
		currentToken = scanner->nextToken();  // consume (
	} else
		syntaxError("Missing left parenthesis");

	if (currentToken->type == IDENTIFIER) {
		node->adopt(parseVariable());
		hasArgument = true;
	} else if ((currentToken->type == CHARACTER)
			|| (currentToken->type == STRING)) {
		node->adopt(parseStringConstant());
		hasArgument = true;
	} else
		syntaxError("Invalid WRITE or WRITELN statement");

// Look for a field width and a count of decimal places.
	if (hasArgument) {
		if (currentToken->type == COLON) {
			currentToken = scanner->nextToken();  // consume ,

			if (currentToken->type == INTEGER) {
				// Field width
				node->adopt(parseIntegerConstant());

				if (currentToken->type == COLON) {
					currentToken = scanner->nextToken();  // consume ,

					if (currentToken->type == INTEGER) {
						// Count of decimal places
						node->adopt(parseIntegerConstant());
					} else
						syntaxError("Invalid count of decimal places");
				}
			} else
				syntaxError("Invalid field width");
		}
	}

	if (currentToken->type == RPAREN) {
		currentToken = scanner->nextToken();  // consume )
	} else
		syntaxError("Missing right parenthesis");
}

Node *Parser::parseExpression() {
// The current token should now be an identifier or a number.

// The expression's root node->
	Node *exprNode = parseSimpleExpression();

// The current token might now be a relational operator.
	if (relationalOperators.find(currentToken->type)
			!= relationalOperators.end()) {
		TokenType tokenType = currentToken->type;
		Node *opNode = tokenType == EQUALS ? new Node(EQ) :
						tokenType == LESS_THAN ? new Node(LT) : nullptr;

		currentToken = scanner->nextToken();  // consume relational operator

		// The relational operator node adopts the first simple expression
		// node as its first child and the second simple expression node
		// as its second child. Then it becomes the expression's root node.
		if (opNode != nullptr) {
			opNode->adopt(exprNode);
			opNode->adopt(parseSimpleExpression());
			exprNode = opNode;
		}
	}

	return exprNode;
}

Node *Parser::parseSimpleExpression() {
// The current token should now be an identifier or a number.

// The simple expression's root node->
	Node *simpExprNode = parseTerm();

// Keep parsing more terms as long as the current token
// is a + or - operator.
	while (simpleExpressionOperators.find(currentToken->type)
			!= simpleExpressionOperators.end()) {
		Node *opNode =
				currentToken->type == PLUS ? new Node(ADD) : new Node(SUBTRACT);

		currentToken = scanner->nextToken();  // consume the operator

		// The add or subtract node adopts the first term node as its
		// first child and the next term node as its second child.
		// Then it becomes the simple expression's root node.
		opNode->adopt(simpExprNode);
		opNode->adopt(parseTerm());
		simpExprNode = opNode;
	}

	return simpExprNode;
}

Node *Parser::parseTerm() {
// The current token should now be an identifier or a number.

// The term's root node->
	Node *termNode = parseFactor();

// Keep parsing more factors as long as the current token
// is a * or / operator.
	while (termOperators.find(currentToken->type) != termOperators.end()) {
		Node *opNode =
				currentToken->type == STAR ?
						new Node(MULTIPLY) : new Node(DIVIDE);

		currentToken = scanner->nextToken();  // consume the operator

		// The multiply or divide node adopts the first factor node as its
		// as its first child and the next factor node as its second child.
		// Then it becomes the term's root node.
		opNode->adopt(termNode);
		opNode->adopt(parseFactor());
		termNode = opNode;
	}

	return termNode;
}

Node *Parser::parseFactor() {
// The current token should now be an identifier or a number or (

	if (currentToken->type == IDENTIFIER)
		return parseVariable();
	else if (currentToken->type == INTEGER)
		return parseIntegerConstant();
	else if (currentToken->type == REAL)
		return parseRealConstant();

	else if (currentToken->type == LPAREN) {
		currentToken = scanner->nextToken();  // consume (
		Node *exprNode = parseExpression();

		if (currentToken->type == RPAREN) {
			currentToken = scanner->nextToken();  // consume )
		} else
			syntaxError("Expecting )");

		return exprNode;
	}

	else
		syntaxError("Unexpected token");
	return nullptr;
}

Node *Parser::parseVariable() {
// The current token should now be an identifier.

// Has the variable been "declared"?
	string variableName = currentToken->text;
	SymtabEntry *variableId = symtab->lookup(toLowerCase(variableName));
	if (variableId == nullptr)
		semanticError("Undeclared identifier");

	Node *node = new Node(VARIABLE);
	node->text = variableName;
	node->entry = variableId;

	currentToken = scanner->nextToken();  // consume the identifier
	return node;
}

Node *Parser::parseIntegerConstant() {
// The current token should now be a number.

	Node *integerNode = new Node(INTEGER_CONSTANT);
	integerNode->value = currentToken->value;

	currentToken = scanner->nextToken();  // consume the number
	return integerNode;
}

Node *Parser::parseRealConstant() {
// The current token should now be a number.

	Node *realNode = new Node(REAL_CONSTANT);
	realNode->value = currentToken->value;

	currentToken = scanner->nextToken();  // consume the number
	return realNode;
}

Node *Parser::parseStringConstant() {
// The current token should now be string.

	Node *stringNode = new Node(STRING_CONSTANT);
	stringNode->value = currentToken->value;

	currentToken = scanner->nextToken();  // consume the string
	return stringNode;
}

void Parser::syntaxError(string message) {
	printf("SYNTAX ERROR at line %d: %s at '%s'\n", lineNumber, message.c_str(),
			currentToken->text.c_str());
	errorCount++;

//ADDED IN - needed line 517  in order for it not to be a infinite loop
	currentToken = scanner->nextToken();
// Recover by skipping the rest of the statement.
// Skip to a statement follower token.
//    printf("recovery attempt \n");
	while (statementFollowers.find(currentToken->type)
			== statementFollowers.end()) {
		currentToken = scanner->nextToken();
	}
}

void Parser::semanticError(string message) {
	printf("SEMANTIC ERROR at line %d: %s at '%s'\n", lineNumber,
			message.c_str(), currentToken->text.c_str());
	errorCount++;
}

}  // namespace frontend

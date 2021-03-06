#include <iostream>

#include "Object.h"
#include "Executor.h"
#include "Symtab.h"
#include "SymtabEntry.h"

namespace backend { namespace interpreter {

using namespace std;
using namespace intermediate::symtab;

Object Executor::visitProgram(Pcl4Parser::ProgramContext *ctx)
{
    cout << "Visiting program" << endl;

    return visit(ctx->block());
}

Object Executor::visitStatement(Pcl4Parser::StatementContext *ctx)
{
    cout << "Line " << ctx->getStart()->getLine() << ": ";
    return visitChildren(ctx);
}

Object Executor::visitStatementList(Pcl4Parser::StatementListContext *ctx)
{
    cout << "Visiting statement list" << endl;

    for (Pcl4Parser::StatementContext *stmtCtx : ctx->statement())
    {
        visit(stmtCtx);
    }

    return nullptr;
}

Object Executor::visitCompoundStatement(Pcl4Parser::CompoundStatementContext *ctx)
{
    cout << "Visiting compound statement" << endl;
    return visit(ctx->statementList());
}

Object Executor::visitAssignmentStatement(Pcl4Parser::AssignmentStatementContext *ctx)
{
    cout << "Visiting assignment statement" << endl;
    string variableName = ctx->lhs()->variable()->getText();
    visit(ctx->lhs());
    Object value = visit(ctx->rhs());

    cout << "Will assign value " << value.as<int>()
         << " to variable " << variableName << endl;
    return nullptr;
}

Object Executor::visitRepeatStatement(Pcl4Parser::RepeatStatementContext *ctx)
{
    /**cout << "Visiting REPEAT statement" << endl;
    return visit(ctx->statementList());
    repeatStatement     : REPEAT statementList UNTIL expression ;
    */
    Pcl4Parser::StatementListContext *listCtx = ctx->statementList();
    bool value = false;

    do {
        visit(listCtx);
        string exprValue = visit(ctx->expression()).as<string>();
        value = (exprValue=="T");
    } while(!value);

    return nullptr;

}

Object Executor::visitForStatement(Pcl4Parser::ForStatementContext *ctx) {
	//forStatement        : FOR variable ':=' expression (TO|DOWNTO) expression DO statement;

	cout << "Visiting for statement" << endl;

    //Pcl4Parser::VariableContext *listCtx = ctx->variable(); //i
	Pcl4Parser::ExpressionContext *ctx1 = ctx->expression(0);

    int start = stoi(ctx1->getText());
    cout << "Will assign value " << start;


    Pcl4Parser::ExpressionContext *simpleCtx = ctx->expression(0); //= 5?
    int end = stoi(simpleCtx->getText());
    cout << "Will assign value " << end;

    if(ctx->TO()->getText() == "TO")
    {
    	for(int i = start; i < end; i++)
    	{
    		Pcl4Parser::StatementContext *stmtCtx = ctx->statement();
    		visit(stmtCtx);
    	}
    }
    else if(ctx->DOWNTO()->getText()=="DOWNTO")
    {
    	for(int i = start; i < end; i--)
    	{
    	    Pcl4Parser::StatementContext *stmtCtx = ctx->statement();
    	    visit(stmtCtx);
    	}
    }

    return nullptr;

}

Object Executor::visitCaseStatement(Pcl4Parser::CaseStatementContext *ctx)
{
	cout << "Visiting case statement" << endl;

    bool match = false;
    long caseValue = (long) stod(visit(ctx->expression()).as<string>());

    Pcl4Parser::CaseBranchListContext *branchlistCtx = ctx->caseBranchList();
    for (Pcl4Parser::CaseBranchContext *branchCtx : branchlistCtx->caseBranch())
    {
        Pcl4Parser::CaseConstantListContext *constListCtx = branchCtx->caseConstantList();

        for (Pcl4Parser::CaseConstantContext *caseConstCtx : constListCtx->caseConstant())
        {
            bool negate = (caseConstCtx->sign() != nullptr) && (caseConstCtx->sign()->getText() == "-");
            long constValue = (long) stod(visit(caseConstCtx->unsignedNumber()).as<string>());
            if (negate)
            {
                constValue = -constValue;
            }

            if (caseValue == constValue)
            {
                visit(branchCtx->statement());
                match = true;
                break;
            }
        }

        if (match)
        {
            break;
        }
    }
    return nullptr;
}



Object Executor::visitWhileStatement(Pcl4Parser::WhileStatementContext *ctx) {

	cout << "Visiting while statement" << endl;

    Pcl4Parser::StatementContext *stmtCtx = ctx->statement();
    bool value = visit(ctx->expression()).as<string>() == "T";
    while (value) {
        visit(stmtCtx);
        value = visit(ctx->expression()).as<string>() == "T";
    }
    return nullptr;
}


Object Executor::visitWriteStatement(Pcl4Parser::WriteStatementContext *ctx)
{
    //cout << "Visiting WRITE statement" << endl;
    visitChildren(ctx);
    return nullptr;
}

Object Executor::visitWritelnStatement(Pcl4Parser::WritelnStatementContext *ctx)
{
    //cout << "Visiting WRITELN statement" << endl;
    visitChildren(ctx);
    cout << endl;
    return nullptr;
}

Object Executor::visitExpression(Pcl4Parser::ExpressionContext *ctx)
{
	cout << "Visiting expression statement" << endl;

    Pcl4Parser::SimpleExpressionContext *simpleCtx1 = ctx->simpleExpression(0);
    Pcl4Parser::RelOpContext *relOpCtx = ctx->relOp();
    string operand1 = visit(simpleCtx1).as<string>();

    if (relOpCtx != nullptr) {
        string op = relOpCtx->getText();
        Pcl4Parser::SimpleExpressionContext *simpleCtx2 = ctx->simpleExpression(1);
        string operand2 = visit(simpleCtx2).as<string>();

        double val1 = stod(operand1);
        double val2 = stod(operand2);
        //Expressions have relational operators, so the result will be a bool
        bool result=false;

        if(op == "=")
            result = (val1 == val2);
        else if (op == "<>")
            result = (val1 != val2);
        else if (op == "<")
            result = (val1 < val2);
        else if (op == ">")
            result = (val1 > val2);
        else if (op == "<=")
            result = (val1 <= val2);
        else if (op == ">=")
            result = val1 >= val2;
        //no default case

        if(result)
            return string("T");
        return string("F");
    }

    return operand1; //occurs if the expr was just a simple expr
}


Object Executor::visitIfStatement(Pcl4Parser::IfStatementContext *ctx)
{
	//ifStatement: IF expression THEN true(ELSE false)? ;

	cout << "Visiting if statement" << endl;


	Pcl4Parser::ExpressionContext *ctx1 = ctx->expression();
	Pcl4Parser::TruestatementContext *truectx = ctx->truestatement();
	Pcl4Parser::FalsestatementContext *falsectx = ctx->falsestatement();

	visit(ctx1);

	visit(truectx);

	if(falsectx != nullptr)
		visit(falsectx);

	return nullptr;

}


Object Executor::visitSimpleExpression(Pcl4Parser::SimpleExpressionContext *ctx) {

	cout << "Visiting simple expression statement" << endl;

    //number of terms
    int count = ctx->term().size();

    //no sign or not a - sign is taken as positive
    bool negative = (ctx->sign() != nullptr) && (ctx->sign()->getText()=="-");

    Pcl4Parser::TermContext *termCtx1 = ctx->term(0);
    string operand1 = visit(termCtx1).as<string>();
    if(negative) {
        operand1 = "-" + operand1;
    }

    for(int i=1; i<count; i++) {
        string op = toLowerCase(ctx->addOp(i-1)->getText());
        Pcl4Parser::TermContext *termCtx2 = ctx->term(i);

        string operand2 = visit(termCtx2).as<string>();
        //remember that possible operators are only OR, +, -
        if( (operand2=="T")||(operand2=="F") ) { //opd2 is a bool
            bool value = (operand1=="T") || (operand2=="T"); //if either opd is true
            operand1 = string(value? "T" : "F");//then opd1 is T (else F)
        }
        else {
            double value1 = stod(operand1);
            double value2 = stod(operand2);

            if(negative) {
                value1 *= -1;
                negative = false;
            }

            if(op=="+"){
                operand1 = to_string(value1+value2);
            }
            else {
                operand1 = to_string(value1-value2);
            }
        }
    }
    return operand1;
}


Object Executor::visitTerm(Pcl4Parser::TermContext *ctx) {
    //number of factors
    int count = ctx->factor().size();

    Pcl4Parser::FactorContext *factorCtx1 = ctx->factor(0);
    string operand1 = visit(factorCtx1).as<string>();

    for(int i=1; i<count; i++) {
        string op = toLowerCase(ctx->mulOp(i-1)->getText());
        Pcl4Parser::FactorContext *factorCtx2 = ctx->factor(i);
        string operand2 = visit(factorCtx2).as<string>();

        //remember that possible operators are only AND, *, / DIV, MOD
        if( (operand2=="T")||(operand2=="F") ) { //if opd2 is a bool
            if( (operand1=="T") && (operand2=="T") ) //AND op1 and op2
                operand1 = string("T");
            else
                operand1 = string("F");
        }
        else {
            double val1 = stod(operand1);
            double val2 = stod(operand2);

            if(op=="*") {
                operand1 = to_string( val1*val2 );
            }
            else if(op=="/") { //check /, DIV, MOD operations
                operand1 = to_string( val1/val2 );
            }
            else if(op=="div") {
                operand1 = to_string( trunc(val1/val2) );
            }
            else if(op=="mod") {
                operand1 = to_string (int(val1) % int(val2));
            }
        }
    }
    return operand1;
}



Object Executor::visitVariable(Pcl4Parser::VariableContext *ctx)
{
    cout << "Visiting variable ";
        string variableName = ctx->getText();
        cout << variableName << endl;

    Object value = symtab[variableName];

    return value;  // should return the variable's value!
}



Object Executor::visitNumber(Pcl4Parser::NumberContext *ctx)
{
    /**cout << "Visiting number: got value ";
    string text = ctx->unsignedNumber()->integerConstant()
                                       ->INTEGER()->getText();
    int value = stoi(text);
    cout << value << endl;
    return value;
    */
    return visit(ctx->unsignedNumber());
}

Object Executor::visitUnsignedNumber(Pcl4Parser::UnsignedNumberContext *ctx) {
    if(ctx->integerConstant() != nullptr)
        return visit(ctx->integerConstant());
    return visit(ctx->realConstant());
}

Object Executor::visitIntegerConstant(Pcl4Parser::IntegerConstantContext *ctx) {
    return ctx->getText();
}

Object Executor::visitRealConstant(Pcl4Parser::RealConstantContext *ctx) {
    return ctx->getText();
}

Object Executor::visitCharacterConstant(Pcl4Parser::CharacterConstantContext *ctx) {
    string pascalStr = ctx->CHARACTER()->getText();
    return convertString(pascalStr, false);
}

Object Executor::visitStringConstant(Pcl4Parser::StringConstantContext *ctx) {
    string pascalStr = ctx->STRING()->getText();
    return convertString(pascalStr, false);
}
// Complete this class!

}}  // namespace backend::interpreter

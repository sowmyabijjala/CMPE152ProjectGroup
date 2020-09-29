#include <iostream>

#include "Object.h"
#include "Executor.h"

namespace backend { namespace interpreter {

using namespace std;

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
    cout << "Visiting REPEAT statement" << endl;
    return visit(ctx->statementList());
}

Object Executor::visitWriteStatement(Pcl4Parser::WriteStatementContext *ctx)
{
    cout << "Visiting WRITE statement" << endl;
    return nullptr;
}

Object Executor::visitWritelnStatement(Pcl4Parser::WritelnStatementContext *ctx)
{
    cout << "Visiting WRITELN statement" << endl;
    return nullptr;
}

Object Executor::visitExpression(Pcl4Parser::ExpressionContext *ctx)
{
    cout << "Visiting expression" << endl;
    return visitChildren(ctx);
}

Object Executor::visitVariable(Pcl4Parser::VariableContext *ctx)
{
    cout << "Visiting variable ";
    string variableName = ctx->getText();
    cout << variableName << endl;

    return nullptr;  // should return the variable's value!
}

Object Executor::visitNumber(Pcl4Parser::NumberContext *ctx)
{
    cout << "Visiting number: got value ";
    string text = ctx->unsignedNumber()->integerConstant()
                                       ->INTEGER()->getText();
    int value = stoi(text);
    cout << value << endl;

    return value;
}

// Complete this class!

}}  // namespace backend::interpreter

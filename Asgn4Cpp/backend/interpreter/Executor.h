#ifndef EXECUTOR_H_
#define EXECUTOR_H_

#include <string>
#include <map>

#include "antlr4-runtime.h"
#include "Pcl4BaseVisitor.h"
#include "Symtab.h"
#include "SymtabEntry.h"

#include "Object.h"

namespace backend { namespace interpreter {

using namespace std;

typedef antlrcpp::Any Object;



class Executor : public Pcl4BaseVisitor
{
public:
    Executor() {}
    virtual ~Executor() {}

    Object visitProgram(Pcl4Parser::ProgramContext *ctx) override;
    Object visitStatement(Pcl4Parser::StatementContext *ctx) override;
    Object visitStatementList(Pcl4Parser::StatementListContext *ctx) override;
    Object visitCompoundStatement(Pcl4Parser::CompoundStatementContext *ctx) override;
    Object visitAssignmentStatement(Pcl4Parser::AssignmentStatementContext *ctx) override;
    Object visitCaseStatement(Pcl4Parser::CaseStatementContext *ctx) override;
    Object visitWhileStatement(Pcl4Parser::WhileStatementContext *ctx) override;
    Object visitRepeatStatement(Pcl4Parser::RepeatStatementContext *ctx) override;
    Object visitForStatement(Pcl4Parser::ForStatementContext *ctx) override;
    Object visitIfStatement(Pcl4Parser::IfStatementContext *ctx) override;
    Object visitSimpleExpression(Pcl4Parser::SimpleExpressionContext *ctx) override;
    //Object visitTerm(Pcl4Parser::TermContext *ctx) override;
    Object visitIntegerConstant(Pcl4Parser::IntegerConstantContext *ctx) override;
    Object visitRealConstant(Pcl4Parser::RealConstantContext *ctx) override;
    Object visitCharacterConstant(Pcl4Parser::CharacterConstantContext *ctx);
    Object visitWriteStatement(Pcl4Parser::WriteStatementContext *ctx) override;
    Object visitUnsignedNumber(Pcl4Parser::UnsignedNumberContext *ctx) override;
    Object visitStringConstant(Pcl4Parser::StringConstantContext *ctx) override;
    Object visitWritelnStatement(Pcl4Parser::WritelnStatementContext *ctx) override;
    Object visitExpression(Pcl4Parser::ExpressionContext *ctx) override;
    Object visitVariable(Pcl4Parser::VariableContext *ctx) override;
    Object visitNumber(Pcl4Parser::NumberContext *ctx) override;
   // Object visitParenthesizedExpression(Pcl4Parser::ParenthesizedExpressionContext *ctx) override;

    // Complete this class!

private:
    map<string,Object> symtab;
};

}}  // namespace backend::interpreter

#endif /* EXECUTOR_H_ */

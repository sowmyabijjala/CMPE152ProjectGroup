#ifndef CONVERTER_H_
#define CONVERTER_H_

#include <map>
#include <vector>

#include "PascalBaseVisitor.h"
#include "antlr4-runtime.h"

#include "intermediate/symtab/SymtabStack.h"
#include "intermediate/symtab/SymtabEntry.h"
#include "intermediate/type/Typespec.h"
#include "CodeGenerator.h"

namespace backend { namespace converter {

using namespace std;
using namespace intermediate::symtab;
using namespace intermediate::type;

class Converter : public PascalBaseVisitor
{
private:
    CodeGenerator code;
    bool programVariables;
    bool recordFields;
    string currentSeparator;

public:
    Converter()
        : programVariables(true), recordFields(false),
          currentSeparator("")
    {
        typeNameTable["integer"] = "int";
        typeNameTable["real"]    = "double";
        typeNameTable["boolean"] = "bool";
        typeNameTable["char"]    = "char";
        typeNameTable["string"]  = "string";
    }

    /**
     * Get the name of the object (Java) file.
     * @return the name.
     */
    string getObjectFileName() const { return code.getObjectFileName(); }

    Object visitProgram(PascalParser::ProgramContext *ctx) override;
    Object visitProgramHeader(PascalParser::ProgramHeaderContext *ctx) override;
    Object visitConstantDefinition(PascalParser::ConstantDefinitionContext *ctx) override;
    Object visitTypeDefinition(PascalParser::TypeDefinitionContext *ctx) override;
    Object visitEnumerationTypespec(PascalParser::EnumerationTypespecContext *ctx) override;
    Object visitRecordTypespec(PascalParser::RecordTypespecContext *ctx) override;
    Object visitVariableDeclarations(PascalParser::VariableDeclarationsContext *ctx) override;
    Object visitTypeIdentifier(PascalParser::TypeIdentifierContext *ctx) override;
    Object visitVariableIdentifierList(PascalParser::VariableIdentifierListContext *ctx) override;
    Object visitRoutineDefinition(PascalParser::RoutineDefinitionContext *ctx) override;
    Object visitParameters(PascalParser::ParametersContext *ctx) override;
    Object visitParameterDeclarations(PascalParser::ParameterDeclarationsContext *ctx) override;
    Object visitStatementList(PascalParser::StatementListContext *ctx) override;
    Object visitCompoundStatement(PascalParser::CompoundStatementContext *ctx) override;
    Object visitAssignmentStatement(PascalParser::AssignmentStatementContext *ctx) override;
    Object visitRepeatStatement(PascalParser::RepeatStatementContext *ctx) override;
    Object visitWhileStatement(PascalParser::WhileStatementContext *ctx) override; //
    Object visitForStatement(PascalParser::ForStatementContext *ctx) override; //
    Object visitExpression(PascalParser::ExpressionContext *ctx) override;
    Object visitSimpleExpression(PascalParser::SimpleExpressionContext *ctx) override;
    Object visitTerm(PascalParser::TermContext *ctx) override;
    Object visitVariableFactor(PascalParser::VariableFactorContext *ctx) override;
    Object visitVariable(PascalParser::VariableContext *ctx) override;
    Object visitNumberFactor(PascalParser::NumberFactorContext *ctx) override;
    Object visitCharacterFactor(PascalParser::CharacterFactorContext *ctx) override;
    Object visitStringFactor(PascalParser::StringFactorContext *ctx) override;
    Object visitNotFactor(PascalParser::NotFactorContext *ctx) override;
    Object visitParenthesizedFactor(PascalParser::ParenthesizedFactorContext *ctx) override;
    Object visitWriteStatement(PascalParser::WriteStatementContext *ctx) override;
    Object visitWritelnStatement(PascalParser::WritelnStatementContext *ctx) override;
    Object visitReadStatement(PascalParser::ReadStatementContext *ctx) override;
    Object visitReadlnStatement(PascalParser::ReadlnStatementContext *ctx) override;
    Object visitReadArguments(PascalParser::ReadArgumentsContext *ctx) override;

private:
    // Map a Pascal datatype name to the C++ datatype name.
    map<string, string> typeNameTable;

    /**
     * Map a Pascal type name to an equivalent C++ type name.
     */
    string mapTypeName(string pascalTypeName);

    /**
     * Convert a Pascal type name to the equivalent C++ type.
     * @param pascalType the datatype name.
     * @return the C++ type name.
     */
    string typeName(Typespec*pascalType);

    /**
     * Emit a variable declaration with allocation for an array or record.
     * @param type the datatype of the variable.
     * @param varId the symbol table entry of the variable.
     */
    void emitVariableDeclaration(Typespec *type, SymtabEntry *varId);

    /**
     * Emit code to allocate data for structured (array or record) variables.
     * @param lhsPrefix the prefix for the target variable name.
     * @param symtab the symbol table containing the variable names.
     */
    void emitAllocateStructuredVariables(string lhsPrefix, Symtab *symtab);

    /**
     * Emit code to allocate structured (array or record) data.
     * @param lhsPrefix the prefix for the target variable name.
     * @param variableId the symbol table entry of the target variable.
     */
    void emitAllocateStructuredData(string lhsPrefix, SymtabEntry *variableId);

    /**
     * Emit a string of bracketed dimension sizes for the array datatype.
     * @param type the array datatype.
     */
    void emitArrayDimensions(Typespec *type);

    /**
     * Emit code to allocate an array element.
     * @param lhsPrefix the prefix for the target variable name.
     * @param variableName the name of the target variable.
     * @param elmtType the element's datatype.
     */
    void emitNewArrayElement(string lhsPrefix, string variableName,
                             Typespec *elmtType);

    /**
     * Emit code to allocate a new record.
     * @param lhsPrefix the prefix for the target variable name.
     * @param variableName the name of the target variable.
     * @param recordType the record's datatype.
     */
    void emitNewRecord(string lhsPrefix, string variableName,
                       Typespec recordType);

    /**
     * Emit code to allocate a record's fields.
     * @param lhsPrefix the prefix for the target variable name.
     * @param recordType the record's datatype.
     */
    void emitNewRecordFields(string lhsPrefix, Typespec *recordType);

    /**
     * Emit a record type definition for an unnamed record.
     * @param symtab the symbol table that can contain unnamed records.
     */
    void emitUnnamedRecordDefinitions(Symtab *symtab);

    /**
     * Emit the record fields of a record.
     * @param symtab the symbol table of the unnamed record.
     */
    void emitRecordFields(Symtab *symtab);

    /**
     * Determine the datatype of a variable that can have modifiers.
     * @param varCtx the VariableContext.
     * @param varType the variable's datatype without the modifiers.
     * @return the datatype with any modifiers.
     */
    Typespec *variableDatatype(PascalParser::VariableContext *varCtx,
                               Typespec *varType);

    /**
     * Create the printf format string.
     * @param ctx the WriteArgumentsContext.
     * @return the format string.
     */
    string createWriteFormat(PascalParser::WriteArgumentsContext *ctx);

    /**
     * Create the string of write arguments.
     * @param ctx the WriteArgumentsContext.
     * @return the string of arguments.
     */
    string createWriteArguments(PascalParser::WriteArgumentsContext *ctx);
};

}} // namespace backend::converter

#endif /* CONVERTER_H_ */

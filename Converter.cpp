#include "antlr4/PascalBaseVisitor.h"
#include "antlr4-runtime.h"

#include "../../Object.h"
#include "intermediate/symtab/Predefined.h"
#include "intermediate/symtab/SymtabEntry.h"
#include "intermediate/type/Typespec.h"
#include "Converter.h"

namespace backend { namespace converter {

Object Converter::visitProgram(PascalParser::ProgramContext *ctx)
{
    visit(ctx->programHeader());

    // Level 1 declarations.
    visit(ctx->block()->declarations());

    // Main.
    code.emitLine();
    code.emitLine("int main(int argc, char *args[])");
    code.emitLine("{");
    code.indent();

    // Execution timer.
    code.emitLine("auto _start = steady_clock::now();");
    code.emitLine();

    // Main compound statement.
    visit(ctx->block()->compoundStatement()->statementList());

    // Print the execution time.
    code.emitLine();
    code.emitLine("auto _end = steady_clock::now();");
    code.emitStart("long _elapsed = duration_cast<milliseconds>");
    code.emitEnd("(_end - _start).count();");
    code.emitLine(string("printf(\"\\n[%ld milliseconds execution time.]") +
                  string("\\n\", _elapsed);"));

    code.dedent();
    code.emitLine("}");

    code.close();
    return nullptr;
}

Object Converter::visitProgramHeader(PascalParser::ProgramHeaderContext *ctx)
{
    string programName = ctx->programIdentifier()->IDENTIFIER()->getText();
    code.open(programName, "cpp");

    code.emitLine("#include <iostream>");
    code.emitLine("#include <iomanip>");
    code.emitLine("#include <chrono>");
    code.emitLine("#include <string>");
    code.emitLine();
    code.emitLine("using namespace std;");
    code.emitLine("using namespace std::chrono;");
    code.emitLine();

    return nullptr;
}

Object Converter::visitConstantDefinition(
                                PascalParser::ConstantDefinitionContext *ctx)
{
    PascalParser::ConstantIdentifierContext *idCtx = ctx->constantIdentifier();
    PascalParser::ConstantContext *constCtx = ctx->constant();
    string constantName = idCtx->entry->getName();
    Typespec *type = constCtx->type;
    string pascalTypeName = type->getIdentifier()->getName();
    string cppTypeName = typeNameTable[pascalTypeName];

    code.emitStart();
    code.emitEnd("const " + cppTypeName + " " + constantName + " = "
                          + constCtx->getText() + ";");

    return nullptr;
}

Object Converter::visitTypeDefinition(PascalParser::TypeDefinitionContext *ctx)
{
    PascalParser::TypeIdentifierContext *idCtx = ctx->typeIdentifier();
    string typeName = idCtx->entry->getName();
    PascalParser::TypeSpecificationContext *typeCtx = ctx->typeSpecification();
    Form form = typeCtx->type->getForm();

    if (form == ENUMERATION)
    {
        code.emitStart();
        code.emit("enum class " + typeName);
        visit(typeCtx);
    }
    else if (form == RECORD)
    {
        code.emitStart();
        code.emitEnd("struct " + typeName);
        code.emitLine("{");
        code.indent();

        emitUnnamedRecordDefinitions(typeCtx->type->getRecordSymtab());
        visit(typeCtx);

        code.dedent();
        code.emitLine("};");
        code.emitLine();
    }

    return nullptr;
}

Object Converter::visitEnumerationTypespec(
                                PascalParser::EnumerationTypespecContext *ctx)
{
    string separator = " {";

    for (PascalParser::EnumerationConstantContext *constCtx :
                                ctx->enumerationType()->enumerationConstant())
    {
        code.emit(separator + constCtx->constantIdentifier()->entry->getName());
        separator = ", ";
    }

    code.emitEnd("};");
    return nullptr;
}

void Converter::emitUnnamedRecordDefinitions(Symtab *symtab)
{
    // Loop to look for names of unnamed record types.
    for (SymtabEntry *id : symtab->sortedEntries())
    {
        if (   (id->getKind() == TYPE)
            && (id->getType()->getForm() == RECORD)
            && (id->getName().length() >= UNNAMED_PREFIX_LENGTH)
            && (id->getName().substr(0, UNNAMED_PREFIX_LENGTH) == UNNAMED_PREFIX))
        {
            code.emitStart();
            code.emitEnd("struct " + id->getName());
            code.emitLine("{");
            code.indent();
            emitRecordFields(id->getType()->getRecordSymtab());
            code.dedent();
            code.emitLine("};");
            code.emitLine();
        }
    }
}

/**
 * Emit the record fields of a record.
 * @param symtab the symbol table of the unnamed record.
 */
void Converter::emitRecordFields(Symtab *symtab)
{
    emitUnnamedRecordDefinitions(symtab);

    // Loop over the entries of the symbol table.
    for (SymtabEntry *fieldId : symtab->sortedEntries())
    {
        if (fieldId->getKind() == RECORD_FIELD)
        {
            code.emitStart(typeName(fieldId->getType()));
            code.emit(" " + fieldId->getName());
            code.emitEnd(";");
        }
    }
}

Object Converter::visitRecordTypespec(PascalParser::RecordTypespecContext *ctx)
{
    PascalParser::RecordFieldsContext *fieldsCtx =
                                            ctx->recordType()->recordFields();
    recordFields = true;
    visit(fieldsCtx->variableDeclarationsList());
    recordFields = false;

    return nullptr;
}

Object Converter::visitVariableDeclarations(
                                PascalParser::VariableDeclarationsContext *ctx)
{
    PascalParser::TypeSpecificationContext *typeCtx = ctx->typeSpecification();
    PascalParser::VariableIdentifierListContext *listCtx =
                                                ctx->variableIdentifierList();

    for (PascalParser::VariableIdentifierContext *varCtx :
                                                listCtx->variableIdentifier())
    {
        code.emitStart();
        code.emit(typeName(typeCtx->type));

        code.emit(" " + varCtx->entry->getName());
        if (typeCtx->type->getForm() == ARRAY) emitArrayDimensions(typeCtx->type);
        code.emitEnd(";");
    }

    return nullptr;
}

void Converter::emitArrayDimensions(Typespec *type)
{
    string sizes = "";

    while (type->getForm() == ARRAY)
    {
        sizes += "[" + to_string(type->getArrayElementCount()) + "]";
        type = type->getArrayElementType();
    }

    code.emit(sizes);
}

string Converter::typeName(Typespec *pascalType)
{
    Form form = pascalType->getForm();
    SymtabEntry *typeId = pascalType->getIdentifier();
    string pascalTypeName = typeId != nullptr ? typeId->getName() : "";

    if (form == ARRAY)
    {
        Typespec *elmtType = pascalType->getArrayBaseType();
        pascalTypeName = elmtType->getIdentifier()->getName();
        return mapTypeName(pascalTypeName);
    }
    else if (form == SUBRANGE)
    {
        Typespec *baseType = pascalType->baseType();
        pascalTypeName = baseType->getIdentifier()->getName();
        return mapTypeName(pascalTypeName);
    }
    else if (form == ENUMERATION)
    {
        return pascalType == Predefined::booleanType ? "bool" : pascalTypeName;
    }
    else if (form == RECORD) return pascalTypeName;
    else                     return mapTypeName(pascalTypeName);
}

string Converter::mapTypeName(string pascalTypeName)
{
    return typeNameTable.find(pascalTypeName) != typeNameTable.end()
                            ? typeNameTable[pascalTypeName] : pascalTypeName;
}

Object Converter::visitTypeIdentifier(PascalParser::TypeIdentifierContext *ctx)
{
    Typespec *pascalType = ctx->type;
    string javaTypeName = typeName(pascalType);
    code.emit(javaTypeName);

    return nullptr;
}

Object Converter::visitVariableIdentifierList(
                             PascalParser::VariableIdentifierListContext *ctx)
{
    string separator = " ";

    for (PascalParser::VariableIdentifierContext *varCtx :
                                                    ctx->variableIdentifier())
    {
        code.emit(separator);
        code.emit(varCtx->getText());
        separator = ", ";
    }

    return nullptr;
}

Object Converter::visitRoutineDefinition(
                                    PascalParser::RoutineDefinitionContext *ctx)
{
    PascalParser::FunctionHeadContext  *funcCtx  = ctx->functionHead();
    PascalParser::ProcedureHeadContext *procCtx  = ctx->procedureHead();
    PascalParser::RoutineIdentifierContext *idCtx = nullptr;
    PascalParser::ParametersContext *parmsCtx = nullptr;
    bool functionDefinition = funcCtx != nullptr;
    string routineName;

    programVariables = false;
    code.emitLine();
    code.emitStart();

    if (functionDefinition)
    {
        idCtx = funcCtx->routineIdentifier();
        parmsCtx = funcCtx->parameters();
        visit(funcCtx->typeIdentifier());
    }
    else
    {
        idCtx = procCtx->routineIdentifier();
        parmsCtx = procCtx->parameters();
        code.emit("void");
    }

    routineName = idCtx->entry->getName();
    code.emit(" " + routineName);

    code.emit("(");
    if (parmsCtx != nullptr) visit(parmsCtx);
    code.emitEnd(")");
    code.emitLine("{");
    code.indent();

    if (functionDefinition)
    {
        // Function associated variable.
        code.emitStart();
        visit(funcCtx->typeIdentifier());
        code.emit(" " + routineName + ";");
        code.emitLine();
    }

    visit(ctx->block()->declarations());
    visit(ctx->block()->compoundStatement()->statementList());

    if (functionDefinition)
    {
        // Return function value.
        code.emitLine();
        code.emitLine("return " + routineName + ";");
    }

    code.dedent();
    code.emitLine("}");

    return nullptr;
}

Object Converter::visitParameters(PascalParser::ParametersContext *ctx)
{
    currentSeparator = "";

    code.mark();
    visit(ctx->parameterDeclarationsList());

    return nullptr;
}

Object Converter::visitParameterDeclarations(
                                PascalParser::ParameterDeclarationsContext *ctx)
{
    bool varParm = ctx->VAR() != nullptr;
    PascalParser::ParameterIdentifierListContext *parmListCtx =
                                                ctx->parameterIdentifierList();
    PascalParser::TypeIdentifierContext *typeCtx = ctx->typeIdentifier();
    Typespec *parmType = typeCtx->type;

    // Loop over the parameters.
    for (PascalParser::ParameterIdentifierContext *parmIdCtx :
                                            parmListCtx->parameterIdentifier())
    {
        code.emit(currentSeparator);
        code.split(60);

        visit(typeCtx);
        if (varParm && (typeCtx->type->getForm() != ARRAY)) code.emit("&");
        code.emit(" " + parmIdCtx->entry->getName());

        if (parmType->getForm() == ARRAY) emitArrayDimensions(parmType);
        currentSeparator = ", ";
    }

    return nullptr;
}

Object Converter::visitStatementList(PascalParser::StatementListContext *ctx)
{
    for (PascalParser::StatementContext *stmtCtx : ctx->statement())
    {
        if (stmtCtx->emptyStatement() == nullptr)
        {
            code.emitStart();
            visit(stmtCtx);
        }
    }

    return nullptr;
}

Object Converter::visitCompoundStatement(
                                    PascalParser::CompoundStatementContext *ctx)
{
    code.emit("{");
    code.indent();
    visitChildren(ctx);
    code.dedent();
    code.emitLine("}");

    return nullptr;
}

Object Converter::visitAssignmentStatement(
                                PascalParser::AssignmentStatementContext *ctx)
{
    string lhs  = visit(ctx->lhs()->variable()).as<string>();
    string expr = visit(ctx->rhs()->expression()).as<string>();
    code.emit(lhs + " = " + expr);
    code.emitEnd(";");

    return nullptr;
}

Object Converter::visitRepeatStatement(PascalParser::RepeatStatementContext *ctx)
{
    bool needBraces = ctx->statementList()->statement().size() > 1;

    code.emit("do");
    if (needBraces) code.emitLine("{");
    code.indent();

    visit(ctx->statementList());

    code.dedent();
    if (needBraces) code.emitLine("}");

    code.emitStart("while (!(");
    code.emit(visit(ctx->expression()).as<string>());
    code.emitEnd("));");

    return nullptr;
}

Object Converter::visitWhileStatement(PascalParser::WhileStatementContext *ctx) {
	code.emit("while  (");
	code.emit(visit(ctx->expression()).as<string>());
	code.emit(") ");

	visit(ctx->statement());

	return nullptr;
}
Object Converter::visitForStatement(PascalParser::ForStatementContext *ctx)
{
	code.emit("for(");
	string var = (visit(ctx->variable())).as<string>();
	code.emit(var);
	code.emit("=");
	code.emit(visit(ctx->expression(0)).as<string>());
	code.emit(";");
	code.emit(var);

	bool to = ctx->TO() != nullptr;
	if(to) code.emit("<");
	else code.emit(">");
	code.emit(visit(ctx->expression(1)).as<string>());
	code.emit(";");

	code.emit(var);
	if(to) code.emit("++");
	else code.emit("--");
	code.emit(")");


	code.emit(visit(ctx->statement()).as<string>());


	return nullptr;
}

Object Converter::visitIfStatement(PascalParser::IfStatementContext *ctx)
{
	code.emit("if  (");
	code.emit(visit(ctx->expression()).as<string>());
	code.emit(") ");

	visit(ctx->trueStatement());

	if(ctx->falseStatement() != nullptr) {
		code.emitStart("else ");
		visit(ctx->falseStatement());
	}

	return nullptr;
}
	
Object Converter::visitCaseStatement(PascalParser::CaseStatementContext *ctx)
{
	code.emit("switch(");
	code.emit(visit(ctx->expression()).as<string>());
	code.emit(") {");
	code.indent();

	PascalParser::CaseBranchListContext *branchListCtx = ctx->caseBranchList();
	for (PascalParser::CaseBranchContext *branchCtx :branchListCtx->caseBranch())
	{
		PascalParser::CaseConstantListContext *constListCtx = branchCtx->caseConstantList();
		PascalParser::StatementContext *stmtCtx = branchCtx->statement();

		if (constListCtx != nullptr) {
			// Loop over the CASE constants of each CASE branch.
			for (PascalParser::CaseConstantContext *caseConstCtx :constListCtx->caseConstant()) {
				code.emitStart("case ");
				code.emit(to_string(caseConstCtx->value));
				code.emitEnd(":");
			}

			visit(stmtCtx);
			code.emitLine("break; ");
		}
		else {
			visit(branchCtx);
		}
	}
	code.emit("}");

	return nullptr;
}	

Object Converter::visitProcedureStatement(PascalParser::ProcedureCallStatementContext *ctx)
{
	string name = (visit(ctx->procedureName()));
	code.emit(name);
	code.emit("(");
	//Parameters - argument lists
	string param = (visit(ctx->argumentList()));
	code.emit(")");
	code.emit(";");

	return nullptr;
}
Object Converter::visitFunctionCallFactor(PascalParser::FunctionCallFactorContext *ctx)
{
    PascalParser::FunctionCallContext *callCtx = ctx->functionCall();
    SymtabEntry *routineId = callCtx->functionName()->entry;
    PascalParser::ArgumentListContext *argListCtx = callCtx->argumentList();
	string name = routineId->getName();
	//gotta code.emit the return type
	code.emit(name);
	code.emit("(");
	//Parameters - argument lists
	vector<SymtabEntry *> *parms = routineId->getRoutineParameters();
	for (int i = 0; i < parms->size(); i++)
	{
		SymtabEntry *parmId = (*parms)[i];
		string parmName = parmId->getName();
		code.emit(parmName);
		if(i- parms->size()>0)
			code.emit(",");
	}
	code.emit(")");
	code.emit(";");

	return nullptr;
}
Object Converter::visitExpression(PascalParser::ExpressionContext *ctx)
{
    PascalParser::SimpleExpressionContext *simpleCtx1 =
                                                    ctx->simpleExpression()[0];
    PascalParser::RelOpContext *relopCtx = ctx->relOp();
    string simpleText1 = visit(simpleCtx1);
    string text = simpleText1;

    // Second simple expression?
    if (relopCtx != nullptr)
    {
        string op = relopCtx->getText();

        if      (op == "=")  op = "==";
        else if (op == "<>") op = "!=";

        PascalParser::SimpleExpressionContext *simpleCtx2 =
                                                    ctx->simpleExpression()[1];
        string simpleText2 = visit(simpleCtx2);
        text = simpleText1 + " " + op + " " + simpleText2;
    }

    return text;
}

Object Converter::visitSimpleExpression(
                                    PascalParser::SimpleExpressionContext *ctx)
{
    int count = ctx->term().size();
    string text = "";
    bool needParens = false;  // surround "or" terms with parentheses

    if ((ctx->sign() != nullptr) && (ctx->sign()->getText() == "-")) text += "-";

    // Loop over the simple expressions.
    for (int i = 0; i < count; i++)
    {
        PascalParser::TermContext *termCtx = ctx->term()[i];
        string termText = visit(termCtx);

        if (i < count-1)
        {
            string addop = toLowerCase(ctx->addOp()[i]->getText());
            if (addop == "or")
            {
                addop = "||";
                needParens = true;
            }

            if (needParens) text += "(" + termText + ") " + addop + " ";
            else            text += termText + " " + addop + " ";
        }
        else if (needParens) text += "(" + termText + ") ";
        else                 text += termText;
    }

    return text;
}

Object Converter::visitTerm(PascalParser::TermContext *ctx)
{
    int count = ctx->factor().size();
    string text = "";

    for (int i = 0; i < count; i++)
    {
        PascalParser::FactorContext *factorCtx = ctx->factor()[i];
        text += visit(factorCtx).as<string>();

        if (i < count-1)
        {
            string mulop = toLowerCase(ctx->mulOp()[i]->getText());
            if      (mulop == "and") mulop = " && ";
            else if (mulop == "div") mulop = "/";
            else if (mulop == "mod") mulop = "%";

            text = text += mulop;
        }
    }

    return text;
}

Object Converter::visitVariableFactor(PascalParser::VariableFactorContext *ctx)
{
    return visit(ctx->variable()).as<string>();
}

Object Converter::visitVariable(PascalParser::VariableContext *ctx)
{
    PascalParser::VariableIdentifierContext *idCtx = ctx->variableIdentifier();
    SymtabEntry *variableId = idCtx->entry;
    string variableName = variableId->getName();
    Typespec *type = ctx->variableIdentifier()->type;

    if (    (type != Predefined::booleanType)
         && (variableId->getKind() == ENUMERATION_CONSTANT))
    {
        variableName = type->getIdentifier()->getName() + "::" + variableName;
    }

    // Loop over any subscript and field modifiers.
    for (PascalParser::ModifierContext *modCtx : ctx->modifier())
    {
        // Subscripts.
        if (modCtx->indexList() != nullptr)
        {
            for (PascalParser::IndexContext *indexCtx :
                                                modCtx->indexList()->index())
            {
                Typespec *indexType = type->getArrayIndexType();
                int minIndex = 0;

                if (indexType->getForm() == SUBRANGE)
                {
                    minIndex = indexType->getSubrangeMinValue();
                }

                PascalParser::ExpressionContext *exprCtx =
                                                    indexCtx->expression();
                string expr = visit(exprCtx).as<string>();
                string subscript =
                  (minIndex == 0) ? expr
                : (minIndex < 0)  ? "(" + expr + ")+" + to_string(-minIndex)
                :                   "(" + expr + ")-" + to_string(minIndex);

                variableName += "[" + subscript + "]";

                type = type->getArrayElementType();
            }
        }
        else // Record field.
        {
            PascalParser::FieldContext *fieldCtx = modCtx->field();
            string fieldName = fieldCtx->entry->getName();
            variableName += "." + fieldName;
            type = fieldCtx->type;
        }
    }

    return variableName;
}

Object Converter::visitNumberFactor(PascalParser::NumberFactorContext *ctx)
{
    return ctx->getText();
}

Object Converter::visitCharacterFactor(PascalParser::CharacterFactorContext *ctx)
{
    return ctx->getText();
}

Object Converter::visitStringFactor(PascalParser::StringFactorContext *ctx)
{
    string pascalString = ctx->stringConstant()->STRING()->getText();
    return "string(\"" + convertString(pascalString, true) + "\")";
}

Object Converter::visitNotFactor(PascalParser::NotFactorContext *ctx)
{
    return "!" + visit(ctx->factor()).as<string>();
}

Object Converter::visitParenthesizedFactor(
                                PascalParser::ParenthesizedFactorContext *ctx)
{
    return "(" + visit(ctx->expression()).as<string>() + ")";
}

Object Converter::visitWriteStatement(PascalParser::WriteStatementContext *ctx)
{
    code.emit("printf(");
    code.mark();

    string format    = createWriteFormat(ctx->writeArguments());
    string arguments = createWriteArguments(ctx->writeArguments());

    code.emit("\"" + format + "\"");

    if (arguments.length() > 0)
    {
        code.emit(", ");
        code.split(60);
        code.emit(arguments);
    }

    code.emitEnd(");");
    return nullptr;
}

Object Converter::visitWritelnStatement(
                                    PascalParser::WritelnStatementContext *ctx)
{
    if (ctx->writeArguments() != nullptr)
    {
        code.emit("printf(");
        code.mark();

        string format    = createWriteFormat(ctx->writeArguments());
        string arguments = createWriteArguments(ctx->writeArguments());

        code.emit("\"" + format + "\\n\"");  // append line feed

        if (arguments.length() > 0)
        {
            code.emit(", ");
            code.split(60);
            code.emit(arguments);
        }

        code.emitEnd(");");
    }
    else
    {
        code.emit("cout << endl;");
    }

    return nullptr;
}

string Converter::createWriteFormat(PascalParser::WriteArgumentsContext *ctx)
{
    string format = "";

    // Loop over the write arguments.
    for (PascalParser::WriteArgumentContext *argCtx : ctx->writeArgument())
    {
        Typespec *type = argCtx->expression()->type;
        string argText = argCtx->getText();

        // Append any literal strings.
        if (argText[0] == '\'')
        {
            format += convertString(argText, true);
        }

        // For any other expressions, append a field specifier.
        else
        {
            format += "%";

            PascalParser::FieldWidthContext *fwCtx = argCtx->fieldWidth();
            if (fwCtx != nullptr)
            {
                string sign = (   (fwCtx->sign() != nullptr)
                               && (fwCtx->sign()->getText() == "-")) ? "-" : "";
                format += sign + fwCtx->integerConstant()->getText();

                PascalParser::DecimalPlacesContext *dpCtx = fwCtx->decimalPlaces();
                if (dpCtx != nullptr)
                {
                    format += "." + dpCtx->integerConstant()->getText();
                }
            }

            string typeFlag = type == Predefined::integerType ? "d"
                            : type == Predefined::realType    ? "f"
                            : type == Predefined::charType    ? "c"
                            : type == Predefined::booleanType ? "d"
                            :                                   "s";
            format += typeFlag;
        }
    }

    return format;
}

string Converter::createWriteArguments(PascalParser::WriteArgumentsContext *ctx)
{
    string arguments = "";
    string separator = "";

    // Loop over the write arguments.
    for (PascalParser::WriteArgumentContext *argCtx : ctx->writeArgument())
    {
        string argText = argCtx->getText();

        // Not a literal string.
        if (argText[0] != '\'')
        {
            string arg = visit(argCtx->expression()).as<string>();

            if (argCtx->expression()->type == Predefined::stringType)
            {
                arguments += separator + "(" + arg + ")" + ".c_str()";
            }
            else
            {
                arguments += separator + arg;
            }

            separator = ", ";
        }
    }

    return arguments;
}

Object Converter::visitReadStatement(PascalParser::ReadStatementContext *ctx)
{
    if (ctx->readArguments()->variable().size() == 1)
    {
        visit(ctx->readArguments());
    }
    else
    {
        code.emit("{");
        code.indent();
        code.emitStart();

        visit(ctx->readArguments());

        code.dedent();
        code.emitLine("}");
    }
    return nullptr;
}

Object Converter::visitReadlnStatement(PascalParser::ReadlnStatementContext *ctx)
{
    code.emit("{");
    code.indent();
    code.emitStart();

    visit(ctx->readArguments());
    code.emitLine("cin.ignore(4096, '\\n');");

    code.dedent();
    code.emitLine("}");

    return nullptr;
}

Object Converter::visitReadArguments(PascalParser::ReadArgumentsContext *ctx)
{
    int size = ctx->variable().size();

    for (int i = 0; i < size; i++)
    {
        PascalParser::VariableContext *varCtx = ctx->variable()[i];
        string varName = varCtx->getText();
        Typespec *type = varCtx->type;

        if (type == Predefined::charType)
        {
            code.emit(varName + " = getchar();");
        }
        else if (type == Predefined::booleanType)
        {
            code.emit("cin >> boolalpha >> " + varName + ";");
        }
        else
        {
            code.emit("cin >> " + varName + ";");
        }

        if (i < size-1) code.emitStart();
    }

    return nullptr;
}

}} // namespace backend::converter

#include <iostream>
#include <fstream>

#include "antlr4-runtime.h"

#include "Object.h"
#include "Pcl4Lexer.h"
#include "Pcl4Parser.h"
#include "Executor.h"

using namespace antlrcpp;
using namespace antlr4;
using namespace backend::interpreter;
using namespace std;

/**
 * Execute the source program.
 * @param tree the parse tree root.
 */
void executeProgram(tree::ParseTree *tree);

int main(int argc, const char *args[])
{
    if (argc != 3)
    {
        cout << "USAGE: PascalJava option sourceFileName" << endl;
        cout << "   option: -execute, -convert, or -compile" << endl;
        return -1;
    }

    string operation = toLowerCase(args[1]);
    string sourceFileName = args[2];

    ifstream source;

    // Create the input stream.
    source.open(sourceFileName);
    ANTLRInputStream input(source);

    // Create a lexer which scans the input stream
    // to create a token stream.
    Pcl4Lexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    // Create a parser which parses the token stream
    // to create a parse tree.
    Pcl4Parser parser(&tokens);
    tree::ParseTree *tree = parser.program();

    // Backend operation.
    if (operation == "-execute") executeProgram(tree);
    else
    {
        cout << "USAGE: PascalJava option sourceFileName" << endl;
        cout << "   option: -execute, -convert, or -compile" << endl;
        return -1;
    }

    return 0;
}

void executeProgram(tree::ParseTree *tree)
{
    cout << "Execution:" << endl << endl;
    Executor executor;
    executor.visit(tree);
}

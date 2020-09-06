/**
 * Scanner class for a simple interpreter.
 *
 * (c) 2020 by Ronald Mak
 * Department of Computer Science
 * San Jose State University
 */
#ifndef SCANNER_H_
#define SCANNER_H_

#include "Source.h"
#include "Token.h"

namespace frontend {

using namespace std;

class Scanner
{
private:
    Source *source;

public:
    /**
     * Constructor.
     * @param source the input source.
     */
    Scanner(Source *source) : source(source) {}

    /**
     * Extract the next token from the source.
     * @return the token.
     */
    Token *nextToken()
    {
        char ch = source->currentChar();
        skipped:
        // Skip blanks and other whitespace characters.
        while (isspace(ch)) ch = source->nextChar();

        if(ch == '{') {
        	while(ch != '}')
        		ch = source->nextChar();
        	ch = source->nextChar();
        	//while (isspace(ch)) ch = source->nextChar();
        	goto skipped;
        }

        if (isalpha(ch))      return Token::Word(ch, source);
        else if (isdigit(ch)) return Token::Number(ch, source);
        else if (ch == '\'')  return Token::String(ch, source);
        else                  return Token::SpecialSymbol(ch, source);
    }
};

}  // namespace frontend

#endif /* SCANNER_H_ */

/**
 * Token class for a simple interpreter.
 *
 * (c) 2020 by Ronald Mak
 * Department of Computer Science
 * San Jose State University
 */
#include <string>
#include <map>
#include <ctype.h>

#include "../Object.h"
#include "Token.h"

namespace frontend {

using namespace std;

map<string, TokenType> Token::reservedWords;

void Token::initialize()
{
    reservedWords["PROGRAM"] = PROGRAM;
    reservedWords["BEGIN"]   = BEGIN;
    reservedWords["END"]     = END;
    reservedWords["REPEAT"]  = REPEAT;
    reservedWords["UNTIL"]   = UNTIL;
    reservedWords["WRITE"]   = WRITE;
    reservedWords["WRITELN"] = WRITELN;
    reservedWords["DIV"] 	 = DIV;
    reservedWords["MOD"]  	 = MOD;
    reservedWords["AND"]     = AND;
    reservedWords["OR"] 	 = OR;
    reservedWords["NOT"]  	 = NOT;
    reservedWords["CONST"]   = CONST;
    reservedWords["TYPE"]	 = TYPE;
    reservedWords["VAR"]	 = VAR;
    reservedWords["PROCEDURE"]   = PROCEDURE;
    reservedWords["FUNCTION"]    = FUNCTION;
    reservedWords["WHILE"] 	 = WHILE;
    reservedWords["DO"]  	 = DO;
    reservedWords["FOR"]  	 = FOR;
    reservedWords["TO"]		 = TO;
    reservedWords["DOWNTO"]  = DOWNTO;
    reservedWords["IF"]  	 = IF;
    reservedWords["THEN"]    = THEN;
    reservedWords["ELSE"] 	 = ELSE;
    reservedWords["CASE"]    = CASE;
    reservedWords["OF"]  	 = OF;

}

Token *Token::Word(char firstChar, Source *source)
{
    Token *token = new Token(firstChar);
    token->lineNumber = source->lineNumber();

    // Loop to get the rest of the characters of the word token.
    // Append letters and digits to the token.
    for (char ch = source->nextChar(); isalnum(ch); ch = source->nextChar())
    {
        token->text += ch;
    }

    // Is it a reserved word or an identifier?
    string upper = toUpperCase(token->text);
    if (Token::reservedWords.find(upper) != Token::reservedWords.end())
    {
        token->type = Token::reservedWords[upper];
    }
    else
    {
        token->type = TokenType::IDENTIFIER;
    }

    return token;
}

Token *Token::Number(char firstChar, Source *source)
{
    Token *token = new Token(firstChar);
    int pointCount = 0;

    // Loop to get the rest of the characters of the number token.
    // Append digits to the token.
    for (char ch = source->nextChar();
         isdigit(ch) || (ch == '.');
         ch = source->nextChar())
    {
        if (ch == '.') pointCount++;
        token->text += ch;
    }

    // Integer constant.
    if (pointCount == 0)
    {
        token->type    = TokenType::INTEGER;
        token->value.L = stol(token->text);
        token->value.D = token->value.L;  // allow using integer value as double
    }

    // Real constant.
    else if (pointCount == 1)
    {
        token->type    = TokenType::REAL;
        token->value.D = stod(token->text);
    }

    else tokenError(token, "Invalid token", source);

    return token;
}

Token *Token::String(char firstChar, Source *source)
{
    Token *token = new Token(firstChar);  // the leading '

    char ch = source->nextChar(); //pick up the character following the leading '
    if(ch == '\'') {//if ''
    	token->text += ch; //add the ' -- may be for closing or for apostrophe
    	ch = source->nextChar();
    	if (ch == '\'') {//if '''
    		//the token is not complete
    		//we already added the apostrophe
    		ch = source->nextChar(); //pick up the char following the apostrophe
    	}
    	else {
    		source->nextChar(); //burn the ending quote
    		goto token_complete;
    	}
    }

    //loop to append the rest of the characters of the string,
    //up to but not including the closing quote
    for(; ch!='\''; ch = source->nextChar()) {
    	token->text += ch;
    }
    token->text += '\''; //the closing quote (or the apostrophe)

    //what if that was only the first ' of an apostrophe?
  apostrophe_case:
    ch = source->nextChar();
    if(ch == '\'') {//the presumed 2nd ' of the apostrophe
    	for (ch=source->nextChar(); ch!= '\''; ch=source->nextChar()) {
    		if(ch == EOF) {
    			token->type = TokenType::ERROR;
    			tokenError(token, "String not closed", source);
    			return token;
    		}
    		token->text += ch;
    	}
    	token->text += '\''; //the closing quote (or another apostrophe actually)
    	goto apostrophe_case;
    }

  token_complete:

   	if (token->text.length() ==3)
   		token->type = TokenType::CHARACTER;
   	else
   		token->type = TokenType::STRING;

    // Don't include the leading and trailing '.
    token->value.S = token->text.substr(1, token->text.length() - 2);

    return token;
}

Token *Token::SpecialSymbol(char firstChar, Source *source)
{
    Token *token = new Token(firstChar);

    switch (firstChar)
    {
        case '.' :
                {
                    char nextChar = source->nextChar();

                    // Is it the .. symbol?
                    if (nextChar == '.')
                    {
                    	token->text += nextChar;
                        token->type = TokenType::PERIOD_PERIOD;
                    }

                    // No, it's just the . symbol.
                    else
                    {
                        token->type = TokenType::PERIOD;
                        return token;  // already consumed first .
                    }

                    break;
                }
        case ',' : token->type = TokenType::COMMA;      break;
        case ':' :
                {
                    char nextChar = source->nextChar();

                    // Is it the := symbol?
                    if (nextChar == '=')
                    {
                    	token->text += nextChar;
                        token->type = TokenType::COLON_EQUALS;
                    }

                    // No, it's just the : symbol.
                    else
                    {
                        token->type = TokenType::COLON;
                        return token;  // already consumed :
                    }

                    break;
                }
        case ';' : token->type = TokenType::SEMICOLON;  break;
        case '+' : token->type = TokenType::PLUS;       break;
        case '-' : token->type = TokenType::MINUS;      break;
        case '*' : token->type = TokenType::STAR;       break;
        case '/' : token->type = TokenType::SLASH;      break;
        case '(' : token->type = TokenType::LPAREN;     break;
        case ')' : token->type = TokenType::RPAREN;     break;

        case '=' : token->type = TokenType::EQUALS;     break;
        case '<' :
                {
                    char nextChar = source->nextChar();

                    // Is it the <> symbol?
                    if (nextChar == '>')
                    {
                    	token->text += nextChar;
                        token->type = TokenType::LESSER_GREATER;
                        //printf("found a lesser-greater");
                    }
                    // Is it the <= symbol?
                    else if (nextChar == '=')
                    {
                    	token->text += nextChar;
                    	token->type = TokenType::LESS_EQUALS;
                    }

                    // No, it's just the < symbol.
                    else
                    {
                        token->type = TokenType::LESS_THAN;
                        return token;  // already consumed <
                    }

                    break;
                }
        case '>' :
                {
                    char nextChar = source->nextChar();

                    // Is it the >= symbol?
                    if (nextChar == '=')
                    {
                    	token->text += nextChar;
                        token->type = TokenType::GREATER_EQUALS;
                    }

                    // No, it's just the > symbol.
                    else
                    {
                        token->type = TokenType::GREATER_THAN;
                        return token;  // already consumed >
                    }

                    break;
                }
        case '\'' : token->type = TokenType::SINGLE_QUOTE;break;
        case '[': token->type = TokenType::LBRACKET; 	break;
        case ']': token->type = TokenType::RBRACKET; 	break;
        case '^': token->type = TokenType::POINTER; 	break;



        case EOF : token->type = END_OF_FILE; break;

        default: token->type = TokenType::ERROR;
        	tokenError(token, "Invalid token", source);
    }

    source->nextChar();  // consume the special symbol
    return token;
}

void Token::tokenError(Token *token, string message, Source *source)
{
	token->lineNumber = source->lineNumber();
    printf("TOKEN ERROR at line %d: %s at '%s'\n",
           token->lineNumber, message.c_str(), token->text.c_str());
}

}  // namespace frontend

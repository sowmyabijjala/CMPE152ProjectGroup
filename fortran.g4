grammar Fortran;

@header {
    #include <map>
    #include "intermediate/symtab/Symtab.h"
    #include "intermediate/type/Typespec.h"
    using namespace intermediate::symtab;
    using namespace intermediate::type;
}

program           : programHeader block programIdentifier (function)*; // this is the definition for the whole program
programHeader     : PROGRAM programIdentifier implicitNone ;

programIdentifier   locals [ SymtabEntry *entry = nullptr ]
    : IDENTIFIER ;

implicitNone      : IMPLICIT 'none'; //IMPLICIT is a keyword but NONE isn't

block         : declarations compoundStatement ;
compoundStatement : statementList END PROGRAM ; //statement list in line 72
declarations  : ( variablesPart)? ; //theoretically we may also need a constants part and a routines part

constantIdentifier  locals [ Typespec *type = nullptr, SymtabEntry *entry = nullptr ]
    : IDENTIFIER ;
    
constant            locals [ Typespec *type = nullptr, Object value = nullptr ]  
    : sign? ( IDENTIFIER | unsignedNumber )
    | characterConstant
    | stringConstant
    ;

characterConstant : CHARACTER ;
stringConstant    : STRING ;
number          : sign? unsignedNumber ;
unsignedNumber  : integerConstant | realConstant ;
integerConstant : INTEGER ;
realConstant    : REAL;

sign : '-' | '+' ;

variablesPart            : variableDeclarationsList ;
variableDeclarationsList : variableDeclarations ( variableDeclarations )* ; //doesn't support arrays
variableDeclarations     : typeSpecification '::' variableIdentifierList ;
variableIdentifierList   : variableIdentifier ( ',' variableIdentifier )* ;

variableIdentifier  locals [ Typespec *type = nullptr, SymtabEntry *entry = nullptr ] 
    : IDENTIFIER ;
    
typeSpecification   locals [ Typespec *type = nullptr ] 
      : simpleType ;       //# simpleTypespec //variable declarations still need this

typeIdentifier      locals [ Typespec *type = nullptr, SymtabEntry *entry = nullptr ]
    : IDENTIFIER ;

//we don't really get this section yet
simpleType          locals [ Typespec *type = nullptr ] 
    : typeIdentifier    # typeIdentifierTypespec 
    | enumerationType   # enumerationTypespec
    | subrangeType      # subrangeTypespec
    ;
           
enumerationType     : '(' enumerationConstant ( ',' enumerationConstant )* ')' ;
enumerationConstant : constantIdentifier ;
subrangeType        : constant '..' constant ;
//^ down to here    

statement : //removed compound statement bc we don't use begin/end
			ifStatement 
          | assignmentStatement
          | doWhileStatement
          | printStatement
        //| emptyStatement removed to support statementList function
          ;
          
statementList    : statement ( statement )* ;
//emptyStatement : ;

assignmentStatement : lhs '=' rhs;

lhs                 locals [ Typespec *type = nullptr ] 
    : variable ;
rhs : expression ;

variable        locals [ Typespec *type = nullptr, SymtabEntry *entry = nullptr ] 
    : variableIdentifier modifier* ;

modifier  : '[' indexList ']' | '.' field ;
indexList : index ( ',' index )* ;
index     : expression ; 

field           locals [ Typespec *type = nullptr, SymtabEntry *entry = nullptr ]     
    : IDENTIFIER ;
    
expression          locals [ Typespec *type = nullptr ] 
    : simpleExpression (relOp simpleExpression)? ;

simpleExpression    locals [ Typespec *type = nullptr ] 
    : sign? term (addOp term)* ;

term                locals [ Typespec *type = nullptr ]
    : factor (mulOp factor)* ;

factor              locals [ Typespec *type = nullptr ] 
    : 
     functionCall		   # functionCallFactor
    | variable             # variableFactor
    | number               # numberFactor
    | NOT factor           # notFactor
    | characterConstant	   # characterFactor
    | stringConstant	   # stringFactor
    | '(' expression ')'   # parenthesizedFactor
    ;
    
relOp : '==' | '<>' | '<' | '<=' | '>' | '>=' ;
addOp : '+' | '-' | OR ;
mulOp : '*' | '/' | AND ;

ifStatement    : IF LPAREN expression RPAREN THEN trueStatement ( ELSE falseStatement )? END IF;
trueStatement  : statementList ;
falseStatement : statementList ;

doWhileStatement : DO WHILE LPAREN expression RPAREN trueStatement END DO;

printStatement   : PRINT '*' ',' printArguments ;
printArguments   :  printArgument (',' printArgument)* ;
printArgument    : expression ;

//Function declaration
function    : FUNCTION  routineIdentifier parameters? implicitNone ('result' '(' variable ')')?  declarations statementList END FUNCTION routineIdentifier;
routineIdentifier   locals [ Typespec *type = nullptr, SymtabEntry *entry = nullptr ]
    : IDENTIFIER ;
parameters                : '(' parameterIdentifierList ')' ;
parameterIdentifierList   : parameterIdentifier ( ',' parameterIdentifier )* ;
parameterIdentifier   locals [ Typespec *type = nullptr, SymtabEntry *entry = nullptr ]
    : IDENTIFIER ;
			 
 
//calling a funtion
functionCall : functionName '(' argumentList? ')';
functionName    locals [ Typespec *type = nullptr, SymtabEntry *entry = nullptr ] 
    : IDENTIFIER ;
argumentList : argument ( ',' argument )* ;
argument     : expression ;

fragment A : ('a' | 'A') ;
fragment B : ('b' | 'B') ;
fragment C : ('c' | 'C') ;
fragment D : ('d' | 'D') ;
fragment E : ('e' | 'E') ;
fragment F : ('f' | 'F') ;
fragment G : ('g' | 'G') ;
fragment H : ('h' | 'H') ;
fragment I : ('i' | 'I') ;
fragment J : ('j' | 'J') ;
fragment K : ('k' | 'K') ;
fragment L : ('l' | 'L') ;
fragment M : ('m' | 'M') ;
fragment N : ('n' | 'N') ;
fragment O : ('o' | 'O') ;
fragment P : ('p' | 'P') ;
fragment Q : ('q' | 'Q') ;
fragment R : ('r' | 'R') ;
fragment S : ('s' | 'S') ;
fragment T : ('t' | 'T') ;
fragment U : ('u' | 'U') ;
fragment V : ('v' | 'V') ;
fragment W : ('w' | 'W') ;
fragment X : ('x' | 'X') ;
fragment Y : ('y' | 'Y') ;
fragment Z : ('z' | 'Z') ;

PROGRAM   : P R O G R A M ;
CONST     : C O N S T ;
IMPLICIT  : I M P L I C I T;
END				: E N D;

//logical operators
AND				: '.' A N D '.';
OR				: '.' O R '.';
NOT				: '.' N O T '.';
EQV		: '.' E Q V '.';
NOT_EQV: '.' N E Q V '.';

IF        : I F ;
THEN      : T H E N ;
ELSE      : E L S E ;

DO				: D O ;
WHILE			: W H I L E ;

PRINT	: P R I N T ;
FUNCTION  : F U N C T I O N ;


IDENTIFIER : [a-zA-Z][a-zA-Z0-9]* ;

NEWLINE : '\r'? '\n' -> skip  ;
WS      : [ \t]+ -> skip ; 
LPAREN  : '(';
RPAREN  : ')';
INTEGER    : [0-9]+ ;
REAL       : INTEGER '.' INTEGER
           | INTEGER ('e' | 'E') ('+' | '-')? INTEGER
           | INTEGER '.' INTEGER ('e' | 'E') ('+' | '-')? INTEGER
           ;

QUOTE     : '"' ;
CHARACTER : QUOTE CHARACTER_CHAR QUOTE ;
STRING    : QUOTE STRING_CHAR* QUOTE ;

fragment CHARACTER_CHAR : ~('"')   // any non-quote character
                        ;

fragment STRING_CHAR :  ~('"')      // any non-quote character
                     ;
                     
                     
COMMENT : '!' COMMENT_CHARACTER* NEWLINE -> skip ;

fragment COMMENT_CHARACTER : ~('\n') ;

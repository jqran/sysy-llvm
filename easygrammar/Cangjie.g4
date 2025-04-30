grammar Cangjie;
import Lex;

identifier
    : Identifier
    ;

translationUnit
    : end* topLevelObject* (end+ mainDefinition)? NL* (topLevelObject (end+ topLevelObject?)*)? EOF
    ;

end
    : NL | SEMI
    ;

topLevelObject
    : functionDefinition
    | variableDeclaration
    ;

functionDefinition
    :FUNC
    NL* identifier
    NL* functionParameters
    (NL* COLON NL* type)?
    (NL* block)?
    ;

functionParameters
    : | (LPAREN NL* (namedParameterList NL*)? RPAREN NL*)
    ;


namedParameterList
    : (namedParameter | defaultParameter)
      (NL* COMMA NL* (namedParameter | defaultParameter))*
    ;
    
namedParameter
    : identifier NL* NOT NL* COLON NL* type
    ;

defaultParameter
    : identifier NL* NOT NL* COLON NL* type NL* ASSIGN NL* expression
    ;

variableDeclaration
    :NL* (LET | VAR | CONST) NL* identifier ( (NL* COLON NL* type)? (NL* ASSIGN NL* expression)
                                                                     | (NL* COLON NL* type)
                                                                     )
    ;


mainDefinition
    : MAIN
      NL* functionParameters
      (NL* COLON NL* type)?
      NL* block
    ;

type
    : charLangTypes
    ;


charLangTypes
    : numericTypes
    | BOOLEAN
    | UNIT
    ;

numericTypes
    : INT8
    | INT16
    | INT32
    | INT64
    | INTNATIVE
    | UINT8
    | UINT16
    | UINT32
    | UINT64
    | UINTNATIVE
    | FLOAT16
    | FLOAT32
    | FLOAT64
    ;

expression
    : assignmentExpression
    ;

assignmentExpression
    : leftValueExpression NL* ASSIGN NL* logicDisjunctionExpression
    | logicDisjunctionExpression
    ;

leftValueExpression
    : identifier
    ;

logicDisjunctionExpression
    : logicConjunctionExpression (NL* OR NL* logicConjunctionExpression)*
    ;

logicConjunctionExpression
    : bitwiseDisjunctionExpression (NL* AND NL* bitwiseDisjunctionExpression)*
    ;
bitwiseDisjunctionExpression
    : bitwiseXorExpression (NL* BITOR NL* bitwiseXorExpression)*
    ;

bitwiseXorExpression
    : bitwiseConjunctionExpression (NL* BITXOR NL* bitwiseConjunctionExpression)*
    ;

bitwiseConjunctionExpression
    : equalityComparisonExpression (NL* BITAND NL* equalityComparisonExpression)*
    ;

equalityComparisonExpression
    : comparisonOrTypeExpression (NL* equalityOperator NL* comparisonOrTypeExpression)?
    ;

comparisonOrTypeExpression
    : shiftingExpression (NL* comparisonOperator NL* shiftingExpression)?
    ;

shiftingExpression
    : additiveExpression (NL* shiftingOperator NL* additiveExpression)*
    ;

additiveExpression
    : multiplicativeExpression (NL* additiveOperator NL* multiplicativeExpression)*
    ;

multiplicativeExpression
    : exponentExpression (NL* multiplicativeOperator NL* exponentExpression)*
    ;

exponentExpression
    : prefixUnaryExpression (NL* exponentOperator NL* prefixUnaryExpression)*
    ;

prefixUnaryExpression
    : prefixUnaryOperator* incAndDecExpression
    ;

incAndDecExpression
    : atomicExpression (INC | DEC )?
    ;

postfixExpression
    : atomicExpression
    | postfixExpression callSuffix
    ;

callSuffix
    : LPAREN NL* (valueArgument (NL* COMMA NL* valueArgument)* NL*)? RPAREN
    ;

valueArgument
    : identifier NL* COLON NL* expression
    | expression
    ;

atomicExpression
    : literalConstant
    | identifier 
    | ifExpression
    | loopExpression
    | jumpExpression
    | numericTypeConvExpr
    ;

literalConstant
    : IntegerLiteral
    | FloatLiteral
    | booleanLiteral
    ;

booleanLiteral
    : TRUE
    | FALSE
    ;

ifExpression
    : IF NL* LPAREN NL* expression NL* RPAREN NL* block
    (NL* ELSE (NL* ifExpression | NL* block))?
    ;

loopExpression
    :whileExpression
    | doWhileExpression
    ;

whileExpression
    : WHILE NL* LPAREN NL* expression NL* RPAREN NL* block
    ;

doWhileExpression
    : DO NL* block NL* WHILE NL* LPAREN NL* expression NL* RPAREN
    ;
jumpExpression
    : RETURN (NL* expression)?
    | CONTINUE
    | BREAK
    ;

numericTypeConvExpr
    : numericTypes LPAREN NL* expression NL* RPAREN
    ;

block
    : LCURL expressionOrDeclarations RCURL
    ;
expressionOrDeclarations
    : end* (expressionOrDeclarationAndEnd end*)* expressionOrDeclaration?
    ;

expressionOrDeclarationAndEnd
    : ( expression | varOrfuncDeclaration ) end
    ;

expressionOrDeclaration
    : expression
    | varOrfuncDeclaration
    ;

varOrfuncDeclaration
    : functionDefinition
    | variableDeclaration
    ;

assignmentOperator
    : ASSIGN
    | ADD_ASSIGN
    | SUB_ASSIGN
    | EXP_ASSIGN
    | MUL_ASSIGN
    | DIV_ASSIGN
    | MOD_ASSIGN
    | AND_ASSIGN
    | OR_ASSIGN
    | BITAND_ASSIGN
    | BITOR_ASSIGN
    | BITXOR_ASSIGN
    | LSHIFT_ASSIGN
    | RSHIFT_ASSIGN
    ;

equalityOperator
    : NOTEQUAL
    | EQUAL
    ;

comparisonOperator
    : LT
    | GT
    | LE
    | GE
    ;

shiftingOperator
    : LSHIFT | RSHIFT
    ;

additiveOperator
    : ADD | SUB
    ;

exponentOperator
    : EXP
    ;

multiplicativeOperator
    : MUL
    | DIV
    | MOD
    ;

prefixUnaryOperator
    : SUB
    | NOT
    ;

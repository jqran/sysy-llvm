lexer grammar Lex;

WS
    : [\u0020\u0009\u000C] -> skip
    ;

NL: '\u000A' | '\u000D' '\u000A' ;
DOT: '.' ;
COMMA: ',' ;
LPAREN: '(' ;
RPAREN: ')' ;
LSQUARE: '[' ;
RSQUARE: ']' ;
LCURL: '{' ;
RCURL: '}' ;
EXP: '**' ;
MUL: '*' ;
MOD: '%' ;
DIV: '/' ;
ADD: '+' ;
SUB: '-' ;
PIPELINE: '|>' ;
COMPOSITION: '~>' ;
INC: '++' ;
DEC: '--' ;
AND: '&&' ;
OR: '||' ;
NOT: '!' ;
BITAND: '&' ;
BITOR: '|' ;
BITXOR: '^' ;
LSHIFT: '<<' ;
RSHIFT: '>>' ;
COLON: ':' ;
SEMI: ';' ;
ASSIGN: '=' ;
ADD_ASSIGN: '+=' ;
SUB_ASSIGN: '-=' ;
MUL_ASSIGN: '*=' ;
EXP_ASSIGN: '**=' ;
DIV_ASSIGN: '/=' ;
MOD_ASSIGN: '%=' ;
AND_ASSIGN: '&&=' ;
OR_ASSIGN: '||=' ;
BITAND_ASSIGN: '&=' ;
BITOR_ASSIGN: '|=' ;
BITXOR_ASSIGN: '^=' ;
LSHIFT_ASSIGN: '<<=' ;
RSHIFT_ASSIGN: '>>=' ;

UPPERBOUND: '<:';
LT: '<' ;
GT: '>' ;
LE: '<=' ;
GE: '>=' ;
NOTEQUAL: '!=' ;
EQUAL: '==' ;
WILDCARD: '_' ;
BACKSLASH: '\\' ;
QUOTESYMBOL: '`';
INT8: 'Int8' ;
INT16: 'Int16' ;
INT32: 'Int32' ;
INT64: 'Int64' ;
INTNATIVE: 'IntNative' ;
UINT8: 'UInt8' ;
UINT16: 'UInt16' ;
UINT32: 'UInt32' ;
UINT64: 'UInt64' ;
UINTNATIVE: 'UIntNative' ;
FLOAT16: 'Float16' ;
FLOAT32: 'Float32' ;
FLOAT64: 'Float64' ;
RUNE: 'Rune' ;
BOOLEAN: 'Bool' ;
UNIT: 'Unit' ;
FUNC: 'func';
MAIN: 'main';
LET: 'let' ;
VAR: 'var' ;
CONST: 'const' ;
IF: 'if' ;
ELSE: 'else' ;
FOR: 'for' ;
DO: 'do' ;
WHILE: 'while' ;
RETURN: 'return' ;
CONTINUE: 'continue' ;
BREAK: 'break' ;
TRUE: 'true';
FALSE: 'false';
STATIC: 'static';
IntegerLiteralSuffix
   : 'i8' |'i16' |'i32' |'i64' |'u8' |'u16' |'u32' | 'u64'
   ; 
 
IntegerLiteral
   : BinaryLiteral IntegerLiteralSuffix?
   | OctalLiteral IntegerLiteralSuffix?
   | DecimalLiteral '_'* IntegerLiteralSuffix?
   | HexadecimalLiteral IntegerLiteralSuffix?
   ;
BinaryLiteral
    : '0' [bB] BinDigit (BinDigit | '_')*
    ;
BinDigit
    : [01]
    ;
OctalLiteral
    : '0' [oO] OctalDigit (OctalDigit | '_')*
    ;
OctalDigit
    : [0-7]
    ;
DecimalLiteral
    : (DecimalDigitWithOutZero (DecimalDigit | '_')*) | DecimalDigit
    ;
fragment DecimalFragment
    : DecimalDigit (DecimalDigit | '_')*
    ;
fragment DecimalDigit
    : [0-9]
    ;
fragment DecimalDigitWithOutZero
    : [1-9]
    ;
HexadecimalLiteral
    : '0' [xX] HexadecimalDigits
    ;

fragment HexadecimalDigits
    : HexadecimalDigit (HexadecimalDigit | '_')*
    ;

fragment HexadecimalDigit
    : [0-9a-fA-F]
    ;

FloatLiteralSuffix
    : 'f16' | 'f32' | 'f64'
    ;
 
FloatLiteral
    : (DecimalLiteral DecimalExponent | DecimalFraction DecimalExponent? | (DecimalLiteral DecimalFraction) DecimalExponent?)  FloatLiteralSuffix? 
    | ( Hexadecimalprefix (HexadecimalDigits | HexadecimalFraction | (HexadecimalDigits HexadecimalFraction)) HexadecimalExponent)
    ;

fragment DecimalFraction : '.' DecimalFragment ;
fragment DecimalExponent : FloatE Sign? DecimalFragment  ;
fragment Sign : [-] ;
fragment Hexadecimalprefix : '0' [xX] ;
 
HexadecimalFraction : '.' HexadecimalDigits ;
HexadecimalExponent : FloatP Sign? DecimalFragment ;
fragment FloatE : [eE] ;
fragment FloatP : [pP] ;
Identifier
    : '_'* Letter (Letter | '_' | DecimalDigit)*
    | '`' '_'* Letter (Letter | '_' | DecimalDigit)* '`'
    ;

fragment Letter : [a-zA-Z] ;
lexer grammar Lex;

DelimitedComment
    : '/*' ( DelimitedComment | . )*? '*/'-> skip
    ;

LineComment
    : '//' ~[\u000A\u000D]* -> skip
    ;
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
ARROW: '->' ;
BACKARROW: '<-' ;
DOUBLE_ARROW: '=>' ;
ELLIPSIS: '...' ;
CLOSEDRANGEOP: '..=' ;
RANGEOP: '..' ;
HASH: '#' ;
AT: '@' ;
QUEST: '?' ;
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
DOLLAR: '$';
// QUOTE_OPEN: '"' ;
QUOTE_OPEN_CLOSE: '"' ;
TRIPLE_QUOTE_OPEN: '"""' NL;
// QUOTE_CLOSE: '"' ;
TRIPLE_QUOTE_CLOSE: '"""' ;
// LineStrExprStart: '${' ;
LineStrExprStart: '${' ;
// MultiLineStrExprStart: '${' ;

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
NOTHING: 'Nothing' ;
STRUCT: 'struct' ;
ENUM: 'enum' ;
THISTYPE: 'This';
PACKAGE: 'package' ;
IMPORT: 'import' ;
CLASS: 'class' ;
INTERFACE: 'interface' ;
FUNC: 'func';
MAIN: 'main';
LET: 'let' ;
VAR: 'var' ;
CONST: 'const' ;
TYPE_ALIAS: 'type' ;
INIT: 'init'  ;
THIS: 'this' ;
SUPER: 'super' ;
IF: 'if' ;
ELSE: 'else' ;
CASE: 'case' ;
TRY: 'try' ;
CATCH: 'catch' ;
FINALLY: 'finally' ;
FOR: 'for' ;
DO: 'do' ;
WHILE: 'while' ;
THROW: 'throw' ;
RETURN: 'return' ;
CONTINUE: 'continue' ;
BREAK: 'break' ;
IS: 'is' ;
AS: 'as' ;
IN: 'in' ;
MATCH: 'match' ;
FROM: 'from' ;
WHERE: 'where';
EXTEND: 'extend';
SPAWN: 'spawn';
SYNCHRONIZED: 'synchronized';
MACRO: 'macro';
QUOTE: 'quote';
TRUE: 'true';
FALSE: 'false';
STATIC: 'static';
PUBLIC: 'public' ;
PRIVATE: 'private' ;
PROTECTED: 'protected' ;
OVERRIDE: 'override' ;
REDEF: 'redef' ;
ABSTRACT: 'abstract' ;
OPEN: 'open' ;
OPERATOR: 'operator' ;
FOREIGN: 'foreign';
INOUT: 'inout';
PROP: 'prop';
MUT: 'mut';
UNSAFE: 'unsafe';
GET: 'get';
SET: 'set';
INTERNAL: 'internal';

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
 

// DecimalFraction : '.' DecimalLiteral ;
// DecimalExponent : FloatE Sign? DecimalLiteral  ;
fragment HexadecimalFraction : '.' HexadecimalDigits ;
fragment HexadecimalExponent : FloatP Sign? DecimalFragment ;
fragment FloatE : [eE] ;
fragment FloatP : [pP] ;
// Sign : [-] ;
// Hexadecimalprefix : '0' [xX] ;

RuneLiteral
    : '\'' (SingleChar | EscapeSeq ) '\''
    ;

fragment SingleChar
    :	~['\\\r\n]
    ;

EscapeSeq
    : UniCharacterLiteral
    | EscapedIdentifier
    ;

UniCharacterLiteral
    : '\\' 'u' '{' HexadecimalDigit '}'
    | '\\' 'u' '{' HexadecimalDigit HexadecimalDigit '}'
    | '\\' 'u' '{' HexadecimalDigit HexadecimalDigit HexadecimalDigit '}'
    | '\\' 'u' '{' HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit '}'
    | '\\' 'u' '{' HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit '}'
    | '\\' 'u' '{' HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit '}'
    | '\\' 'u' '{' HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit '}'
    | '\\' 'u' '{' HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit '}'
    ;

EscapedIdentifier
    : '\\' ('t' | 'b' | 'r' | 'n' | '\'' | '"' | '\\' | 'f' | 'v' | '0' | '$')
    ;

ByteLiteral
    : 'b' '\'' (SingleCharByte | ByteEscapeSeq) '\''
    ;
 
ByteEscapeSeq
    : HexCharByte
    | ByteEscapedIdentifier
    ;
 
fragment SingleCharByte
    // ASCII 0x00~0x7F without \n \r \' \" \\
    // +-------+-----+-----+
    // | Rune  | Hex | Dec |
    // +-------+-----+-----+
    // | \n    |  0A |  10 |
    // | \r    |  0D |  13 |
    // | \"    |  22 |  34 |
    // | \'    |  27 |  39 |
    // | \\    |  5C |  92 |
    // +-------+-----+-----+
    : [\u0000-\u0009\u000B\u000C\u000E-\u0021\u0023-\u0026\u0028-\u005B\u005D-\u007F]
    ;
 
fragment ByteEscapedIdentifier
    : '\\' ('t' | 'b' | 'r' | 'n' | '\'' | '"' | '\\' | 'f' | 'v' | '0')
    ;
 
fragment HexCharByte
    : '\\' 'u' '{' HexadecimalDigit '}'
    | '\\' 'u' '{' HexadecimalDigit HexadecimalDigit '}'
    ;
 
ByteStringArrayLiteral
    : 'b' '"' (SingleCharByte | ByteEscapeSeq)* '"'
    ;

JStringLiteral
    : 'J' '"' (SingleChar | EscapeSeq)* '"'
    ;
  
// LineStrText
//     : ~["\\\r\n]
//     | EscapeSeq
//     ;

// TRIPLE_QUOTE_CLOSE
//     : MultiLineStringQuote? '"""' ;

MultiLineStringQuote
    : '"'+
    ;

// MultiLineStrText
//     : ~('\\')
//     | EscapeSeq
//     ;

MultiLineRawStringLiteral
    : MultiLineRawStringContent
    ;

fragment MultiLineRawStringContent
    : HASH MultiLineRawStringContent HASH 
    | HASH '"' .*? '"' HASH
    ;



Identifier
    : '_'* Letter (Letter | '_' | DecimalDigit)*
    | '`' '_'* Letter (Letter | '_' | DecimalDigit)* '`'
    ;

fragment Letter : [a-zA-Z] ;

DollarIdentifier
    : '$' Identifier
    ;

  
LineStrText
    : ~["\\\r\n]
    | EscapeSeq
    ;


MultiLineStrText
    : ~('\\')
    | EscapeSeq
    ;
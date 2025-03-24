#include <algorithm>
#include <memory>
#include<string>
#include<iostream>
#include <sys/types.h>
#ifndef __LEXER__
#define __LEXER__
using std::string,std::cout,std::cin,std::endl,std::unique_ptr,std::make_unique,std::move;
enum tokenType:int{
    LEXEOF=-1,
    ILLEGAL=1,//illegal
    IDENT, //标识符
    INT,//字面量
    FLOAT,
    INT_BIN,
    INT_OCTAL,
    INT_HEX,
    BANG,//!
    ASSIGN,//=
    PLUS,//+
    MINUS,//-
    RIGHT_ARROW,//->
    DOT,//.
    ASTERISK,// *
    SLASH,// /
    MOD,
    OR,//|
    ESPERLUTTE,//&
    QUOTE,//'

    D_QUOTE,//"
    D_ESPERLUTTE,//&&
    D_OR,//||
    EQUAL,//==
    NOTEQUAL,//!=
    LT,// <
    LE,//,=
    GT,//> 
    GE,//>=
    COLON,//:
    COMMA,//,
    SEMICOLON,//;
    LPAREM,//(
    RPAREM,//)
    LSQ_BRACE,//square方括号[
    RSQ_BRACE,//]
    LBRACE,//{
    RBRACE,//}
    KW_THIS,
    KW_INIT,
    KW_MAIN,
    KW_INOUT,
    KW_MACRO,
    KW_PACKAGE,
    KW_PUBLIC,
    KW_PRIVATE,    
    KW_PROTECTED,
    KW_STRUCT,
    KW_RUNE,
    KW_INT64,
    KW_INT32,
    KW_INT16,
    KW_INT8,
    KW_UINT64,
    KW_UINT32,
    KW_UINT16,
    KW_UINT8,
    KW_FLOAT64,
    KW_FLOAT32,
    KW_FLOAT16,
    KW_FUNCTION,//function
    KW_CONST,
    KW_LET,
    KW_VAR,
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_FOR,
    KW_RETURN,
    KW_VOID,
    KW_CONTINUE,
    KW_BREAK,
    KW_FALSE,
    KW_TRUE,
};
struct Pos{
    uint line;
    uint column;
    Pos(uint,uint);
    Pos();
};
struct Token{
    string literal;
    enum::tokenType type;
    // int line;
    // int column;
    Pos begin,end;
    Token(string,enum::tokenType);
    Token(string,enum::tokenType,Pos,Pos);
    // Token(string);
    Token(string,Pos,Pos);
    Token(int,enum::tokenType);
    enum::tokenType lookupIdent();
};
struct Lexer{
    string input;
    size_t position;
    uint line;
    uint column;
    int32_t ch;//当前查看的字符

    //init
    Lexer(string input);
    void skipwhite();
    void skipOther();
    int readChar();
    int peekChar();
    unique_ptr<Token>  nextToken();
    string readStr();
    string readIdentifier();
    string readNumber(tokenType &type);
    
    
};



#endif

#include "frontend/lex.hpp"
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <map>
#include <memory>
Pos::Pos(uint line,uint column):line(line),column(column){
}
Pos::Pos():line(0),column(0){
}
Token::Token(string literal,enum::tokenType type):literal(literal), type(type),begin(),end(){
}
Token::Token(string literal,enum::tokenType type,Pos begin,Pos end):literal(literal), type(type),begin(begin),end(end){
}
Token::Token(int ch,enum::tokenType type): literal(1,ch), type(type),begin(),end(){  
}
// Token::Token(string literal):literal(literal),type(lookupIdent()){
// }
Token::Token(string literal,Pos begin,Pos end):literal(literal),type(lookupIdent()),begin(begin),end(end){
}
enum::tokenType Token::lookupIdent(){
    tokenType ret;
    static const std::map<string const,tokenType>toke_type{
        {"package",tokenType::KW_PACKAGE},
        {"struct",tokenType::KW_STRUCT},
        {"this",tokenType::KW_THIS},
        {"init",tokenType::KW_INIT},
        {"inout",tokenType::KW_INOUT},
        {"public",tokenType::KW_PUBLIC},
        {"private",tokenType::KW_PRIVATE},
        {"protected",tokenType::KW_PROTECTED},
        {"let",tokenType::KW_LET},
        {"const",tokenType::KW_CONST},
        {"var",tokenType::KW_VAR},
        {"func",tokenType::KW_FUNCTION},
        {"if",tokenType::KW_IF},
        {"else",tokenType::KW_ELSE},
        {"while",tokenType::KW_WHILE},
        {"for",tokenType::KW_FOR},
        {"return",tokenType::KW_RETURN},
        {"Int64",tokenType::KW_INT64},
        {"Int32",tokenType::KW_INT32},
        {"Int16",tokenType::KW_INT16},
        {"Int8",tokenType::KW_INT8},
        {"UInt64",tokenType::KW_UINT64},
        {"UInt32",tokenType::KW_UINT32},
        {"UInt16",tokenType::KW_UINT16},
        {"UInt8",tokenType::KW_UINT8},
        {"Float64",tokenType::KW_FLOAT64},
        {"Float32",tokenType::KW_FLOAT32},
        {"Float16",tokenType::KW_FLOAT16},
	{"false",tokenType::KW_FALSE},
	{"true",tokenType::KW_TRUE},
    };
    if(auto iter=toke_type.find(this->literal);iter!=toke_type.end()){
        return iter->second;
    }
    return tokenType::IDENT;
}

Lexer::Lexer(string input) :input(input),position(0),ch(input[0]),line(1),column(1) {}
int Lexer::readChar(){
    ch=peekChar();
    this->position++;
    // this->readPosition++;
    this->column++;
    return ch;
}
int Lexer::peekChar(){
    int ret;
    if(this->position+1>this->input.length()){
        ret=0;
    }else{
        ret=this->input[position+1];      
    }
    return ret;
}

std::unique_ptr<Token>   Lexer::nextToken(/*std::unique_ptr<Lexer> l*/){
    std::unique_ptr<Token> tok=nullptr;
    // bool flagRead=true;
    this->skipOther();
    Pos begin{this->line,this->column};
    // int l1=this->line,c1=this->column;
    switch (this->ch){
    case '|':
        if(this->peekChar()=='|'){
            this->readChar();
            tok=std::make_unique<Token>("||",tokenType::D_OR);
        }else
            tok=std::make_unique<Token>("!",tokenType::BANG);
        break;
    case '&':
        if(this->peekChar()=='&'){
            this->readChar();
            tok=std::make_unique<Token>("&&",tokenType::D_ESPERLUTTE);
        }else
            tok=std::make_unique<Token>("&",tokenType::ESPERLUTTE);
        break;
    case '!':
        if(this->peekChar()=='='){
            this->readChar();
            tok=std::make_unique<Token>("!=",tokenType::NOTEQUAL);
        }
        else
            tok=std::make_unique<Token>("!",tokenType::BANG);
        break;
    case '<':
        if(this->peekChar()=='='){
            this->readChar();
            tok=std::make_unique<Token>("<=",tokenType::LE);
        }
        else
            tok=std::make_unique<Token>("<",tokenType::LT);
        break;
    case '>':
        if(this->peekChar()=='='){
            this->readChar();
            tok=std::make_unique<Token>(">=",tokenType::GE);
        }
        else
            tok=std::make_unique<Token>(">",tokenType::GT);
        break;
    case '=':
        if(this->peekChar()=='='){
            this->readChar();
            tok=std::make_unique<Token>("==",tokenType::EQUAL);
        }
        else
            tok=std::make_unique<Token>("=",tokenType::ASSIGN);
        break;
    case '+':
        tok=std::make_unique<Token>("+",tokenType::PLUS);
        break;
    case '-':
        if(this->peekChar()=='>'){
            this->readChar();
            tok=std::make_unique<Token>("->",tokenType::EQUAL);
        }
        else
            tok=std::make_unique<Token>("-",tokenType::MINUS);
        break;
    case '*':
        tok=std::make_unique<Token>("*",tokenType::ASTERISK);
        break;
    case '/':
        tok=std::make_unique<Token>("/",tokenType::SLASH);
        break;
    case '%':
        tok=std::make_unique<Token>("%",tokenType::MOD);
        break;
    case ',':
        tok=std::make_unique<Token>(",",tokenType::COMMA);
        break;    
    case '.':
        tok=std::make_unique<Token>(".",tokenType::DOT);
        break;
    case ';':
        tok=std::make_unique<Token>(";",tokenType::SEMICOLON);
        break;    
    case ':':
        tok=std::make_unique<Token>(":",tokenType::COLON);
        break;
    case '(':
        tok=std::make_unique<Token>("(",tokenType::LPAREM);  
        break;
    case ')':
        tok=std::make_unique<Token>(")",tokenType::RPAREM);
        break;
    case '[':
        tok=std::make_unique<Token>("[",tokenType::LSQ_BRACE);  
        break;
    case ']':
        tok=std::make_unique<Token>("]",tokenType::RSQ_BRACE);
        break;
    case '{':
        tok=std::make_unique<Token>("{",tokenType::LBRACE);  
        break;
    case '}':
        tok=std::make_unique<Token>("}",tokenType::RBRACE);
        break;
    case 0:
        tok=std::make_unique<Token>("",tokenType::LEXEOF);
        // flagRead=false;
        break;
    default:
        if(this->ch=='\"'||this->ch=='\''){
            auto  ttype=(this->ch=='\"')?D_QUOTE:QUOTE;
	    string s=readStr();
	    tok=std::make_unique<Token>(s,ttype,begin,Pos{this->line,this->column});
        }else if(isalpha(this->ch)||this->ch=='_'){
            string s=readIdentifier();
            tok=std::make_unique<Token>(s,begin,Pos{this->line,this->column});
            // flagRead=false;
            return std::move(tok);
        }else if(isdigit(this->ch)||ch=='.'){
            tokenType type;
            string s{readNumber(type)};
            tok=std::make_unique<Token>(s,type,begin,Pos{this->line,this->column});
            // if(tok->literal[1]=='x'||tok->literal[1]=='X'){
            //     tok->type=INT_HEX;
            // }else if(tok->literal[1]=='b'||tok->literal[1]=='B'){
            //     tok->type=INT_BIN;
            // }else if(tok->literal[0]=='0'){
            //     tok->type=INT_OCTAL;
            // }
            return std::move(tok);
        }else{
            tok=std::make_unique<Token>(this->ch,tokenType::ILLEGAL);
            exit(1);
        }
        
    }
    // if(flagRead){
    //     this->readChar();
    // }
    tok->begin=begin;
    tok->end=Pos{line,column};
    this->readChar();

    return std::move(tok);

}
string Lexer::readStr(){
    char start=ch;
    auto beginpos=this->position;
    size_t sublen=0;
    do{
        readChar();
        sublen++;
    }while(ch!=start);
    readChar();
    ++sublen;
    return this->input.substr(beginpos,sublen);
}
string Lexer::readIdentifier(){
    auto beginpos=this->position;
    size_t sublen=0;
    if(isalpha(this->input[position])||this->input[position]=='_'){
        readChar();
        sublen++;
        while(isalnum(this->input[position])||this->input[position]=='_'){
            readChar();
            sublen++;
        }
    }
    return this->input.substr(beginpos,sublen);
}
void Lexer::skipwhite(){
    while(isspace(this->ch)){ 
        if(this->ch=='\n'){
            this->column=0;
            this->line++;
        }
        readChar();
    
    }
}
void Lexer::skipOther(){
    //单行注释
    this->skipwhite();
    while(ch=='/'&&(peekChar()=='/'||peekChar()=='*')){
        if(peekChar()=='/'){
            while(this->ch!='\n'){ 
                readChar();
                if(this->ch==0){
                    cout<<11<<endl;
                break;
            }
            }
        line++;
        column=0;
        readChar();
        }
        /*多行注释*/
        else if(peekChar()=='*'){
            readChar();
            readChar();
            while(!(ch=='*'&&peekChar()=='/')){
                if(this->ch=='\n'){
                    this->column=0;
                    this->line++;
                }
                readChar();
                if(this->ch==0){
                    cout<<11<<endl;
                    break;
                }
            }
            readChar();
            readChar();
        }
    this->skipwhite();
    }
}
int isodigit(int c){
    if(c>47&&c<'8')
        return 1;
    return 0;
}
int isbdigit(int c){
    if(c=='0'||c=='1')
        return 1;
    return 0;
}
string Lexer::readNumber(tokenType &type){
    int beginpos=this->position;
    int sublen=0;
    // int (*tmpIsDigit)(int)=isdigit;
    type=tokenType::INT;
    int dot_num=0;
    bool front0=false;
    if(this->input[position]=='0'){
        if(this->input[position+1]=='x'||this->input[position+1]=='X'){
            readChar();
            readChar();
            sublen+=2;
            // tmpIsDigit=isxdigit;
            type=tokenType::INT_HEX;
        }else if(this->input[position+1]=='b'||this->input[position+1]=='B'){
            readChar();
            readChar();
            sublen+=2;
            // tmpIsDigit=isbdigit;
            type=tokenType::INT_BIN;
        }else if(isodigit(this->input[position+1])){
            // tmpIsDigit=isodigit;
            readChar();
            ++sublen;
            type=tokenType::INT_OCTAL;
        }
        // else if(this->input[readPosition]=='.'){
        //     tmpIsDigit=isdigit;
        //     type=tokenType::FLOAT;
        // }
    }
    if(type==tokenType::INT_BIN){
        while (isbdigit(ch)) {
            readChar();
            sublen++;
        }
    }else if(type==tokenType::INT_HEX){
        bool hasp=false;
        while (isxdigit(ch)||ch=='p'||ch=='P'||ch=='.'||ch=='-'||ch=='+') {
            if(ch=='p'||ch=='P'){
                type=tokenType::FLOAT;                
                if(hasp){
                    exit(199);
                }
                hasp=true;
                readChar();
                sublen++;
                if(ch=='-'||ch=='+'||isdigit(ch)){
                    do{
                        readChar();
                        sublen++;
                    }while(isdigit(ch));
                    break;
                }else{
                    std::cerr<<"err"<<endl;
                    exit(191);
                }
            }else if (ch=='.') {
                if(dot_num){
                    break;
                }
                ++dot_num;
                readChar();
                sublen++;
            }else if(isxdigit(ch)){
                readChar();
                sublen++;
            }else if (ch=='-'||ch=='+') {
                break;
            }
            else{
                std::cerr<<"err"<<endl;
                exit(191);
            }
        }
    }else if(front0){
        bool has9=false,hase=false;
        while (isdigit(ch)||ch=='.'||ch=='e'||ch=='E'||ch=='-'||ch=='+') {
            if(isdigit(ch)){
                if(ch=='9')
                    has9=true;
                readChar();
                sublen++;
            }else if(ch=='.'){
                //只有1个点
                if(dot_num){
                    break;
                }
                ++dot_num;
                readChar();
                sublen++;
            }else if(ch=='e'||ch=='E'){
                hase=true;
                readChar();
                sublen++;
                if(ch=='-'||ch=='+'||isdigit(ch)){
                    do{
                        readChar();
                        sublen++;
                    }while(isdigit(ch));
                    break;
                }else{
                    std::cerr<<"err"<<endl;
                    exit(191);
                }
            }else if (ch=='-'||ch=='+') {
                break;
            }
            else{
                    std::cerr<<"err"<<endl;
                    exit(191);
            }

        }
        if(has9&&dot_num==0&&!hase){
            std::cerr<<"is not octal"<<endl;
            exit(119);
        }else if(dot_num){
            type=tokenType::FLOAT;
        }else if (hase) {
            type=tokenType::FLOAT;
        }
        else if(!hase&&!has9&&dot_num==0){
            type=tokenType::INT_OCTAL;
        }

    }else{
        while (isdigit(ch)||ch=='.'||ch=='e'||ch=='E'||ch=='-'||ch=='+') {
            if(isdigit(ch)){
                readChar();
                sublen++;
            }else if(ch=='.'){
                if(dot_num){
                    break;
                }
                ++dot_num;
                type=tokenType::FLOAT;
                readChar();
                sublen++;
            }else if(ch=='e'||ch=='E'){
                type=tokenType::FLOAT;
                readChar();
                sublen++;
                if(isdigit(ch)||ch=='-'||ch=='+'){
                    do {
                        readChar();
                        sublen++;
                    }while (isdigit(ch));
                    break;
                }else{
                    std::cerr<<"err"<<endl;
                    exit(117);
                }
            }else if (ch=='-'||ch=='+') {
                break;
            }
            else{
                    std::cerr<<"err"<<endl;
                    exit(191);
            }
        }
    }
    if(ch=='f'||ch=='F'){
        if(type!=tokenType::FLOAT)exit(51);
        readChar();
        sublen++;
    }
    return this->input.substr(beginpos,sublen);
}

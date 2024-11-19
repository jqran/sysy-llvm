#include "frontend/node.hpp"
#include "frontend/type.hpp"
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
//FuncDef f{"1",Pos{1,1}};
#ifndef PARSER__AST
#define PARSER__AST
enum parserOpPrec{
    LOWEST=51,
    OP_ASSIGN,
    OP_DOR,//||
    OP_DESP,//&&
    OP_OR,//|
    OP_ESP,//&
    OP_EQUALS,//==
    OP_LESSGREATER,// > or <
    OP_SUMS,//+
    OP_PRODUCTS,//*
    OP_PREFIX,//-x or !x
    CALL ,//func(x)
};

struct Parser{
    //unique_ptr<ast::SyntaxTree> synatx;
    
    std::unique_ptr<Lexer> lex;
    unique_ptr<Token>  curTok;
    unique_ptr<Token>  peekTok;
    unique_ptr<ast::CompunitNode> comp;
    unique_ptr<ast::ExprNode> (Parser::*prefixFn)();//=&Parser::parserIdentifier;
    unique_ptr<ast::ExprNode> (Parser::*InfixFn)(unique_ptr<ast::ExprNode>);
    Pos cur_pos;
    string file_name;
    
    static const std::map<tokenType, parserOpPrec>precedences;
    type::TypeManager type_man;
    unique_ptr<ast::CompunitNode> parserComp();
    // unique_ptr<ast::Statement> parserStmt();
    void parserStructDecl();
    unique_ptr<ast::DefStmt> parserValDefStmt();
    unique_ptr<ast::ArrDefStmt> parserArrDefStmt(bool ismut);
    unique_ptr<ast::InitializerExpr> parserInitlizer();
    // unique_ptr<ast::ValDeclStmt> parserValDeclStmt(type::Type*);
    unique_ptr<ast::FuncDef> parserFuncStmt();
    unique_ptr<ast::IfStmt> parserIfStmt();
    unique_ptr<ast::ExprNode> parserConst();
    unique_ptr<ast::ExprNode> parserExpr(parserOpPrec prec=parserOpPrec::LOWEST);
    unique_ptr<ast::ExprNode> parserGroupedExpr();
    unique_ptr<ast::ExprNode> parserPrefixExpr();
    unique_ptr<ast::ExprNode> parserInfixExpr(unique_ptr<ast::ExprNode>);
    unique_ptr<ast::ExprNode> parserAssignExpr(unique_ptr<ast::ExprNode>);
    unique_ptr<ast::ExprNode> parserSuffixExpr(unique_ptr<ast::ExprNode>);
    parserOpPrec curPrecedence();
    unique_ptr<ast::WhileStmt> parserWhileStmt();
    unique_ptr<ast::Statement> parserStmts();
    unique_ptr<ast::BlockStmt> parserBlock();
    unique_ptr<ast::BlockStmt>  parserBlockItems( );
    unique_ptr<ast::RetStmt>  parserRetStmt( );
    unique_ptr<ast::ExprNode>  parserLval( );
    void  AddLvalIndex(ast::LvalExpr *lval );
    unique_ptr<ast::CallExpr>  parserCall(unique_ptr<ast::ExprNode> );
    unique_ptr<ast::Statement> parserExprStmt();
    void parserArg(std::vector< unique_ptr<ast::FuncFParam>> &);
    unique_ptr<ast::CompunitNode> getComp();
    type::Type* parserDefType();

    //return current token may return nullptr
    unique_ptr<Token> nextToken();
    void skipIfCurIs(tokenType);
    void ConsumToken(tokenType);
    Parser(std::string );
    [[deprecated]]
    void reParser(string ) ;
    void selectPreFn(tokenType);
    void selectInFn(tokenType);
    bool curTokIs(tokenType type);
    bool peekTokIs(tokenType type);
};

#endif
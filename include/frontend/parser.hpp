#include "frontend/node.hpp"
#include "frontend/type.hpp"
#include <map>
#include <memory>
#include <string>
#include <variant>
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
    HIGHEST,
};

struct Parser{
    //unique_ptr<ast::SyntaxTree> synatx;
    
    std::unique_ptr<Lexer> lex;
    unique_ptr<Token>  curTok;
    unique_ptr<Token>  peekTok;
    unique_ptr<ast::CompunitNode> comp;
    unique_ptr<ast::ExprNode> (Parser::*prefixFn)();//=&Parser::parserIdentifier;
    unique_ptr<ast::ExprNode> (Parser::*InfixFn)(unique_ptr<ast::ExprNode>);
    Pos last_pos_;
    string file_name;
    
    static const std::map<tokenType, parserOpPrec>precedences;
    type::TypeManager type_man;
    unique_ptr<ast::CompunitNode> parserComp();
    // unique_ptr<ast::Statement> parserStmt();
    // void parserStructDecl(ast::StructDeclStmt *const struct_decl);
    unique_ptr<ast::DefStmt> parserMember();
    unique_ptr<ast::DefStmt> parserStructDecl();
    unique_ptr<ast::DefStmt> parserValDefStmt();
    unique_ptr<ast::ArrDefStmt> parserArrDefStmt(bool ismut);
    unique_ptr<ast::InitializerExpr> parserInitlizer();
    // unique_ptr<ast::ValDeclStmt> parserValDeclStmt(type::Type*);
    unique_ptr<Token>  parserFuncTy();
    unique_ptr<ast::FuncDef> parserFuncStmt();
    unique_ptr<ast::FuncDef> parserSpecialFuncStmt();
    // unique_ptr<ast::IfStmt> parserIfStmt();
    std::variant<unique_ptr<ast::ExprNode>,unique_ptr<ast::Statement>> parserStmtExpr();
    unique_ptr<ast::ExprNode> parserBlockExpr();
    unique_ptr<ast::ExprNode> parserIfExpr();
    unique_ptr<ast::ExprNode> parserWhileExpr();
    unique_ptr<ast::ExprNode> parserConst();
    unique_ptr<ast::ExprNode> parserExpr(parserOpPrec prec=parserOpPrec::LOWEST);
    unique_ptr<ast::ExprNode> parserGroupedExpr();
    unique_ptr<ast::ExprNode> parserPrefixExpr();
    unique_ptr<ast::ExprNode> parserInfixExpr(unique_ptr<ast::ExprNode>);
    unique_ptr<ast::ExprNode> parserAssignExpr(unique_ptr<ast::ExprNode>);
    unique_ptr<ast::ExprNode> parserSuffixExpr(unique_ptr<ast::ExprNode>);
    unique_ptr<ast::ExprNode> parserStr();
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
    void inline autoConsumToken(tokenType type){
        if(curTok->type==type)
            nextToken();
    }
    void inline consumToken(tokenType type){ this->skipIfCurIs(type);}
    void consumToken(std::vector<tokenType>&&type);

    Parser(std::string );
    [[deprecated]]
    void reParser(string ) ;
    void selectPreFn(tokenType);
    void selectInFn(tokenType);
    bool curTokIs(tokenType type);
    bool peekTokIs(tokenType type);
};

#endif

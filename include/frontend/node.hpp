#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include "frontend/lex.hpp"
#include "frontend/type.hpp"
using std::cout,std::string,std::vector;

enum class ScopeType:int{
    GLOBAL=1,
    FUNC,
    STRUCT,
    IF,
    ELSE,
    LOOP,
    TRY,
    CATCH,
    EXPR,
};

namespace ast {
class ASTVisitor;

enum ExprType{
    FLOAT_LITERAL,
    INT_LITERAL,
    ASSIGN_EXPR,
    BIN_OP_EXPR,
    REL_OP_EXPR,
    Eq_EXPR,
    OR_EXPR,
    AND_EXPR,
    ARR_USE_EXPR,
    PREFIX,
    CALL_EXPR,
    LVAL_EXPR,
    INITIALIZER,
};
enum StmtType{
    // NULL_STMT,
    // ROOT,
    // VAL_DEF_STMT,
    // VAL_DECL_STMT,
    // FUNSTMT,
    // CONSTSTMT,
    // RETURNSTMT,
    // IF_STMT,
    // WHILE_STMT,
    // BLOCK_STMT,
    // FLOAT_LITERAL,
    // INT_LITERAL,
    // INFIX,
    // PREFIX,
    // SUFFIX,
    // CONTINUE_STMT,
    // BREAK_STMT,
};

enum class BinOp{
    ILLEGAL,
    PlUS,
    MINUS,
    MULTI,
    SLASH,
    MOD,
    ASSIGN,
    DOR,
    DAND,
    EQ,
    NOT_EQ,
    LT,
    LE,
    GT,
    GE,
};
enum class UnOp{
    PLUS='+',
    MINUS='-',
    NOT='!',
};
struct SyntaxNode {
    Pos pos;
    SyntaxNode(Pos);
    // virtual int getType()=0;
    virtual void print(int level=0)=0;
    virtual void accept(ASTVisitor &visitor)=0 ;
};
struct ExprNode: SyntaxNode {
//   public:
//     int line;
//     // 用于访问者模式
    //std::unique_ptr<Token> tok;//记录位置
    type::Type const* ty=nullptr;
    ExprNode(Pos pos);
    virtual int getType()=0;
    virtual void print(int level=0)=0;
    virtual void accept(ASTVisitor &visitor) =0;
};
struct Statement:public SyntaxNode{
    Statement(Pos pos );
    // virtual int getType()=0;
    virtual void print(int level=0)=0;
};
	
struct PrefixExpr:public ExprNode{
    UnOp operat;//type
    unique_ptr<ExprNode> rhs;
    PrefixExpr(Pos pos);
    virtual int getType()override;
    virtual void print(int level=0)override;
    virtual void accept(ASTVisitor &visitor) override final;
};
// struct ArrUse:public ExprNode{
//     unique_ptr<ExprNode> Lval_name;
//     vector<unique_ptr<ExprNode>> index_num;
//     ArrUse(Pos pos);
//     virtual int getType()override;
//     virtual void print(int level=0)override;
//     virtual void accept(ASTVisitor &visitor)  final;
// };
struct BlockExpr:public ExprNode{
    std::vector<unique_ptr<Statement>>stmts_;
    unique_ptr<ExprNode> expr_;
    ScopeType scope_ty_;
    BlockExpr(Pos pos,ScopeType);
    ~BlockExpr();
    virtual int getType(){}
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor);
};

struct IfExpr:public ExprNode{
    unique_ptr<ExprNode> cond_;
    unique_ptr<ExprNode> then_,else_;
    IfExpr(Pos pos);
    ~IfExpr();
	virtual int getType(){}
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor);
};
struct WhileExpr :public  ExprNode{   
    unique_ptr<ExprNode>cond_;
    unique_ptr<BlockExpr>loop_;
    WhileExpr(Pos Pos);
    ~WhileExpr();
	virtual int getType(){}
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};

struct InfixExpr:public ExprNode{
    BinOp operat;
    unique_ptr<ExprNode> rhs;
    unique_ptr<ExprNode> lhs;
    InfixExpr(Pos pos ,unique_ptr<ExprNode> lhs);
    ~InfixExpr();
    virtual int getType()=0;
    virtual void print(int level=0)=0;
    virtual void accept(ASTVisitor &visitor)=0;

};
struct AssignExpr:public InfixExpr{
    AssignExpr(Pos pos ,unique_ptr<ExprNode> lhs);
    virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct RelopExpr:public InfixExpr{
    RelopExpr(Pos pos ,unique_ptr<ExprNode> lhs);
    virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct EqExpr:public InfixExpr{
    EqExpr(Pos pos ,unique_ptr<ExprNode> lhs);
    virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct AndExp:public InfixExpr{
    AndExp(Pos pos ,unique_ptr<ExprNode> lhs);
    virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct ORExp:public InfixExpr{
    ORExp(Pos pos ,unique_ptr<ExprNode> lhs);
    virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct BinopExpr:public InfixExpr{
    BinopExpr(Pos pos ,unique_ptr<ExprNode> lhs);
    virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
///////中缀表达式/////
// union valUnion{
//     float f;
//     int   i;
// };
enum class LitType{
    INT=1,
    INT_BIN,
    INT_OCTAL,
    INT_HEX,
    FLOAT,
    BOOL,
    DOUBLE,
    STRING,
    RUNE,
};
struct Literal:public ExprNode{
    unique_ptr<Token> literal;
    LitType type;   // 字面量类型，包括整数、浮点数、字符串等
    // Bad   bool    // true means the literal Value has syntax errors
    // expr
    // valUnion Value;
    Literal(Pos pos,unique_ptr<Token>,LitType type);
    virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor) ;
};
// struct IntConst:public Literal{
//     IntConst(Pos pos,valUnion);
//     virtual int getType();
//     virtual void print(int level=0);
//     virtual void accept(ASTVisitor &visitor)  final;
// };
// struct FloatConst:public Literal{
//     // FloatConst(Pos pos,valUnion);
//     virtual int getType();
//     virtual void print(int level=0);
//     virtual void accept(ASTVisitor &visitor)  final;
// };

struct InitializerExpr:public ExprNode{
    InitializerExpr(Pos pos);
    vector<unique_ptr<ExprNode>> initializers;
    virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct CallExpr:public ExprNode{
    // string name;//type
    unique_ptr<ExprNode> call_name;
    vector<unique_ptr<ast::ExprNode>> func_r_params;
    CallExpr(Pos pos);
    CallExpr(Pos pos,unique_ptr<ExprNode> call_name);
    virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};

// enum  struct SufOp{
//     DOT='.',
//     ARROW='-',
//     CALL='(',
//     ARRAY='[',
// };
struct SelectorExpr:public ExprNode{
    // SufOp operat;
    unique_ptr<ExprNode> lhs;
    unique_ptr<Token> rhs;
    SelectorExpr(Pos pos ,unique_ptr<ExprNode> lhs);
    virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor);
};

struct LvalExpr:public ExprNode{
    string name;
    // unique_ptr<ExprNode> expr;
    LvalExpr(Pos pos,string name);
    vector<unique_ptr<ExprNode>> index_num;
    // LvalExpr(string name ,Pos pos,ValType,unique_ptr<ExprNode>);
    // virtual void print();
    virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;

};
struct ExprStmt:public Statement{
    unique_ptr<ast::ExprNode> expr;
    ExprStmt(Pos pos );
    ExprStmt(Pos pos,unique_ptr<ExprNode> );
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor);
};
struct AssignStmt:public Statement{
    unique_ptr<ast::ExprNode> l_val;
    unique_ptr<ast::ExprNode> expr;
    AssignStmt(Pos pos,unique_ptr<ast::ExprNode> lval,unique_ptr<ast::ExprNode> expr);
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor);
};
struct BreakStmt:public Statement{
    BreakStmt(Pos pos );
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct ContinueStmt:public Statement{
    ContinueStmt(Pos pos );
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct BlockStmt :public  Statement{
    vector<unique_ptr<Statement>> block_items;
    BlockStmt(Pos pos );
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
//抽象类
struct DefStmt:public Statement{
    string name;
    // type::Type* type;//变量类型
    // unique_ptr<Token> type;
    unique_ptr<Token> type;
    DefStmt (string name ,Pos pos,unique_ptr<Token>);
    // virtual int getType()=0;
    virtual void print(int level=0)=0;
    virtual void accept(ASTVisitor &visitor)=0;
};

struct CompunitNode : public SyntaxNode
{
    vector<std::unique_ptr<DefStmt>> global_defs;
    bool isReDef(string s);
    CompunitNode();
    ~CompunitNode();
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor);
};
/*语法树*/
struct stynaxTree{
    unique_ptr<CompunitNode >root;
};


/*函数声明*/
// struct FuncStmt :public  DefStmt
// {   
//     //在父类里
//     // string name;
//     // ValType val_type;//变量类型    
//     // FuncStmt(string name ,Pos pos,type::Type* );
//     using DefStmt::DefStmt;
//     // virtual int getType()=0;
//     virtual void print(int level=0)=0;
//     virtual void accept(ASTVisitor &visitor) =0;
// };
struct FuncFParam:public DefStmt{
    //在父类里
    // string name;
    // ValType val_type;//变量类型
    //nullptr if empty in [ ] 如果[]中为空则为nullptr
    vector<unique_ptr<ExprNode>> index_num;
    using DefStmt::DefStmt;
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};

/*函数定义*/
struct FuncDef :public  DefStmt
{   
    //在父类里
    // string name;
    // ValType val_type;//变量类型
    unique_ptr<BlockExpr> body;
    std::vector<std::pair<string,unique_ptr<Token>>>  func_f_params;
    // FuncDef(string name ,Pos pos);
    // FuncDef(string name ,Pos pos,type::Type* );
    using DefStmt::DefStmt;
    ~FuncDef();
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor);

    // bool isReDef(string tok_name);
};

struct [[deprecated]] ValDeclStmt :public  Statement
{   
    type::Type* all_type;
    vector<unique_ptr<DefStmt>> var_def_list;
    //ValDeclStmt(string name ,Pos pos,ValType);
    //vector<unique_ptr<int>> body;
    ValDeclStmt(Pos pos);
    ValDeclStmt(Pos pos,type::Type* type);
    ~ValDeclStmt();
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  ;

};
struct ValDefStmt :public  DefStmt
{   
    unique_ptr<ExprNode> init_expr;
    bool ismut;
    ValDefStmt(string name ,Pos pos,unique_ptr<Token>type,bool ismut);
    ValDefStmt(string name ,Pos pos,unique_ptr<Token>type,bool ismut,unique_ptr<ExprNode>);
    ~ValDefStmt();
    //vector<unique_ptr<int>> body;
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  ;
};
struct ArrDefStmt :DefStmt
{
    //每行长度
    vector<unique_ptr<ExprNode>> array_length; // nullptr for non-array variables
    unique_ptr<InitializerExpr> initializers;//初始化列表
    //不知道有什么意义
    vector<int> initializers_index;
    // ArrDefStmt(string name ,Pos pos,type::Type*);
    using DefStmt::DefStmt;
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  ;
};
struct [[deprecated]] ConstDeclStmt :public  ValDeclStmt
{   //数据在父类里
    ConstDeclStmt(Pos pos);
    ConstDeclStmt(Pos pos,type::Type* type);
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;

};
struct [[deprecated]] ConstDefStmt :public  ValDefStmt
{   //数据在父类里
    // ConstDefStmt(string name ,Pos pos,type::Type*);
    // ConstDefStmt(string name ,Pos pos,type::Type*,unique_ptr<ExprNode>);
    using ValDefStmt::ValDefStmt;
    //vector<unique_ptr<int>> body;
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
struct [[deprecated]] ConstArrDefStmt :ArrDefStmt
{   //数据在父类里
    // ConstArrDefStmt(string name ,Pos pos,type::Type*);
    using ArrDefStmt::ArrDefStmt;
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};
// struct IfStmt :public  Statement
// {   
//     unique_ptr<ExprNode>pred;
//     //可能是一个语句，也可能是一个block
//     unique_ptr<Statement> then_stmt;
//     unique_ptr<Statement> else_stmt;
//     IfStmt(Pos Pos);
//     ~IfStmt();
//     // virtual int getType();
//     virtual void print(int level=0);
//     virtual void accept(ASTVisitor &visitor)  final;

// };
struct WhileStmt :public  Statement
{   
    unique_ptr<ExprNode>pred;
    unique_ptr<Statement>loop_stmt;
    WhileStmt(Pos Pos);
    ~WhileStmt();
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;

};
struct RetStmt :public  Statement
{   
    unique_ptr<ExprNode> expr;
    //vector<unique_ptr<int>> body;
    RetStmt(Pos pos);
    ~RetStmt();
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;

};
struct EmptyStmt :public  Statement
{   
    //vector<unique_ptr<int>> body;
    EmptyStmt(Pos pos);
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};


struct StructDeclStmt;
struct MemberFunc :public  FuncDef
{   
    StructDeclStmt* parent;
    using FuncDef::FuncDef;
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
    // bool isReDef(string tok_name);
};
struct Member :public  ValDefStmt
{   
    StructDeclStmt * parent;
    using ValDefStmt::ValDefStmt;
    // virtual int getType();
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
    // bool isReDef(string tok_name);
};

struct StructDeclStmt:DefStmt{
    vector<string>field;
    unique_ptr<FuncDef>init;
    vector<std::pair<unique_ptr<DefStmt>,type::perm>> members;    
    using DefStmt::DefStmt;
    virtual void print(int level=0);
    virtual void accept(ASTVisitor &visitor)  final;
};





class ASTVisitor
{
public:
    virtual void visit(CompunitNode &node) = 0;
    virtual void visit(FuncFParam &node) = 0;
    virtual void visit(FuncDef &node) = 0;
    virtual void visit(StructDeclStmt&node) = 0;

    // virtual void visit(ValDeclStmt &node) = 0;
    virtual void visit(ValDefStmt &node) = 0;
    virtual void visit(ArrDefStmt &node) = 0;
    // virtual void visit(ConstDeclStmt &node) = 0;
    // virtual void visit(ConstDefStmt &node) = 0;
    // virtual void visit(ConstArrDefStmt &node) = 0;
    virtual void visit(ExprStmt &node) = 0;
    virtual void visit(AssignStmt &node) = 0;
    virtual void visit(PrefixExpr &node) = 0;
    virtual void visit(SelectorExpr &node) = 0;
    // virtual void visit(InfixExpr &node) = 0;
    virtual void visit(AssignExpr &node) = 0;
    virtual void visit(RelopExpr &node) = 0;
    virtual void visit(EqExpr &node) = 0;
    virtual void visit(AndExp &node) = 0;
    virtual void visit(ORExp &node) = 0;
    virtual void visit(BinopExpr &node) = 0;
    virtual void visit(LvalExpr &node) = 0;
    virtual void visit(Literal &node) = 0;

    // virtual void visit(IntConst &node) = 0;
    virtual void visit(InitializerExpr &node) = 0;
    // virtual void visit(FloatConst &node) = 0;
    // virtual void visit(AssignStmt &node) = 0;
    virtual void visit(BlockStmt &node) = 0;
    // virtual void visit(IfStmt &node) = 0;
    virtual void visit(WhileStmt &node) = 0;
    virtual void visit(BlockExpr &node) = 0;
    virtual void visit(IfExpr &node) = 0;
    virtual void visit(WhileExpr &node) = 0;
    virtual void visit(CallExpr &node) = 0;
    virtual void visit(RetStmt &node) = 0;
    virtual void visit(ContinueStmt &node) = 0;
    virtual void visit(BreakStmt &node) = 0;
    virtual void visit(EmptyStmt &node) = 0;
};


  
}

#endif

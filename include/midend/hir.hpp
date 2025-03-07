#ifndef HIR_HPP
#define HIR_HPP
#include "frontend/node.hpp"
#include "frontend/type.hpp"
#include <memory>
#include <string>
#include <vector>
namespace cjir {
struct Node{
  virtual void print(int lv)=0;
};
struct Stmt:public Node{
  virtual void print(int lv)=0;
};
struct Expr:public Node{
  type::Type const * ty;
  Expr():ty(nullptr){}
  Expr(type::Type const*ty):ty(ty){}
  ~Expr()=default;
  virtual void print(int lv)=0;
};
struct ExprStmt:public Stmt{
  unique_ptr<Expr>expr_;
  ExprStmt(unique_ptr<Expr>expr):expr_(std::move(expr)){}
  virtual void print(int lv);
};
struct Lval:public Expr{
  string id_;
  Lval(type::Type const* ty,string id):Expr(ty),id_(id){}
  virtual void print(int lv);
};
struct Bin:public Expr{
  enum Binop{
    ILLEGAL,
    PlUS,
    MINUS,
    MULTI,
    SLASH,
    MOD,
    ASSIGN,
    LOR,//logic
    LAND,
    EQ,
    NOT_EQ,
    LT,
    LE,
    GT,
    GE,
  }op;
  unique_ptr<Expr> lhs_,rhs_;
  Bin(Binop op,unique_ptr<Expr> lhs,unique_ptr<Expr> rhs):op(op),lhs_(std::move(lhs)),rhs_(std::move(rhs)),Expr(){}
  virtual void print(int lv);
};
struct And:public Bin{
  using Bin::Bin;
  using Bin::print;
};
struct Or:public Bin{
  using Bin::Bin;
  using Bin::print;
};

struct Block:public Expr{
  std::vector<unique_ptr<Stmt>>stmts_;
  unique_ptr<Expr> expr;
  Block(type::Type const*ty):Expr(ty){}
  Block():Expr(){}
  virtual void print(int lv);
};
struct If:public Expr{
  unique_ptr<Expr> cond_;
  unique_ptr<Block> then_,else_;
  using Expr::Expr;
  If(type::Type const*ty,unique_ptr<Expr>cond,unique_ptr<Block>then,unique_ptr<Block>el=nullptr):Expr(ty),cond_(std::move(cond)),then_(std::move(then)),else_(std::move(el)){}
  virtual void print(int lv);
};
struct While:public Expr{
  unique_ptr<Expr> cond_;
  unique_ptr<Block> loop_;
  using Expr::Expr;
  While(type::Type const*ty,unique_ptr<Expr>cond,unique_ptr<Block>loop):Expr(ty),cond_(std::move(cond)),loop_(std::move(loop)){}
  virtual void print(int lv);
};
// enum CJINSID:int {

// };
// struct CJConst:public CJValue{
// };
// struct CJConstInt:public CJConst{
//   type::Type* type;
//   ::std::string val;
// };
// struct CJConstFp:public CJConst{
//   type::Type* type;
//   ::std::string val;
// };
// // struct CJConstLiter:public CJConst{
// };
struct Lit:public Expr{
  ::std::string lit_;
  Lit(string lit,type::Type const *const ty):Expr(ty),lit_(lit){}
  virtual void print(int lv);
};
// struct BinExpr:public Expr{
//   enum  binop:uint{
//     add=1,sub,mul,div,rem,land,lor,
//     eq,ne,gt,ge,lt,le,cand,cor,
//   }op;
//   unique_ptr<Expr> lhs,rhs;
//   BinExpr(binop op,unique_ptr<Expr> lhs,unique_ptr<Expr> rhs):op(op),lhs(std::move(lhs)),rhs(std::move(rhs)),Expr(){}
// };
struct Assign:public Stmt{
  unique_ptr<Expr> lval_;
  unique_ptr<Expr>expr_;
  virtual void print(int lv);
};
struct Decl:public Stmt{
  string name_;
  type::Type const * const ty_;
  Decl(type::Type const * const ty,string name):Stmt(),ty_(ty),name_(name){}
  virtual void print(int lv)=0;
};
struct VarDecl:public Decl{
  unique_ptr<Expr> init_;
  using Decl::Decl;
  VarDecl(type::Type const * const ty,string name,unique_ptr<Expr> init):Decl(ty,name),init_(std::move(init)){}
  virtual void print(int lv);
};

  struct Func:public Decl{
  type::Type const* func_ty;
  std::vector<std::pair<unique_ptr<Expr>,type::Type const*>>param_;
  unique_ptr<Block> block_;
  Expr* getRet(){return block_->expr.get();}
    Func(type::Type const*const ret_ty,std::string name):Decl(ret_ty,name),block_(std::make_unique<Block>(ret_ty)){}
  virtual void print(int lv);
};
struct StructDecl;
struct MemVar:public VarDecl{
    StructDecl* parent_;
    MemVar(type::Type const * const ty,string name,unique_ptr<Expr> init):VarDecl(ty ,name,std::move(init)){}
    using VarDecl::print;
};
struct MemFunc:public Func{
    StructDecl* parent_;
    using Func::Func;
    using Func::print;
};
struct StructDecl:public Decl{
  std::vector<unique_ptr<MemVar>> vars_;
  std::vector<unique_ptr<MemFunc>> funcs_;
  using Decl::Decl;
  // VarDecl(type::Type const * const ty,string name,unique_ptr<Expr> init):Decl(ty,name),init_(std::move(init)){}
  virtual void print(int lv);
};

// struct Struct:public Expr{
//   std::vector<std::pair<string,Expr*>> val;
// };
struct Module:public Node{
  std::vector<unique_ptr<Stmt>>defs_;
  virtual void print(int lv);
};

};
#endif

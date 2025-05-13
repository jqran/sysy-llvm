#ifndef HIR_HPP
#define HIR_HPP
#include "midend/scope.hpp"
#include "midend/type.hpp"
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>
using std::unique_ptr;
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

namespace chir {
struct HirVisitor;
struct Node{
    virtual void print(int lv)=0;
    virtual void accept(HirVisitor&visitor)=0;
};
struct Stmt:public Node{
    virtual void print(int lv)=0;
    virtual void accept(HirVisitor&visitor)=0;
};
struct Expr:public Node{
    type::Type const * ty_;
    Expr():ty_(nullptr){}
    Expr(type::Type const*ty):ty_(ty){}
    ~Expr()=default;
    virtual void print(int lv)=0;
    virtual void accept(HirVisitor&visitor)=0;
    virtual void check(type::TypeManager& tyman,type::Type const* ty)=0;
};
struct ExprStmt:public Stmt{
    bool ret_;
    unique_ptr<Expr>expr_;
    ExprStmt(unique_ptr<Expr>expr,bool ret_):expr_(std::move(expr)),ret_(ret_){}
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
};
struct IncDec:public Expr{
    bool is_inc_;
    unique_ptr<::chir::Expr> expr_;
    IncDec(bool is_,unique_ptr<Expr> expr):Expr(expr->ty_),is_inc_(is_),expr_(std::move(expr)){}
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
};

struct Unary:public Expr{
    enum class UnOp{
        // PLUS='+',
        MINUS='-',
        NOT='!',
    }op;
    unique_ptr<::chir::Expr> rhs_;
    Unary(UnOp op,unique_ptr<Expr> rhs):op(op),rhs_(std::move(rhs)),Expr(){}
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
};

struct Bin:public Expr{
    enum Binop{
	ILLEGAL,
	ADD,
	SUB,
	MULTI,
	SLASH,
	MOD,
	ASSIGN,
	LOR,//logic
	LAND,
    BIT_OR,
    BIT_XOR,
    BIT_AND,
	EQ,
	NOT_EQ,
	LT,
	LE,
	GT,
	GE,
    }op;
    unique_ptr<Expr> lhs_,rhs_;
    Bin(Binop op,unique_ptr<Expr> lhs,unique_ptr<Expr> rhs);
protected:
    Bin(type::Type const* ty,Binop op,unique_ptr<Expr> lhs,unique_ptr<Expr> rhs):Expr(ty),op(op),lhs_(std::move(lhs)),rhs_(std::move(rhs)){}
public:
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
};
struct Rel:public Bin{
    Rel(type::Type const* ty,Binop op,unique_ptr<Expr> lhs,unique_ptr<Expr> rhs);
    using Bin::print;
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
};    
struct And:public Bin{
    And(type::Type const*ty,unique_ptr<Expr> lhs,unique_ptr<Expr> rhs);
    using Bin::print;
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
};
struct Or:public Bin{
    Or(type::Type const*ty, unique_ptr<Expr> lhs,unique_ptr<Expr> rhs);
    using Bin::print;
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
};

struct Block:public Expr{
    std::vector<unique_ptr<Stmt>>stmts_;
    unique_ptr<Expr> expr;
    ScopeType bty_;
    // Block(type::Type const*ty,ScopeType bty=ScopeType::EXPR):Expr(ty),bty_(bty){}
    Block(std::vector<unique_ptr<Stmt>> stmts,ScopeType bty=ScopeType::EXPR):Expr(),bty_(bty),stmts_(std::move(stmts)){
        if(this->stmts_.empty()){
            this->ty_=nullptr;
        }else{
            auto back=stmts_.back().get();
            if(auto expr=dynamic_cast<ExprStmt*>(back)){
                if(expr->ret_){
                    this->ty_=expr->expr_->ty_;
                }
            }
        }
    }
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
};
struct If:public Expr{
    unique_ptr<Expr> cond_;
    unique_ptr<Expr> then_,else_;
    using Expr::Expr;
    If(unique_ptr<Expr>cond,unique_ptr<Expr>then,unique_ptr<Expr>el=nullptr);
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
};
struct Jump:public Expr{
    bool is_continue_;
    Jump(type::Type const*ty,bool is_continue):Expr(ty),is_continue_(is_continue){}
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
};
struct Ret:public Expr{
    unique_ptr<Expr> expr_;
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
    Ret(unique_ptr<Expr>expr):Expr(nullptr),expr_(std::move(expr)){}
};

struct NumConv:public Expr{

    unique_ptr<::chir::Expr> expr_;
    NumConv(type::Type const* ty,unique_ptr<Expr> expr_):Expr(ty),expr_(std::move(expr_)){}
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
};

struct While:public Expr{
    unique_ptr<Expr> cond_;
    unique_ptr<Block> loop_;
    using Expr::Expr;
    While(type::Type const*ty,unique_ptr<Expr>cond,unique_ptr<Block>loop):Expr(ty),cond_(std::move(cond)),loop_(std::move(loop)){}
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
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
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
};
struct ArrayLit:public Expr{
    std::vector<unique_ptr<Expr>> elements_;
    bool all_lit_=false;
    ArrayLit(type::Type const *const ty):Expr(ty){}
    ArrayLit(std::vector<unique_ptr<Expr>> elements);
    ArrayLit(type::Type const *const ty,std::vector<unique_ptr<Expr>> elements);
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
};
// struct BinExpr:public Expr{
//   enum  binop:uint{
//     add=1,sub,mul,div,rem,land,lor,
//     eq,ne,gt,ge,lt,le,cand,cor,
//   }op;
//   unique_ptr<Expr> lhs,rhs;
//   BinExpr(binop op,unique_ptr<Expr> lhs,unique_ptr<Expr> rhs):op(op),lhs(std::move(lhs)),rhs(std::move(rhs)),Expr(){}
// };
struct Assign:public Bin{
    Assign(unique_ptr<Expr> lhs,unique_ptr<Expr> rhs);
    using Bin::print;
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
};

struct Decl:public Node{
    string name_;
    type::Type const *  ty_;
    Decl(type::Type const * const ty,string name):Node(),ty_(ty),name_(name){}
    virtual void print(int lv)=0;
    virtual void accept(HirVisitor&visitor)=0;
    virtual void check(type::TypeManager& tyman)=0;
};
struct VarDecl:public Decl{
    DefTy mod_;
    unique_ptr<Expr> init_;
    using Decl::Decl;
    VarDecl(type::Type const * const ty,string name,unique_ptr<Expr> init,DefTy mod_):Decl(ty,name),init_(std::move(init)),mod_(mod_){}
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman);    
};
struct WCDecl:public Decl{
    unique_ptr<Expr> init_;
    using Decl::Decl;
    WCDecl(unique_ptr<Expr> init):Decl(init->ty_,"_"),init_(std::move(init)){}
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman);    
};
// struct RetExpr:public Expr{
//     unique_ptr<Expr> expr_;
// };

struct DeclStmt:public Stmt{
    unique_ptr<Decl>decl_;
    DeclStmt(unique_ptr<Decl>decl):decl_(std::move(decl)){}
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
};
    
struct RetStmt:public Stmt{
    unique_ptr<Expr> expr_;
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    RetStmt(unique_ptr<Expr>expr):expr_(std::move(expr)){}
};

    
struct FuncDecl:public Decl{
    // type::Type const* func_ty;
    std::vector<std::pair<string,type::Type const*>>params_;
    type::Type const* ret_ty_;
    unique_ptr<Block> block_;
    std::vector<Ret*> rets_;
    // Expr* getRet(){return block_->expr.get();}
    FuncDecl( std::string name,std::vector<std::pair<string,type::Type const*>>params):Decl(nullptr,name),ret_ty_(nullptr),params_(std::move(params)){}
    FuncDecl(type::TypeManager*tyman, type::Type const*const ret_ty,std::string name,std::vector<std::pair<string,type::Type const*>>params):Decl(nullptr,name),ret_ty_(ret_ty),params_(std::move(params)){
        vector<type::Type const *> paramtys{};
        // for(auto &iter:params_){
        //     if(params.empty()){assert(0);}
        //     paramtys.push_back(iter.second);
        // }
        size_t size=params_.size();
        for(auto i=0;i<size;++i){
            paramtys.push_back(params_[i].second);
        }
        this->ty_=tyman->getFuncTy(ret_ty, paramtys);
    }
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman);
};

struct StructDecl;


struct MemVar:public VarDecl{
    StructDecl* parent_;
    using VarDecl::VarDecl;
    // MemVar(type::Type const * const ty,string name,unique_ptr<Expr> init):VarDecl(ty ,name,std::move(init)){}
    using VarDecl::print;
    using VarDecl::check;
    virtual void accept(HirVisitor&visitor);
};
struct MemFunc:public FuncDecl{
    StructDecl* parent_;
    string id_;
    using FuncDecl::FuncDecl;
    using FuncDecl::print;
    using FuncDecl::check;
    virtual void accept(HirVisitor&visitor);
};
struct Init:public MemFunc{
    // type::Type const* func_ty;
    using MemFunc::MemFunc;
    using MemFunc::print;
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman);
};
struct StructDecl:public Decl{
    std::vector<unique_ptr<VarDecl>> vars_;
    std::vector<unique_ptr<FuncDecl>> funcs_;
    std::vector<unique_ptr<FuncDecl>> inits_;
    using Decl::Decl;
    ssize_t findVarOffset(string name)const{
        for(ssize_t i=0;i<vars_.size();++i){
            if(vars_[i]->name_==name)
            return i;
        }
        return (ssize_t)(-1);
    }
    VarDecl *findVar(string name)const{
        for(auto &v:vars_){
            if(v->name_==name){
                return v.get();
            }
        }
        return nullptr;
    }
    FuncDecl* findFunc(string name)const{
        for(ssize_t i=0;i<funcs_.size();++i){
            if(funcs_[i]->name_==name)
            return funcs_[i].get();
        }
        return nullptr;
    }
    Decl* find(string name)const{
        Decl* decl=findVar(name);
        if(decl==nullptr){
            return findFunc(name);
        }else{
            return decl;
        }
    }
    // VarDecl(type::Type const * const ty,string name,unique_ptr<Expr> init):Decl(ty,name),init_(std::move(init)){}
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman);
};

struct EnumDecl:public Decl{
    std::vector<std::pair<string, vector<type::Type const*>>> contexts_;
    std::vector<unique_ptr<FuncDecl>> funcs_;
    // std::vector<unique_ptr<FuncDecl>> inits_;
    using Decl::Decl;
    uint getTagSize()const{
        if(contexts_.size()<=2){
            return 1;
        }else if(contexts_.size()-1<=UINT8_MAX){
            return 8;
        }else if(contexts_.size()-1<=UINT16_MAX){
            return 16;
        }else if(contexts_.size()-1<=UINT32_MAX){
            return 32;
        }else{
            return 64;
        }
    }
    // VarDecl(type::Type const * const ty,string name,unique_ptr<Expr> init):Decl(ty,name),init_(std::move(init)){}
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman);
};

struct Lval:public Expr{
    string id_;
    // Decl *const  decl_;
    // Lval(string id,Decl*decl):Expr(decl->ty_),id_(id),decl_(decl){assert(this->ty_!=nullptr);}
    // Lval(Decl*decl):Expr(decl->ty_),id_(decl->name_),decl_(decl){assert(this->ty_!=nullptr);}
    Lval(type::Type const* ty,string id):Expr(ty),id_(id){}
    Lval(string id,type::Type const* ty):Expr(ty),id_(id){}
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
};
struct ThisSuper:public Expr{
    StructDecl*struct_decl_;
    bool is_this_;
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
    ThisSuper(StructDecl*struc,bool isthis):Expr(struc->ty_),struct_decl_(struc),is_this_(isthis){}
};
struct ArrIndex:public Expr{
    type::Type const * arr_ty_;
    unique_ptr<Expr> lhs_;
    vector<unique_ptr<Expr>>indexs_;
    // Decl *const  decl_;
    // Lval(string id,Decl*decl):Expr(decl->ty_),id_(id),decl_(decl){assert(this->ty_!=nullptr);}
    // Lval(Decl*decl):Expr(decl->ty_),id_(decl->name_),decl_(decl){assert(this->ty_!=nullptr);}
    bool isleft_;
    ArrIndex(type::Type const* ty,unique_ptr<Expr> lhs,unique_ptr<Expr> index,bool isleft_):arr_ty_(ty),lhs_(std::move(lhs)),indexs_(),isleft_(isleft_){
        indexs_.push_back(std::move(index));
        if(ty->isArray()){
            auto arrty=(type::ArrayType const*  )ty;
            this->ty_=arrty->element_;
        }else{
            assert(0);
        }
    }
    void addIndex(unique_ptr<Expr> index){
        if(ty_->isArray()){
            auto arr=(type::ArrayType const *)ty_;
            ty_=arr->element_;
        }else{assert(0);}
        indexs_.push_back(std::move(index));
    }
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
};
struct Selector:public Expr{
    unique_ptr<Expr>lhs_;
    StructDecl*struct_;
    bool isleft_;
    string rhs_;
    Selector(unique_ptr<Expr>lhs,string rhs,bool isleft_):lhs_(std::move(lhs)),rhs_(rhs),isleft_(isleft_){
        type::Type const* ty=nullptr;
        if(auto lval=dynamic_cast<Lval*>(lhs_.get())){
            auto ty=lval->ty_;
            assert(ty->type==type::TypeId::STRUCT);
            struct_=((type::StructType*)(ty))->struct_;
        }else if(auto _this=dynamic_cast<ThisSuper*>(lhs_.get())){
            auto ty=_this->ty_;
            assert(ty->type==type::TypeId::STRUCT);
            struct_=_this->struct_decl_;
        }else if(auto selector=dynamic_cast<Selector*>(lhs_.get())){
            auto ty=selector->ty_;
            assert(ty->type==type::TypeId::STRUCT);
            auto decl=selector->struct_->find(selector->rhs_);
            assert(decl->ty_->isStruct()==true);
            this->struct_=((type::StructType const*)(decl->ty_))->struct_;
        }else{

            //this_super
            assert(0);
        }
        if(struct_->findVar(rhs_)==nullptr){
            auto f=struct_->findFunc(rhs_);
            this->ty_=f->ty_;
        }else{
            auto v=struct_->vars_[struct_->findVarOffset(rhs_)].get();
            this->ty_=v->ty_;
        }// assert(v!=nullptr||f!=nullptr);
    }
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
};
struct Call:public Expr{
    unique_ptr<Expr> lhs_;
    vector<unique_ptr<Expr>> args_;
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
    Call(type::Type const *ty,unique_ptr<Expr>lhs):Expr(ty),lhs_(std::move(lhs)){}
};
struct Struct:public Expr{
    StructDecl& decl_;
    FuncDecl* init_;
    vector<unique_ptr<Expr>> args_;
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
    virtual void check(type::TypeManager& tyman,type::Type const *ty);
    Struct(StructDecl&decl):Expr(decl.ty_),decl_(decl){}
};
// struct Struct:public Expr{
//   std::vector<std::pair<string,Expr*>> val;
// };
struct Module:public Node{
    std::vector<unique_ptr<Decl>>defs_;
    unique_ptr<FuncDecl> main_;
    virtual void print(int lv);
    virtual void accept(HirVisitor&visitor);
};

struct HirVisitor{
    virtual void visit(chir::Module &node) =0 ;
    virtual void visit(chir::FuncDecl &node) =0 ;
    virtual void visit(chir::StructDecl&node) =0 ;
    virtual void visit(chir::EnumDecl&node) =0 ;
    virtual void visit(chir::Init&node) =0 ;
    // virtual void visit(chir::ValDeclStmt &node) =0 ;
    virtual void visit(chir::WCDecl &node) =0 ;
    virtual void visit(chir::VarDecl &node) =0 ;
    virtual void visit(chir::ExprStmt &node) =0 ;
    virtual void visit(chir::Assign &node) =0 ;
    virtual void visit(chir::Unary &node) =0 ;
    virtual void visit(chir::IncDec &node) =0 ;
    virtual void visit(chir::Bin &node) =0 ;
    virtual void visit(chir::Rel &node) =0 ;
    virtual void visit(chir::Or &node) =0 ;
    virtual void visit(chir::And &node) =0 ;
    virtual void visit(chir::Lval &node) =0 ;
    virtual void visit(chir::ThisSuper &node) =0 ;
    virtual void visit(chir::ArrIndex &node) =0 ;
    virtual void visit(chir::Selector &node) =0 ;
    virtual void visit(chir::Call &node) =0 ;
    virtual void visit(chir::Struct&node) =0 ;
    virtual void visit(chir::ArrayLit &node) =0 ;
    virtual void visit(chir::Lit &node) =0 ;
    // virtual void visit(chir::IntConst &node) =0 ;
    virtual void visit(chir::If &node) =0 ; 
    virtual void visit(chir::Jump &node) =0 ; 
    virtual void visit(chir::Ret &node) =0 ; 
    virtual void visit(chir::NumConv &node) =0 ; 
    virtual void visit(chir::While &node) =0 ;
    virtual void visit(chir::Block &node) =0 ;
    // virtual void visit(chir::FloatConst &node) =0 ;
    // virtual void visit(chir::AssignStmt &node) =0 ;
    virtual void visit(chir::DeclStmt &node) =0 ;
    virtual void visit(chir::RetStmt &node) =0 ;
    // virtual void visit(chir::ContinueStmt &node) =0 ;
    // virtual void visit(chir::BreakStmt &node) =0 ;
    // virtual void visit(chir::EmptyStmt &node) =0 ;

};

};
#endif

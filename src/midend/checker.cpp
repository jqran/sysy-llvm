#include "midend/checker.hpp"
#include "cassert"
#include "frontend/node.hpp"
#include "frontend/type.hpp"
#include "midend/hir.hpp"
#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <set>
#include <stack>
#include <string_view>
#include <utility>
#include <vector>

using uToken=unique_ptr<Token>;
static cjir::FuncDecl  *cur_func=nullptr;
static unique_ptr<cjir::Expr>  tmp_expr=nullptr;
static unique_ptr<cjir::Stmt>  tmp_stmt=nullptr;
static type::Type const *tmp_type=nullptr;
static type::Type const *target_type=nullptr;
static type::Type const * bool_type=nullptr;
static ::std::stack<cjir::Block*> blocks{};
static ::std::map<cjir::Expr*,vector<cjir::Expr*>> parent_lits;
static ::std::map<cjir::Decl*,type::Type const *>decl_tys;
static ::std::vector<type::Type const*>cur_func_tys;
static type::Type const *  INT_LIT_TY=nullptr;
static type::Type const *  FLOAT_LIT_TY=nullptr;
static std::set<cjir::Expr*> cur_func_lit;
template  <typename target_ty>
bool is_ty(cjir::Node  *const ptr){
    return static_cast<target_ty*>(ptr);
}

#define is_lit_ty(ty) ((ty)==INT_LIT_TY||(ty)==FLOAT_LIT_TY)
#define is_int_lit_ty(ty) ((ty)==INT_LIT_TY)
#define is_float_lit_ty(ty) ((ty)==FLOAT_LIT_TY)
#define is_int_ty(ty) ((ty)->type==type::TypeId::INT||(ty)->type==type::TypeId::UINT)
#define is_float_ty(ty)  ((ty)->type==type::TypeId::FLOAT)
// void lit_check(cjir::Expr* lit,type::Type const *const new_ty){
//     std::vector<cjir::Expr*> lits{lit};
//     while(!lits.empty()){
// 	auto expr=lits.back();
// 	expr->ty_=new_ty;
// 	cur_func_lit.erase(expr);
// 	lits.pop_back();
// 	auto iter=parent_lits.find(lit);
// 	if(iter!=parent_lits.end()){
// 	    lits.insert(lits.end(),iter->second.begin(),iter->second.end());
// 	    parent_lits.erase(iter);
// 	}
//     }
    
// }
static type::Type const*func_ty_check(type::Type const* decl=nullptr){
    auto rm_lit_ty=[&](){
	auto iter=cur_func_tys.begin();
	while(iter!=cur_func_tys.end()){
	    if(*iter==INT_LIT_TY||*iter==FLOAT_LIT_TY){
		iter=cur_func_tys.erase(iter);
	    }else ++iter;
	}
    };
    //defed
    if(decl!=nullptr){
	rm_lit_ty();
	for(auto i:cur_func_tys){
	    if(i!=decl)
		return nullptr;
	}
	return decl;
    }else{
	if(cur_func_tys.size()==1){
	    return cur_func_tys.front();
	}
	type::Type const* int_lit_ty=0,*float_lit_ty=0;
	for(auto i:cur_func_tys){
	    if(i==INT_LIT_TY){
		int_lit_ty=INT_LIT_TY;
	    }else if(i==FLOAT_LIT_TY){
		float_lit_ty=FLOAT_LIT_TY;
	    }
	}
	rm_lit_ty();
	auto t=cur_func_tys.front();
	for(auto i:cur_func_tys){
	    if(t!=i)
		return nullptr;
	}
	if(int_lit_ty!=0){
	    if(t->type!=type::TypeId::INT&&t->type!=type::TypeId::UINT){
		return nullptr;
	    }
	}else if(float_lit_ty!=0){
	    if(t->type!=type::TypeId::FLOAT){
		return nullptr;
	    }
	}
	return t;
    }
}

// type::Type const*Checker::var_type_check(type::Type const* var,type::Type const* val){
//     assert(is_lit_ty(var)==false);
//     if(val==INT_LIT_TY){
// 	assert(is_int_ty(var));
// 	if(is_int_lit_ty(var)||var==nullptr){
// 	    return type_man->getI64();
// 	}else{
// 	    assert(is_int_ty(var));
// 	    return var;
// 	}
// 	// val=var;
//     }else if(val==FLOAT_LIT_TY){
// 	assert(is_float_ty(var));
// 	// val=var;
// 	if(is_float_lit_ty(var)||var==nullptr){
// 	    return type_man->getF64();
// 	}else{
// 	    assert(is_float_ty(var));
// 	    return var;
// 	}
//     }
//     if(var==nullptr){
// 	return val;
//     }
//     assert(val==var);
//     return var;
// }
// type::Type const*Checker::var_type_check(type::Type const* decl,type::Type const* l,type::Type const* r){
//     assert(isvoid(l)==false||isvoid(r)==false);
//     if(decl==nullptr){
// 	if(is_int_lit_ty(l)&&is_int_lit_ty(r)){
// 	    // return i64;
// 	    this->type_man->getI64();
// 	}
// 	if(is_float_lit_ty(l)&&is_float_lit_ty(r)){
// 	    // return  f64;
// 	    this->type_man->getF64();	    
// 	}
// 	if(is_lit_ty(r)){
// 	    std::swap(l,r);
// 	}

// 	if(is_int_lit_ty(l)){
// 	    if(is_int_ty(r)){
// 		return r;
// 	    }
// 	}
// 	if(is_float_lit_ty(l)){
// 	    if(is_float_ty(r)){
// 		return r;
// 	    }
// 	}
// 	if(l==r){
// 	    return l;
// 	}else{
// 	    return nullptr;
// 	}
	
//     }else{
// 	if(l==INT_LIT_TY){
// 	    assert(is_int_ty(decl));
// 	    l=decl;
// 	}
// 	if(r==INT_LIT_TY){
// 	    assert(is_int_ty(decl));
// 	    r=decl;
// 	}
// 	if(l==FLOAT_LIT_TY){
// 	    assert(is_float_ty(decl));
// 	    l=decl;
// 	}
// 	if(r==FLOAT_LIT_TY){
// 	    assert(is_float_ty(decl));
// 	    r=decl;
// 	}
// 	assert(l==decl&&r==decl);
//     }
//     return decl;
// }

// void Checker::tranTy(cjir::Expr* l,cjir::Expr* r){
//     if(l->ty==tye)
// }

cjir::Bin::Binop astbin2hirbin(ast::BinOp op){
    using ast::BinOp;
    switch (op) {
    case BinOp::PlUS:
        return cjir::Bin::Binop::PlUS;
    case BinOp::MINUS:
        return cjir::Bin::Binop::MINUS;
    case BinOp::MULTI:
        return cjir::Bin::Binop::MULTI;
    case BinOp::SLASH:
        return cjir::Bin::Binop::SLASH;
    case BinOp::MOD:
        return cjir::Bin::Binop::MOD;
    case BinOp::ASSIGN:
        return cjir::Bin::Binop::ASSIGN;
    case BinOp::EQ:
        return cjir::Bin::Binop::EQ;
    case BinOp::NOT_EQ:
        return cjir::Bin::Binop::NOT_EQ;
    case BinOp::DOR:
        return cjir::Bin::Binop::LOR;
    case BinOp::DAND:
        return cjir::Bin::Binop::LAND;
    case BinOp::LT:
        return cjir::Bin::Binop::LT;
    case BinOp::LE:
        return cjir::Bin::Binop::LE;
    case BinOp::GT:
        return cjir::Bin::Binop::GT;
    case BinOp::GE:
        return cjir::Bin::Binop::GE;
    default:
        exit(10);
    }

}
// type::Type const* Checker::bin_type_check(type::Type const *const lhs,type::Type const *const rhs){
//     assert(isvoid(lhs)==false||isvoid(rhs)==false);
//     if(lhs==rhs){
// 	return lhs;
//     }
//     auto int_literltype=this->type_man->int_liter;
//     if(lhs==int_literltype){
// 	if(rhs->type==type::TypeId::UINT||rhs->type==type::TypeId::INT){
// 	    return rhs;
// 	}
//     }
//     if(rhs==int_literltype){
// 	if(lhs->type==type::TypeId::UINT||lhs->type==type::TypeId::INT){
// 	    return lhs;
// 	}
//     }
//     auto float_literltype=this->type_man->float_liter;
//     if(lhs==float_literltype){
// 	if(rhs->type==type::TypeId::FLOAT){
// 	    return rhs;
// 	}
//     }
//     if(rhs==float_literltype){
// 	if(lhs->type==type::TypeId::FLOAT){
// 	    return lhs;
// 	}
//     }
//     return nullptr;
// }
// bool Checker::auto_type_conversion(type::Type const *const type,type::Type const *const target_type){
//     if(type==target_type){
// 	return true;
//     }
//     if(type==this->type_man->int_liter){
// 	return target_type->type==type::TypeId::INT||target_type->type==type::TypeId::UINT;
//     }
//     if(type==this->type_man->float_liter){
// 	return target_type->type==type::TypeId::FLOAT;
//     }
//     return false;
// }
Checker::Checker():scopes({{{},ScopeType::GLOBAL}}),type_man(std::make_unique<type::TypeManager>()){
    bool_type=this->type_man->getBool();
}
void Checker::visit(ast::CompunitNode &node) {
    INT_LIT_TY=type_man->int_liter;
    FLOAT_LIT_TY=type_man->float_liter;
    this->module_=new cjir::Module();
    for(auto &stmt:node.global_defs){
	auto &defs=scopes.front().first;
        std::string  def(stmt->name);
        if(defs.count(def)){
            std::cerr<<"Redefinition of declaration '"<<def<<'\''<<endl;
	    assert(0);
	  
        }
        defs.insert({def,nullptr});
        stmt->accept(*this);
	assert(tmp_stmt);
	this->module_->defs_.emplace_back(std::move(tmp_stmt));
    }
    this->module_->print(0);
}
void Checker::visit(ast::FuncFParam &node) {
    auto &tok=node.type;
    auto type=type_man->getType(tok.get()->literal);
    cur_func->param_.push_back({std::move(tmp_expr),type});
    assert(type!=nullptr);
    // node.type=type;
}
void Checker::visit(ast::FuncDef &node) {
    cur_func_tys.clear();
    cur_func_lit.clear();
    scopes.push_back({{},ScopeType::FUNC});
    auto &defs=scopes.back().first;
    type::Type const*ret_ty=nullptr;
    if(node.type!=nullptr){
	ret_ty=this->type_man->getType(node.type->literal);
    }
    unique_ptr<cjir::FuncDecl> ufunc=nullptr;
    if(this->scopes.back().second==ScopeType::STRUCT){
	ufunc=std::make_unique<cjir::MemFunc>(ret_ty,node.name);
    }else{
	ufunc=std::make_unique<cjir::FuncDecl>(ret_ty,node.name);
    }
    
    cur_func=ufunc.get();
    blocks.push(ufunc->block_.get());
    // this->module_->defs_.push_back(std::move(ufunc));
    for(auto &arg:node.func_f_params){
        std::string def{arg.first};
        if(defs.count(def)){
            std::cerr<<"Redefinition of declaration '"<<def<<'\''<<endl;
            assert(0);
        }
        auto type=arg.second.get();
        if(type==0){
            assert(0);
        }
	// arg->accept(*this);
	// ufunc->param_.push_back({ std::move(tmp_expr),nullptr});
        defs.insert({def,type_man->getType(type->literal)});
        // arg->accept(*this);
    }
    node.body->accept(*this);
    assert(is_ty<cjir::Block>(tmp_expr.get()));
    
    ufunc->block_=unique_ptr<cjir::Block>(static_cast<cjir::Block*>(tmp_expr.release()));
    if(ufunc->block_->expr==nullptr&&ret_ty==nullptr){
	ret_ty=type_man->getVoid();
    }else{
	ret_ty=func_ty_check(ret_ty);
	assert(ret_ty!=0);
    }

    ufunc->ty_=ret_ty;
    scopes.pop_back();
    cur_func=nullptr;
    tmp_stmt=std::move(ufunc);
}
enum class perm{
    PUBLIC=1,
    DEFAULT,
    PROTECTED,
    PRIVATE,
};
void Checker::visit(ast::StructDeclStmt&node) {
    unique_ptr<type::DeclType>t=std::make_unique<type::DeclType>(node.name);
    scopes.push_back({{},ScopeType::STRUCT});
    for(auto& def:node.members){
        t->members.push_back({def.first->name,type_man->getType(def.first->name),def.second});
    }
    tmp_type=type_man->addDeclType(t.release());
    if(0==tmp_type){
        std::cerr<<"Redefinition of declaration '"<<node.name<<'\''<<endl;
        assert(0);
    }
    auto struct_decl=make_unique<cjir::StructDecl>(nullptr,node.name);
    std::vector<ast::FuncDef*> funs;
    for(auto& def:node.members){
	auto d=def.first.get();
	if(auto f=dynamic_cast<ast::FuncDef*>(d);f!=nullptr){
	    funs.push_back(f);
	}else{
	    d->accept(*this);
	    assert(is_ty<cjir::MemVar>(tmp_stmt.get()));
	    struct_decl->vars_.emplace_back(static_cast<cjir::MemVar*>(tmp_stmt.release()));
	}
    }
    for(auto f:funs){
	f->accept(*this);
    }
    
    tmp_stmt=std::move(struct_decl);
    scopes.pop_back();
}
// void Checker::visit(ast::ValDeclStmt &node) {assert(0);}
void Checker::visit(ast::ValDefStmt &node) {
    auto &tok=node.type;
    type::Type const * type=nullptr;
    if(tok!=nullptr){
        type=this->type_man->getType(tok->literal);
	target_type=type;
    }

    if(node.init_expr!=nullptr){
	node.init_expr->accept(*this);
    }
    if(tok==nullptr&&node.init_expr==nullptr){
	assert(0&&"unknown type");
    }

    tmp_expr->ty_=type;

    unique_ptr<cjir::VarDecl> decl;
    if(this->scopes.back().second==ScopeType::STRUCT){
	decl=make_unique<cjir::MemVar>(type,node.name,unique_ptr<cjir::Expr> {std::move(tmp_expr)});
    }else{
	decl=make_unique<cjir::VarDecl>(type,node.name,unique_ptr<cjir::Expr> {std::move(tmp_expr)});
    }
    decl->check(*type_man);
    decl_tys.emplace(decl.get(),type);
    tmp_expr=nullptr;
    tmp_type=nullptr;
    std::string na=decl->name_;
    auto &defs=this->scopes.back().first;
    defs.insert({na,type});
    // node.type=type;
    tmp_stmt=std::move(decl);
}
void Checker::visit(ast::ArrDefStmt &node) {assert(0);}
// void Checker::visit(ast::ConstDeclStmt &node) {assert(0);}
// void Checker::visit(ast::ConstDefStmt &node) {assert(0);}
// void Checker::visit(ast::ConstArrDefStmt &node) {assert(0);}
void Checker::visit(ast::ExprStmt &node) {
    node.expr->accept(*this);
    tmp_stmt=make_unique<cjir::ExprStmt>(std::move(tmp_expr));
    tmp_type=nullptr;
}
void Checker::visit(ast::AssignStmt &node) {tmp_type=nullptr;
    node.l_val->accept(*this);
    auto target_type=tmp_type;
    node.expr->accept(*this);
    auto expr=std::move(tmp_expr);
    // expr->ty_=var_type_check(target_type,expr->ty_);
    auto block=blocks.top();
    // unique_ptr<cjir::Assign> assign=make_unique<cjir::Assign>(l_val,epxr);
    // block->stmts_.push_back();
}
void Checker::visit(ast::PrefixExpr &node) {
    node.rhs->accept(*this);
    auto expr=std::move(tmp_expr);
    tmp_expr=make_unique<cjir::Unary>(static_cast<cjir::Unary::UnOp>(node.operat),std::move(expr));
    // assert(node.operat!=);
    assert(false);
}
void Checker::visit(ast::SelectorExpr &node) {assert(0);}
// void Checker::visit(ast::InfixExpr &node) {assert(0);}
void Checker::visit(ast::AssignExpr &node) {
    node.lhs->accept(*this);
    auto ltype=tmp_type;
    auto l=std::move(tmp_expr);
    node.rhs->accept(*this);
    auto r=std::move(tmp_expr);
    auto rtype=tmp_type;
    assert(tmp_type!=nullptr);
    tmp_expr=make_unique<cjir::Assign>(astbin2hirbin(node.operat),std::move(l),std::move(r));
    tmp_type=nullptr;
}
void Checker::visit(ast::RelopExpr &node) {    
    tmp_type=bool_type;
    node.ty=bool_type;
    node.lhs->accept(*this);
    auto l=std::move(tmp_expr);
    node.rhs->accept(*this);
    auto r=std::move(tmp_expr);
    node.ty=tmp_type;
    assert(tmp_type!=nullptr);
    tmp_expr=make_unique<cjir::Rel>(astbin2hirbin(node.operat),std::move(l),std::move(r));
    tmp_type=bool_type;
}
void Checker::visit(ast::EqExpr &node) {
    // tmp_type=bool_type;
    // node.ty=bool_type;
    node.lhs->accept(*this);
    auto l=std::move(tmp_expr);
    node.rhs->accept(*this);
    auto r=std::move(tmp_expr);
    assert(tmp_type!=nullptr);
    tmp_expr=make_unique<cjir::Rel>(astbin2hirbin(node.operat),std::move(l),std::move(r));
    tmp_type=bool_type;
}
void Checker::visit(ast::AndExp &node) {
    // tmp_type=bool_type;
    // node.ty=tmp_type;
    node.lhs->accept(*this);
    auto ltype=tmp_type;
    auto l=std::move(tmp_expr);
    node.rhs->accept(*this);
    auto r=std::move(tmp_expr);
    auto rtype=tmp_type;
    assert(ltype==bool_type);
    assert(rtype==bool_type);
    tmp_type=bool_type;
  
    assert(tmp_type!=nullptr);
    tmp_expr=make_unique<cjir::And>(cjir::Bin::LAND,std::move(l),std::move(r));
    tmp_type=bool_type;
}
void Checker::visit(ast::ORExp &node) {
    // tmp_type=bool_type;
    // node.ty=tmp_type;
    node.lhs->accept(*this);
    auto ltype=tmp_type;
    auto l=std::move(tmp_expr);
    node.rhs->accept(*this);
    auto r=std::move(tmp_expr);
    auto rtype=tmp_type;
    assert(ltype==bool_type);
    assert(rtype==bool_type);
    tmp_type=bool_type;
    // node.ty=tmp_type;
    
    assert(tmp_type!=nullptr);
    tmp_expr=make_unique<cjir::Or>(cjir::Bin::LOR,std::move(l),std::move(r));
    tmp_type=bool_type;
}
void Checker::visit(ast::BinopExpr &node) {
    node.lhs->accept(*this);
    auto l=std::move(tmp_expr);
    node.rhs->accept(*this);
    auto r=std::move(tmp_expr);
    // if(ltype==type_man->int_liter||ltype==type_man->float_liter){
    //     std::swap(ltype,rtype);
    // }
    // node.ty=tmp_type;
    assert(tmp_type!=nullptr);
    auto bin=make_unique<cjir::Bin>(astbin2hirbin(node.operat),std::move(l),std::move(r));
    bin->ty_=tmp_type;
    if(is_lit_ty(tmp_type)){
	parent_lits.insert({bin.get(),{bin->lhs_.get(),bin->rhs_.get()}});
    }
    tmp_expr=std::move(bin);
}
void Checker::visit(ast::LvalExpr &node) {
    tmp_type=findDef(node.name);
    if(tmp_type==nullptr){
	std::cerr<<node.name<<endl;
	assert(false);
    }
    tmp_expr=make_unique<cjir::Lval>(tmp_type,node.name);
}
void Checker::visit(ast::Literal &node) {
    if(node.type>=ast::LitType::INT&&node.type<=ast::LitType::INT_HEX){
        tmp_type=type_man->int_liter;
    }else if(node.type==ast::LitType::FLOAT){
        tmp_type=type_man->float_liter;
    }else if(node.type==ast::LitType::BOOL){
        tmp_type=type_man->getBool();
    }else{
        assert(0);
    }
    tmp_expr=make_unique<cjir::Lit>(node.literal->literal,tmp_type);
    cur_func_lit.insert(tmp_expr.get());
    node.ty=tmp_type;
}
// void Checker::visit(ast::IntConst &node) {assert(0);}
void Checker::visit(ast::InitializerExpr &node) {assert(0);}
// void Checker::visit(ast::FloatConst &node) {assert(0);}
// void Checker::visit(ast::AssignStmt &node) {assert(0);}
void Checker::visit(ast::BlockStmt &node) {
    for(auto &stmt:node.block_items){
        stmt->accept(*this);
    }
}
void Checker::visit(ast::IfExpr &node) {
    // string type="Bool";
    // auto bool_type=type_man->getType(type);
    node.cond_->accept(*this);
    if(bool_type!=tmp_type){
	assert(false);
    }
    auto cond=std::move(tmp_expr);
    assert(tmp_expr==0);
    assert(tmp_type==bool_type);
  
    scopes.push_back({{},ScopeType::IF});
  
    node.then_->accept(*this);
    auto then=std::move(tmp_expr);
    {
	assert(is_ty<cjir::Block>(then.get()));
    }
  
    unique_ptr<cjir::Expr> elsee=nullptr;
    unique_ptr<cjir::If> _if;
    if(node.else_!=nullptr){
        scopes.push_back({{},ScopeType::IF});
	node.else_->accept(*this);
	elsee=std::move(tmp_expr);
	{
	    assert(is_ty<cjir::Block>(elsee.get())||is_ty<cjir::If>(elsee.get()));
	}
	// tmp_type=bin_type_check(then->ty_,elsee->ty_);
	_if=make_unique<cjir::If>(tmp_type, std::move(cond),unique_ptr<cjir::Block>(static_cast<cjir::Block*>(then.release())),unique_ptr<cjir::Block>(static_cast<cjir::Block*>(elsee.release())));
      
    }else{
	tmp_type=then->ty_;
	_if=make_unique<cjir::If>(tmp_type, std::move(cond),unique_ptr<cjir::Block>(static_cast<cjir::Block*>(then.release())),nullptr);
    }
    if(is_lit_ty(_if->ty_)){
	auto iter=parent_lits.insert({_if.get(),{_if->then_.get()}}).first;
	if(_if->else_)
	    iter->second.push_back(_if->then_.get());
    }
    tmp_expr=std::move(_if);
    
}
void Checker::visit(ast::WhileExpr &node) {
    node.cond_->accept(*this);
    assert(tmp_type==bool_type);
    auto cond=std::move(tmp_expr);
    node.loop_->accept(*this);
    auto lo=std::move(tmp_expr);
    {
	assert(is_ty<cjir::Block>(lo.get()));
    }
    tmp_type=this->type_man->getUnit();
    auto loop=unique_ptr<cjir::Block>(static_cast<cjir::Block*>(lo.release()));
    if(loop->expr!=nullptr){
	loop->stmts_.emplace_back(make_unique<cjir::ExprStmt>(std::move(loop->expr)));
    }
    tmp_expr=make_unique<cjir::While>(tmp_type,std::move(cond),std::move(loop));
  
  
    // auto tmp_type=tmp_expr->ty;
}
void Checker::visit(ast::BlockExpr &node) {
    auto block=make_unique<cjir::Block>(node.scope_ty_);
    for(auto & s:node.stmts_){
	s->accept(*this);
	block->stmts_.push_back( std::move(tmp_stmt));
    }
    if(node.expr_!=nullptr){
	node.expr_->accept(*this);
	block->ty_=tmp_type;
	block->expr=std::move(tmp_expr);
    }else{
	tmp_type=nullptr;
    }

    if(is_lit_ty(block->ty_)){
	parent_lits.insert({block.get(),{block->expr.get()}});
    }

    tmp_expr=std::move(block);
}
// void Checker::visit(ast::IfStmt &node) {
//     // string type="Bool";
//     // auto bool_type=type_man->getType(type);
//     node.pred->accept(*this);
//     if(bool_type!=tmp_type){
//       assert(false);
//     }
//     node.then_stmt->accept(*this);
//     if(node.else_stmt)
//         node.else_stmt->accept(*this);  
// }
void Checker::visit(ast::WhileStmt &node) {assert(0);}
void Checker::visit(ast::CallExpr &node) {assert(0);}
void Checker::visit(ast::RetStmt &node) {
    assert(cur_func!=nullptr);
    node.expr->accept(*this);
    type::Type const * type=tmp_type;
    // auto target_type=this->findDef(cur_func->name_);
    // assert(auto_type_conversion(type,target_type ));

    if(tmp_expr->ty_==INT_LIT_TY){
	if(cur_func->ty_!=nullptr){
	    assert(is_int_ty(cur_func->ty_));
	}
    }else if(tmp_expr->ty_==FLOAT_LIT_TY){
	if(cur_func->ty_!=nullptr){
	    assert(is_float_ty(cur_func->ty_));
	}
    }else{
	cur_func_tys.push_back(tmp_expr->ty_);
    }
    
    tmp_stmt=make_unique<cjir::RetStmt>(std::move(tmp_expr));
}
void Checker::visit(ast::ContinueStmt &node) {
    bool is_loop=false;
    for(auto s:scopes){
        is_loop|=(s.second==ScopeType::LOOP);
    }
    assert(is_loop);

}
void Checker::visit(ast::BreakStmt &node) {
    bool is_loop=false;
    for(auto s:scopes){
        is_loop|=(s.second==ScopeType::LOOP);
    }
    assert(is_loop);
}
void Checker::visit(ast::EmptyStmt &node) {
    tmp_type=nullptr;
}
type::Type const * Checker::findDef(string&s){
    ssize_t i=scopes.size()-1;
    while(i>=0){
	auto &scop=scopes[i].first;
	auto iter=scop.find(s);
	if(iter!=scop.end()){
	    return iter->second;
	}
	--i;
    }
    return nullptr;
}

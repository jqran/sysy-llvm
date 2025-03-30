#include "midend/hir.hpp"
#include "frontend/type.hpp"
#include <cassert>
string binopTOStr(cjir::Bin::Binop op){
    switch (op) {
    case cjir::Bin::Binop::PlUS:
        return "+";
    case cjir::Bin::Binop::MINUS:
        return "-";
    case cjir::Bin::Binop::MULTI:
        return "*";
    case cjir::Bin::Binop::SLASH:
        return "/";
    case cjir::Bin::Binop::MOD:
        return "%";
    case cjir::Bin::Binop::ASSIGN:
        return "=";
    case cjir::Bin::Binop::EQ:
        return "==";
    case cjir::Bin::Binop::NOT_EQ:
        return "!=";
    case cjir::Bin::Binop::LOR:
        return "||";
    case cjir::Bin::Binop::LAND:
        return "&&";
    case cjir::Bin::Binop::LT:
        return "<";
    case cjir::Bin::Binop::LE:
        return "<=";
    case cjir::Bin::Binop::GT:
        return ">";
    case cjir::Bin::Binop::GE:
        return ">=";
    default:
        exit(10);
    }
}
void LevelPrint(int cur_level,string name){
    for(int i = 0 ; i < cur_level; ++i) cout << "|  ";
    cout << ">-->"<< name;
    cout << std::endl;
}

void cjir::ExprStmt::print(int lv){
    this->expr_->print(lv);
}
void cjir::Lval::print(int lv){
    LevelPrint(lv,"variable "+this->id_);
}
void cjir::Block::print(int lv){
    ++lv;
    for(auto &s:this->stmts_){
	s->print(lv);
    }
    if(this->expr)
	this->expr->print(lv);
    --lv;
}
void cjir::If::print(int lv){
    LevelPrint(lv, "if expr");
    LevelPrint(lv, "if cond begin");
    this->cond_->print(lv);
    LevelPrint(lv, "if cond end");
    LevelPrint(lv, "then begin");
    this->then_->print(lv);
    LevelPrint(lv, "then end");
    if(this->else_){
	LevelPrint(lv, "else begin");
	this->else_->print(lv);
	LevelPrint(lv, "else end");
    }

}
void cjir::While::print(int lv){
    LevelPrint(lv, "while expr");
    this->cond_->print(lv);
    ++lv;
    this->loop_->print(lv);
    --lv;
}
void cjir::Unary::print(int lv){
    LevelPrint(lv,((char const *)"operater")+(char)(this->op));  
    ++lv;
    this->rhs_->print(lv);
    --lv;
}

void cjir::Bin::print(int lv){
    LevelPrint(lv,"operater"+ binopTOStr(this->op));  
    ++lv;
    this->lhs_->print(lv);
    this->rhs_->print(lv);
    --lv;
}
void cjir::Lit::print(int lv){
    LevelPrint(lv, "literal "+this->lit_);
}
// void cjir::Decl::print(int lv){
//     assert(0);
// }
void cjir::VarDecl::print(int lv){
    if(this->init_){
	LevelPrint(lv, "decl "+this->name_ + " to" );
	this->init_->print(lv);
    }else{
	LevelPrint(lv, "decl "+this->name_ );
    }
    LevelPrint(lv, "decl end" );
}
void cjir::RetStmt::print(int lv){
    LevelPrint(lv, "return ");
    this->expr_->print(lv);
}

void cjir::FuncDecl::print(int lv){
    LevelPrint(lv, "func "+this->name_);
    ++lv;
    for(auto &[expr,ty]:this->param_){
	// expr->print(lv);
	// LevelPrint();
    }
    LevelPrint(lv, name_+" func body begin");
    this->block_->print(lv);
    LevelPrint(lv, name_+" func body end");
    --lv;
}
void cjir::StructDecl::print(int lv){
    ++lv;
    for(auto &m:this->vars_){
	m->print(lv);
    }
    for(auto &m:this->funcs_){
	m->print(lv);
    }
    --lv;
}
void cjir::Module::print(int lv){
    for(auto &i:this->defs_){
	i->print(lv);
    }
}
void cjir::ExprStmt::accept(cjir::HirVisitor& visitor){
    visitor.visit(*this);
}

void cjir::Lval::accept(cjir::HirVisitor& visitor){
    visitor.visit(*this);
}

void cjir::Unary::accept(cjir::HirVisitor& visitor){
    visitor.visit(*this);
}

void cjir::Bin::accept(cjir::HirVisitor& visitor){
    visitor.visit(*this);
}
void cjir::Rel::accept(cjir::HirVisitor& visitor){
    visitor.visit(*this);
}
void cjir::Or::accept(cjir::HirVisitor& visitor){
    visitor.visit(*this);
}
void cjir::And::accept(cjir::HirVisitor& visitor){
    visitor.visit(*this);
}

void cjir::Assign::accept(cjir::HirVisitor& visitor){
    visitor.visit(*this);
}

void cjir::Block::accept(cjir::HirVisitor& visitor){
    visitor.visit(*this);
}

void cjir::If::accept(cjir::HirVisitor& visitor){
    visitor.visit(*this);
}

void cjir::While::accept(cjir::HirVisitor& visitor){
    visitor.visit(*this);
}

void cjir::Lit::accept(cjir::HirVisitor& visitor){
    visitor.visit(*this);
}

void cjir::VarDecl::accept(cjir::HirVisitor& visitor){
    visitor.visit(*this);
}

void cjir::RetStmt::accept(cjir::HirVisitor& visitor){
    visitor.visit(*this);
}

void cjir::FuncDecl::accept(cjir::HirVisitor& visitor){
    visitor.visit(*this);
}

void cjir::MemVar::accept(cjir::HirVisitor& visitor){
    visitor.visit(*this);
}

void cjir::MemFunc::accept(cjir::HirVisitor& visitor){
    visitor.visit(*this);
}

void cjir::StructDecl::accept(cjir::HirVisitor& visitor){
    visitor.visit(*this);
}

void cjir::Module::accept(cjir::HirVisitor& visitor){
    visitor.visit(*this);
}


#define is_lit_ty(ty) ((ty)==tyman.int_liter||(ty)==tyman.float_liter)
#define is_int_lit_ty(ty) ((ty)==tyman.int_liter)
#define is_float_lit_ty(ty) ((ty)==tyman.float_liter)
#define is_int_ty(ty) ((ty)->type==type::TypeId::INT||(ty)->type==type::TypeId::UINT)
#define is_float_ty(ty)  ((ty)->type==type::TypeId::FLOAT)

void cjir::Lval::check(type::TypeManager& tyman,type::Type const *ty){
    assert(ty==this->ty_);
}
void cjir::Unary::check(type::TypeManager& tyman,type::Type const *ty){
    assert(ty==this->ty_);
}
void cjir::Bin::check(type::TypeManager& tyman,type::Type const *ty){
    if(is_lit_ty(this->ty_)){
	assert(ty->getSize()!=0);
	if(ty->type==this->ty_->type){
	    this->ty_=ty;
	}
    }else{
	assert(ty==this->ty_);
    }
    this->lhs_->check(tyman, ty);
    this->rhs_->check(tyman, ty);
}
void cjir::Rel::check(type::TypeManager& tyman,type::Type const *ty){
    auto _bool=tyman.getBool();
    assert(ty==_bool);
    assert(this->ty_==_bool);
    if(is_lit_ty(lhs_->ty_)&&is_lit_ty(rhs_->ty_)){
    }else{
	if(is_lit_ty(lhs_->ty_)){
	}

    }
}
void cjir::Or::check(type::TypeManager& tyman,type::Type const *ty){
    auto _bool=tyman.getBool();
    assert(ty==_bool);
    assert(this->ty_==_bool);
    this->lhs_->check(tyman, ty);
    this->rhs_->check(tyman, ty);
}
void cjir::And::check(type::TypeManager& tyman,type::Type const *ty){
    auto _bool=tyman.getBool();
    assert(ty==_bool);
    assert(this->ty_==_bool);
    this->lhs_->check(tyman, ty);
    this->rhs_->check(tyman, ty);

}
void cjir::Assign::check(type::TypeManager& tyman,type::Type const *ty){
    if(is_lit_ty(lhs_->ty_)){
	assert(this->rhs_->ty_->type==this->ty_->type);
	this->rhs_->ty_=ty;
    }else{
	assert(this->rhs_->ty_==this->ty_);
    }
    this->rhs_->check(tyman, ty);
}
void cjir::Block::check(type::TypeManager& tyman,type::Type const *ty){
    assert(ty==this->ty_);
    this->expr->check(tyman,ty);
}
void cjir::If::check(type::TypeManager& tyman,type::Type const *ty){
    assert(ty==this->ty_);
    this->cond_->check(tyman, tyman.getBool());
    this->then_->check(tyman, ty);
    this->else_->check(tyman, ty);

}
void cjir::While::check(type::TypeManager& tyman,type::Type const *ty){
    assert(ty==this->ty_);
    assert(ty==tyman.getUnit());
}
void cjir::Lit::check(type::TypeManager& tyman,type::Type const *ty){
    assert(ty->type==this->ty_->type);
    this->ty_=ty;
}
void cjir::VarDecl::check(type::TypeManager& tyman){
    if(this->ty_==nullptr){
	if(is_int_lit_ty(this->init_->ty_)){
	    this->ty_=tyman.getI64();
	}else if(is_float_lit_ty(this->init_->ty_)){
	    this->ty_=tyman.getF64();
	}else{
	    this->ty_=init_->ty_;
	}
	init_->check(tyman,this->ty_);
    }else{
	if(is_lit_ty(this->init_->ty_)){
	    assert(this->ty_->type==init_->ty_->type);
	}else{
	    assert(this->ty_=init_->ty_);
	}
	init_->check(tyman,this->ty_);
    }
}
void cjir::FuncDecl::check(type::TypeManager& tyman){
}
void cjir::StructDecl::check(type::TypeManager& tyman){
}

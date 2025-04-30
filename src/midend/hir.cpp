#include "midend/hir.hpp"
#include "midend/type.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <utility>
using std::cout;
namespace chir {

string binopTOStr(chir::Bin::Binop op){
    switch (op) {
    case chir::Bin::Binop::ADD:
        return "+";
    case chir::Bin::Binop::SUB:
        return "-";
    case chir::Bin::Binop::MULTI:
        return "*";
    case chir::Bin::Binop::SLASH:
        return "/";
    case chir::Bin::Binop::MOD:
        return "%";
    case chir::Bin::Binop::ASSIGN:
        return "=";
    case chir::Bin::Binop::EQ:
        return "==";
    case chir::Bin::Binop::NOT_EQ:
        return "!=";
    case chir::Bin::Binop::LOR:
        return "||";
    case chir::Bin::Binop::LAND:
        return "&&";
    case chir::Bin::Binop::LT:
        return "<";
    case chir::Bin::Binop::LE:
        return "<=";
    case chir::Bin::Binop::GT:
        return ">";
    case chir::Bin::Binop::GE:
        return ">=";
    default:
        exit(10);
    }
}


type::Type const* initCheck(chir::Expr&l,chir::Expr&r){
    assert(l.ty_->type==r.ty_->type);
    if(l.ty_->isLit()){
        if(r.ty_->isLit()==false){
            l.ty_=r.ty_;
            return r.ty_;
        }else{
            assert(l.ty_==r.ty_);
            return r.ty_;
        }
    }else if(r.ty_->isLit()){
        if(l.ty_->isLit()==false){
            r.ty_=l.ty_;
            return l.ty_;
        }else{
            assert(l.ty_==r.ty_);
            return l.ty_;
        }
    }else{
        assert(l.ty_==r.ty_);
        return r.ty_;
    }
}
ArrayLit::ArrayLit(std::vector<unique_ptr<Expr>> elements):elements_(std::move(elements)){
    // for(auto &e:elements_){
    //     if(e->ty_->isLit()==false){
    //         // this->ty_=e->ty_;
    //         break;
    //     }
    // }
}
ArrayLit::ArrayLit(type::Type const *const ty,std::vector<unique_ptr<Expr>> elements):Expr(ty),elements_(std::move(elements)){}
Bin::Bin(Bin::Binop op,unique_ptr<Expr> lhs,unique_ptr<Expr> rhs):Expr(::chir::initCheck(*lhs.get(),*rhs.get())),op(op),lhs_(std::move(lhs)),rhs_(std::move(rhs)){
}
Rel::Rel(type::Type const* ty,Bin::Binop op,unique_ptr<Expr> lhs,unique_ptr<Expr> rhs):Bin(ty,op,std::move(lhs),std::move(rhs)){
    assert(this->ty_->isBool());
}
Or::Or(type::Type const*ty,unique_ptr<Expr> lhs,unique_ptr<Expr> rhs):Bin(ty,Binop::LOR,std::move(lhs),std::move(rhs)){
    assert(this->ty_->isBool());
}
And::And(type::Type const* ty,unique_ptr<Expr> lhs,unique_ptr<Expr> rhs):Bin(ty,Binop::LAND,std::move(lhs),std::move(rhs)){
    assert(this->ty_->isBool());
}
Assign::Assign(unique_ptr<Expr> lhs,unique_ptr<Expr> rhs):Bin(nullptr,Bin::Binop::ASSIGN,std::move(lhs),std::move(rhs)){
    if(rhs_->ty_->isLit()){
        assert(lhs_->ty_->type==rhs_->ty_->type);
        rhs_->ty_=lhs_->ty_;
    }else
        assert(lhs_->ty_==rhs_->ty_);
}
If::If(unique_ptr<Expr>cond,unique_ptr<Expr>then,unique_ptr<Expr>el):Expr(),cond_(std::move(cond)),then_(std::move(then)),else_(std::move(el)){
    if(else_!=nullptr){
        if(then_->ty_==nullptr&&else_->ty_==nullptr){
            this->ty_=nullptr;
            return ;   
        }
        this->ty_=initCheck(*this->then_.get(), *this->else_.get());
    }else{
        this->ty_=nullptr;
    }
}


void LevelPrint(int cur_level,string name){
    for(int i = 0 ; i < cur_level; ++i) cout << "|  ";
    cout << ">-->"<< name;
    cout << std::endl;
}

void chir::ExprStmt::print(int lv){
    this->expr_->print(lv);
}
void chir::Lval::print(int lv){
    LevelPrint(lv,"variable "+this->id_);
}
void chir::ThisSuper::print(int lv){
    // LevelPrint(lv,"variable "+this->id_);
    assert(0);
}
void chir::Selector::print(int lv){
    // LevelPrint(lv,"variable "+this->id_);
    assert(0);
}
void chir::Call::print(int lv){
    // LevelPrint(lv,"variable "+this->id_);
    assert(0);
}
void chir::Struct::print(int lv){
    // LevelPrint(lv,"variable "+this->id_);
    assert(0);
}
void chir::Block::print(int lv){
    ++lv;
    for(auto &s:this->stmts_){
	s->print(lv);
    }
    if(this->expr)
	    this->expr->print(lv);
    --lv;
}
void chir::If::print(int lv){
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
void chir::Ret::print(int lv){
    LevelPrint(lv, "ret");
    if(this->expr_){
        expr_->print(lv);
    }
}
void chir::NumConv::print(int lv){
    LevelPrint(lv, "num conv");
}
void chir::While::print(int lv){
    LevelPrint(lv, "while expr");
    this->cond_->print(lv);
    ++lv;
    this->loop_->print(lv);
    --lv;
}
void chir::IncDec::print(int lv){
    this->expr_->print(lv);
    char const* s;
    if(this->is_inc_){
        s="++";
    }else{
        s="--";
    }
    LevelPrint(lv,s);  
}
void chir::Unary::print(int lv){
    LevelPrint(lv,((char const *)"operater")+(char)(this->op));  
    ++lv;
    this->rhs_->print(lv);
    --lv;
}

void chir::Bin::print(int lv){
    LevelPrint(lv,"operater"+ binopTOStr(this->op));  
    ++lv;
    this->lhs_->print(lv);
    this->rhs_->print(lv);
    --lv;
}
void chir::ArrayLit::print(int lv){
    assert(0);
}
void chir::Lit::print(int lv){
    LevelPrint(lv, "literal "+this->lit_);
}
// void chir::Decl::print(int lv){
//     assert(0);
// }
void chir::VarDecl::print(int lv){
    if(this->init_){
	LevelPrint(lv, "decl "+this->name_ + " to" );
	    this->init_->print(lv);
    }else{
	LevelPrint(lv, "decl "+this->name_ );
    }
    LevelPrint(lv, "decl end" );
}
void chir::WCDecl::print(int lv){
    if(this->init_){
	LevelPrint(lv, "decl "+this->name_ + " to" );
	this->init_->print(lv);
    }else{
	LevelPrint(lv, "decl "+this->name_ );
    }
    LevelPrint(lv, "decl end" );
}
void chir::DeclStmt::print(int lv){
    LevelPrint(lv, "decl stmt ");
    this->decl_->print(lv);
}
void chir::RetStmt::print(int lv){
    LevelPrint(lv, "return ");
    this->expr_->print(lv);
}

void chir::FuncDecl::print(int lv){
    LevelPrint(lv, "func "+this->name_);
    ++lv;
    for(auto &[expr,ty]:this->params_){
	// expr->print(lv);
	// LevelPrint();
    }
    LevelPrint(lv, name_+" func body begin");
    this->block_->print(lv);
    LevelPrint(lv, name_+" func body end");
    --lv;
}
void chir::StructDecl::print(int lv){
    ++lv;
    for(auto &m:this->vars_){
	m->print(lv);
    }
    for(auto &m:this->funcs_){
	m->print(lv);
    }
    --lv;
}
void chir::Module::print(int lv){
    for(auto &i:this->defs_){
	    i->print(lv);
    }
    this->main_->print(lv);
}
void chir::ExprStmt::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::Lval::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::ThisSuper::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::Selector::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::Call::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::Struct::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}
void chir::Unary::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::IncDec::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::Bin::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}
void chir::Rel::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}
void chir::Or::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}
void chir::And::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::Assign::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::Block::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::If::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::Ret::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::NumConv::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::While::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::ArrayLit::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::Lit::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::VarDecl::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::WCDecl::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::DeclStmt::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}
void chir::RetStmt::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::FuncDecl::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::MemVar::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::MemFunc::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::Init::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::StructDecl::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}

void chir::Module::accept(chir::HirVisitor& visitor){
    visitor.visit(*this);
}


#define is_lit_ty(ty) ((ty)==tyman.int_liter||(ty)==tyman.float_liter)
#define is_int_lit_ty(ty) ((ty)==tyman.int_liter)
#define is_float_lit_ty(ty) ((ty)==tyman.float_liter)
#define is_int_ty(ty) ((ty)->type==type::TypeId::INT||(ty)->type==type::TypeId::UINT)
#define is_float_ty(ty)  ((ty)->type==type::TypeId::FLOAT)

void chir::Lval::check(type::TypeManager& tyman,type::Type const *ty){
    if(ty!=nullptr)
        assert(ty==this->ty_);
}

void chir::ThisSuper::check(type::TypeManager& tyman,type::Type const *ty){
    if(ty!=nullptr){
        assert(ty==this->ty_);
    }
}

void chir::Selector::check(type::TypeManager& tyman,type::Type const *ty){
    if(ty!=nullptr){
        assert(ty==this->ty_);
    }
}

void chir::Call::check(type::TypeManager& tyman,type::Type const *ty){
    if(ty!=nullptr){
        assert(ty==this->ty_);
    }
}
void chir::Struct::check(type::TypeManager& tyman,type::Type const *ty){
    // if(ty!=nullptr){
    //     assert(ty==this->ty_);
    // }
}
void chir::IncDec::check(type::TypeManager& tyman,type::Type const *ty){
    assert(ty==this->ty_);
}
void chir::Unary::check(type::TypeManager& tyman,type::Type const *ty){
    assert(ty==this->ty_);
}
void chir::Bin::check(type::TypeManager& tyman,type::Type const *ty){
    // if(is_lit_ty(this->ty_)){
	//     assert(ty->getSize()!=0);
	// if(ty->type==this->ty_->type){
	//     this->ty_=ty;
	// }
    // }else{
	//     assert(ty==this->ty_);
    // }
    if(ty==nullptr){
        if(is_lit_ty(this->lhs_->ty_)){
            if(rhs_->ty_==nullptr){
                rhs_->check(tyman,nullptr);
            }
            if(rhs_->ty_!=nullptr)
                this->lhs_->ty_=rhs_->ty_;
            else{
                if(lhs_->ty_->isInt())
                    this->ty_=tyman.getI64();
                else{
                    this->ty_=tyman.getF64();
            }
            }
        }else if(is_lit_ty(this->rhs_->ty_)){
            if(lhs_->ty_==nullptr){
                lhs_->check(tyman,nullptr);
            }
            if(lhs_->ty_!=nullptr)
                this->rhs_->ty_=lhs_->ty_;
            else{
                if(rhs_->ty_->isInt())
                    this->ty_=tyman.getI64();
                else{
                    this->ty_=tyman.getF64();
            }
            }
        }
        if(is_lit_ty(lhs_->ty_)){
            if(lhs_->ty_->isInt())
                this->ty_=tyman.getI64();
            else{
                this->ty_=tyman.getF64();
            }
        }
    }else{
        this->ty_=ty;
    }
    ty=this->ty_;

    this->lhs_->check(tyman, ty);
    this->rhs_->check(tyman, ty);
}
void chir::Rel::check(type::TypeManager& tyman,type::Type const *ty){
    assert(lhs_->ty_!=nullptr);
    assert(rhs_->ty_!=nullptr);
    if(ty!=nullptr){
        assert(ty==tyman.getBool());
    }
    if(lhs_->ty_->isLit()){
        assert(tyman.findDist(lhs_->ty_,rhs_->ty_)>=0);
        if(rhs_->ty_->getSize()==0){
            if(rhs_->ty_->isFloat()){
                ty=tyman.getF64();
            }else{
                ty=tyman.getI64();
            }
        }else {
            ty=rhs_->ty_;
        }
    }else if(rhs_->ty_->isLit()){
        assert(tyman.findDist(rhs_->ty_,lhs_->ty_)>=0);
        if(lhs_->ty_->getSize()==0){
            if(lhs_->ty_->isFloat()){
                ty=tyman.getF64();
            }else{
                ty=tyman.getI64();
            }
        }else{
            ty=lhs_->ty_;
        }
    }else{
        assert(lhs_->ty_==rhs_->ty_);
        ty=lhs_->ty_;
    }
    this->lhs_->check(tyman, ty);
    this->rhs_->check(tyman, ty);
}
void chir::Or::check(type::TypeManager& tyman,type::Type const *ty){
    auto _bool=tyman.getBool();
    assert(ty==_bool);
    assert(this->ty_==_bool);
    this->lhs_->check(tyman, ty);
    this->rhs_->check(tyman, ty);
}
void chir::And::check(type::TypeManager& tyman,type::Type const *ty){
    auto _bool=tyman.getBool();
    assert(ty==_bool);
    assert(this->ty_==_bool);
    this->lhs_->check(tyman, ty);
    this->rhs_->check(tyman, ty);

}
void chir::Assign::check(type::TypeManager& tyman,type::Type const *ty){
    // if(is_lit_ty(lhs_->ty_)){
	//     assert(this->rhs_->ty_->type==this->ty_->type);
	//     this->rhs_->ty_=ty;
    // }else{
	//     assert(this->rhs_->ty_==this->ty_);
    // }
    this->rhs_->check(tyman, lhs_->ty_);
}
void chir::Block::check(type::TypeManager& tyman,type::Type const *ty){
    if(ty!=nullptr){
        if(this->ty_->isLit()){
            assert(this->ty_->type==ty->type);
            this->ty_=ty;
        }
        auto stmt=this->stmts_.back().get(); //->check(tyman,ty);
        if(auto expr=dynamic_cast<ExprStmt*>(stmt)){
            expr->expr_->check(tyman, ty);
        }
    }else{
    }
}
void chir::If::check(type::TypeManager& tyman,type::Type const *ty){
    if(ty!=nullptr){
        assert(ty==this->ty_);
        this->cond_->check(tyman, tyman.getBool());
        this->then_->check(tyman, ty);
        if(this->else_)
            this->else_->check(tyman, ty);
    }else{
        this->cond_->check(tyman, tyman.getBool());
        this->then_->check(tyman, ty);
        if(this->else_)
            this->else_->check(tyman, ty);
    }
}
void chir::Ret::check(type::TypeManager& tyman,type::Type const *ty){
    // assert(ty==this->ty_);
    if(this->expr_)
        if(ty!=nullptr)
            this->expr_->check(tyman, ty);
        else{
        }
    else{
        assert(ty==nullptr&&this->ty_==nullptr);
    }
}
void chir::NumConv::check(type::TypeManager& tyman,type::Type const *ty){
    if(ty)
        assert(ty==this->ty_);
    if(this->expr_->ty_->isLit()){
        if(this->expr_->ty_->isFloat()){
            this->expr_->ty_=tyman.getF64();
        }else{
            this->expr_->ty_=tyman.getI64();
        }
    }
}
void chir::While::check(type::TypeManager& tyman,type::Type const *ty){
    if(ty)
        assert(ty==this->ty_);
    this->cond_->check(tyman,tyman.getBool());
    // assert(ty==tyman.getUnit());
}
void chir::ArrayLit::check(type::TypeManager& tyman,type::Type const *ty){
    if(ty!=nullptr){
        if(this->ty_==nullptr){
            this->ty_=ty;
        }
        assert(this->ty_=ty);
        assert(ty->isArray());
    }else{
    }
    this->all_lit_=true;
    for(auto &e:elements_){
        if(e->ty_->isLit()==false){
            all_lit_=false;
            break;
        }
    }
    //类型不为空，类型检查
    if(this->ty_!=nullptr){
        auto arrty=(type::ArrayType const*)this->ty_;
        auto elementty=arrty->element_;
        for(auto &e:this->elements_){
            if(e->ty_->isLit()){
                if(elementty->type==e->ty_->type){
                    e->ty_=elementty;
                }
            }else{
                assert(e->ty_==elementty);
            }
        }
    }else{
        if(elements_.empty()){
            throw std::logic_error("empty array literal unknown type");
        }
        //数组不为空，不是literal，直接赋值类型
        for(auto &e:elements_){
            if(e->ty_->isLit()==false){
                this->ty_=tyman.getArrayTy(ty);
            }
        }
        //数组全为literal
        if(this->ty_==nullptr){
            auto &e=elements_.front();
            type::Type const * et;
            if(e->ty_->isSInt()){
                et=tyman.getI64();
            }else{
                et=tyman.getF64();
            }
            for(auto &e:elements_){
                e->ty_=et;
            }
            this->ty_=tyman.getArrayTy(et);
        }
    }
}
void chir::Lit::check(type::TypeManager& tyman,type::Type const *ty){
    if(ty!=nullptr){
        assert(tyman.findDist(ty,ty_)>=0);
        if(this->ty_==tyman.int_liter||this->ty_==tyman.float_liter){
            this->ty_=ty;
        }else{
            assert(ty==this->ty_);
        }
    }else{
        if(this->ty_->isInt()){
            this->ty_=tyman.getI64();
        }else if(this->ty_->isFloat()){
            this->ty_=tyman.getF64();
        }
    }
    // this->ty_=ty;
}
void chir::VarDecl::check(type::TypeManager& tyman){
    if(this->ty_==nullptr){
        this->init_->check(tyman,nullptr);
        assert(this->init_!=nullptr);
        if(is_int_lit_ty(this->init_->ty_)){
            this->ty_=tyman.getI64();
        }else if(is_float_lit_ty(this->init_->ty_)){
            this->ty_=tyman.getF64();
        }else{
            this->ty_=init_->ty_;
        }
	    init_->check(tyman,this->ty_);
    }else{
        if(this->init_){
            if(is_lit_ty(this->init_->ty_)){
                assert(tyman.findDist(this->ty_,init_->ty_)>=0);
                this->init_->ty_=this->ty_;
            }else{
                assert(this->ty_==init_->ty_);
            }
            init_->check(tyman,this->ty_);
        }

    }
}
void chir::WCDecl::check(type::TypeManager& tyman){
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

void chir::FuncDecl::check(type::TypeManager& tyman){
    if(this->ret_ty_!=nullptr){
        //不为数字类型直接判断
        if(!ret_ty_->isNum())
            for(auto ret:this->rets_){
                assert(ret_ty_->type==ret->ty_->type);
            }
        else {
            for(auto ret:rets_){
                //数字类型判断是否为常量
                if(ret->ty_->getSize()==0){
                    ret->check(tyman,ret_ty_);
                }else{
                    assert(ret_ty_->type==ret->ty_->type);
                }
            }

        }
    }else{
        for(auto ret:rets_){
            if(!ret->ty_->isNum()){
                this->ret_ty_=ret->ty_;
                
            }else if(ret->ty_->getSize()!=0){
                this->ret_ty_=ret->ty_;
            }else{
                continue;
            }
            break;
        }
        if(this->ret_ty_==nullptr){
            if(!rets_.empty()){
                auto ret=rets_.front();
                if(ret->ty_->isSInt()){
                    this->ret_ty_=tyman.getI64();
                }else{
                    this->ret_ty_=tyman.getF64();
                }
            }
        }
        this->check(tyman);
    }
    for(auto ret:rets_){
        ret->check(tyman,this->ret_ty_);
    }
}

void chir::Init::check(type::TypeManager& tyman){
    
}
void chir::StructDecl::check(type::TypeManager& tyman){
}
}

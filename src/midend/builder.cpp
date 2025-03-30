#include "midend/builder.hpp"
#include "frontend/node.hpp"
#include "frontend/type.hpp"
#include "midend/hir.hpp"
#include "midend/scope.hpp"
#include <alloca.h>
#include <cassert>
#include <iostream>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/Casting.h>
#include <string>
#include <sys/types.h>
#include <vector>

static ::llvm::Value* tmp_val=nullptr;
static ::llvm::Type* tmp_type=nullptr;
static ::llvm::Function* cur_func=nullptr;
static ::llvm::AllocaInst* ret_addr=nullptr;
static ::llvm::BasicBlock *entry_bb_of_cur_func=nullptr;
static ::llvm::BasicBlock *cur_bb_of_cur_func=nullptr;
static ::llvm::BasicBlock *ret_bb_of_cur_func=nullptr;
static std::vector<std::pair<llvm::BasicBlock*,llvm::BasicBlock*>>tf_bb;
static bool require_addr=false;
static std::vector<ScopeType> Scopes;
static ::llvm::Instruction::BinaryOps hir_binop2llvm_binop(cjir::Bin::Binop op){
    ::llvm::Instruction::BinaryOps ret;
    switch (op) {
    case cjir::Bin::Binop::PlUS:
	ret=::llvm::Instruction::BinaryOps::Add;
	break;
    case cjir::Bin::Binop::MINUS:
	ret=::llvm::Instruction::BinaryOps::Sub;
	break;
    case cjir::Bin::Binop::MULTI:
	ret=::llvm::Instruction::BinaryOps::Mul;
	break;
    case cjir::Bin::Binop::SLASH:
	ret=::llvm::Instruction::BinaryOps::SDiv;
	break;
    case cjir::Bin::Binop::MOD:
	ret=::llvm::Instruction::BinaryOps::SRem;
	break;
    case cjir::Bin::Binop::ASSIGN:
	// ret=::llvm::Instruction::BinaryOps::Add;
	assert(0);
	break;
    // case cjir::Bin::Binop::LOR:
    // 	// ret=::llvm::Instruction::BinaryOps::Or;
    // 	assert(0);
    // 	break;
    // case cjir::Bin::Binop::LAND:
    // 	// ret=::llvm::Instruction::BinaryOps::Add;
    // 	assert(0);
    // 	break;
    default:
	std::cerr<<op<<endl;
	assert(0);
    }
    return ret;
}

static ::llvm::CmpInst::Predicate hir_cmpop2llvm_cmpop(cjir::Bin::Binop op,type::TypeId tyid){
    ::llvm::CmpInst::Predicate ret;
    if(tyid==type::TypeId::INT)
	switch (op) {
	case cjir::Bin::Binop::EQ:
	    ret=::llvm::CmpInst::Predicate::ICMP_EQ;
	    break;
	case cjir::Bin::Binop::NOT_EQ:
	    ret=::llvm::CmpInst::Predicate::ICMP_NE;;
	    break;
	case cjir::Bin::Binop::LT:
	    ret=::llvm::CmpInst::Predicate::ICMP_SLT;
	    break;
	case cjir::Bin::Binop::LE:
	    ret=::llvm::CmpInst::Predicate::ICMP_SLE;
	    break;
	case cjir::Bin::Binop::GT:
	    ret=::llvm::CmpInst::Predicate::ICMP_SGT;
	    break;
	case cjir::Bin::Binop::GE:
	    ret=::llvm::CmpInst::Predicate::ICMP_SGE;
	    break;
	default:
	    assert(0);
	}
    else if(tyid==type::TypeId::UINT){
	switch (op) {
	case cjir::Bin::Binop::EQ:
	    ret=::llvm::CmpInst::Predicate::ICMP_EQ;
	    break;
	case cjir::Bin::Binop::NOT_EQ:
	    ret=::llvm::CmpInst::Predicate::ICMP_NE;;
	    break;
	case cjir::Bin::Binop::LT:
	    ret=::llvm::CmpInst::Predicate::ICMP_ULT;
	    break;
	case cjir::Bin::Binop::LE:
	    ret=::llvm::CmpInst::Predicate::ICMP_ULE;
	    break;
	case cjir::Bin::Binop::GT:
	    ret=::llvm::CmpInst::Predicate::ICMP_UGT;
	    break;
	case cjir::Bin::Binop::GE:
	    ret=::llvm::CmpInst::Predicate::ICMP_UGE;
	    break;
	default:
	    assert(0);
	}
    }else if(tyid==type::TypeId::FLOAT){
	switch (op) {
	case cjir::Bin::Binop::EQ:
	    ret=::llvm::CmpInst::Predicate::FCMP_OEQ;
	    break;
	case cjir::Bin::Binop::NOT_EQ:
	    ret=::llvm::CmpInst::Predicate::FCMP_ONE;;
	    break;
	case cjir::Bin::Binop::LT:
	    ret=::llvm::CmpInst::Predicate::FCMP_OLT;
	    break;
	case cjir::Bin::Binop::LE:
	    ret=::llvm::CmpInst::Predicate::FCMP_OLE;
	    break;
	case cjir::Bin::Binop::GT:
	    ret=::llvm::CmpInst::Predicate::FCMP_OGT;
	    break;
	case cjir::Bin::Binop::GE:
	    ret=::llvm::CmpInst::Predicate::FCMP_OGE;
	    break;
	default:
	    assert(0);
	}
    }
    return ret;
}

::llvm::Type*  Builder::getType(::type::Type const* ty){
    ::llvm::Type* type=nullptr;
    if(ty->type==type::TypeId::INT||ty->type==type::TypeId::UINT){
	type=(ty->getSize()==0)?this->irbuilder_->getInt64Ty(): this->irbuilder_->getIntNTy(ty->getSize());
    }else if(ty->type==type::TypeId::FLOAT){
	switch (ty->getSize()) {
	case 16:
	    type=this->irbuilder_->getHalfTy();
	    break;
	case 32:
	    type=this->irbuilder_->getFloatTy();
	    break;
	default:
	    type=this->irbuilder_->getDoubleTy();
	}
    }else{
	assert(0);
    }
    // if(IS_INT(front_type.t)){
    //     switch (front_type.size.size) {
    //     case LONG_SIZE:
    //         type=builder_->getInt64Ty();
    //         break;
    //     case INT_SIZE:
    //         type=builder_->getInt32Ty();
    //         break;
    //     case SHORT_SIZE:
    //         type=builder_->getInt16Ty();
    //         break;
    //     case CHAR_SIZE:
    //         type=builder_->getInt8Ty();
    //         break;
    //     }
    // }else if(IS_FLOAT(front_type.t)) {
    //     type=builder_->getFloatTy();
    // }else{
    //     type=builder_->getVoidTy();
    // }
    return type;
}
// void Builder::visit(ast::CompunitNode &node) {
//     for(auto &stmt:node.global_defs){
// 	stmt->accept(*this);
//     }
// }
// void Builder::visit(ast::FuncFParam &node) {assert(0);}
// void Builder::visit(ast::FuncDef &node) {
//     auto ret_ty=this->getType(get<type::Type const *>(node.type));
//     std::vector<::llvm::Type *> param_vec;
//     for(auto &param:node.func_f_params){
//         param->accept(*this);
//         param_vec.push_back(tmp_type);
//     }
    
//     auto func_type=::llvm::FunctionType::get(ret_ty,std::move(param_vec),false);
//     ::llvm::Function *fun =::llvm::Function::Create(func_type,::llvm::GlobalValue::ExternalLinkage,node.name,module_);
//     func_table.insert({node.name,fun});
//     cur_func=fun;
//     ::llvm::BasicBlock *bb_entry=::llvm::BasicBlock::Create(*context_,"entry_bb",fun);
//     irbuilder_->SetInsertPoint(bb_entry);
//     ret_addr = irbuilder_->CreateAlloca(ret_ty);
//     ret_bb_of_cur_func=::llvm::BasicBlock::Create(*context_,"ret_bb",fun);
//     node.body->accept(*this);

//     irbuilder_->SetInsertPoint(ret_bb_of_cur_func);
//     auto ret_val = irbuilder_->CreateLoad(ret_ty, ret_addr);
//     irbuilder_->CreateRet(ret_val);
//     // cur_bb_of_cur_func=null
// }
// void Builder::visit(ast::StructDeclStmt&node) {assert(0);}
// //  void Builder::visit(ast::ValDeclStmt &node) {assert(0);}
// void Builder::visit(ast::ValDefStmt &node) {
//     type::Type const* const ty=get<::type::Type const*>(node.type);
//     auto def=irbuilder_->CreateAlloca(getType(ty));
//     val_table.insert({node.name,def});
//     if(node.init_expr!=nullptr){
//         node.init_expr->accept(*this);
//         irbuilder_->CreateStore(tmp_val,def);
//     }
// }
// void Builder::visit(ast::ArrDefStmt &node) {assert(0);}
// //  void Builder::visit(ast::ConstDeclStmt &node) {assert(0);}
// //  void Builder::visit(ast::ConstDefStmt &node) {assert(0);}
// //  void Builder::visit(ast::ConstArrDefStmt &node) {assert(0);}
// void Builder::visit(ast::ExprStmt &node) {assert(0);}
// void Builder::visit(ast::AssignStmt &node) {assert(0);}
// void Builder::visit(ast::PrefixExpr &node) {assert(0);}
// void Builder::visit(ast::SelectorExpr &node) {assert(0);}
// //  void Builder::visit(ast::InfixExpr &node) {assert(0);}
// void Builder::visit(ast::AssignExpr &node) {assert(0);}
// void Builder::visit(ast::RelopExpr &node) {assert(0);}
// void Builder::visit(ast::EqExpr &node) {
//     need_load=true;
//     node.lhs->accept(*this);
//     auto lhs=tmp_val;
//     node.rhs->accept(*this);
//     auto rhs=tmp_val;
//     tmp_val=irbuilder_->CreateICmpEQ(lhs, rhs);
// }
// void Builder::visit(ast::AndExp &node) {assert(0);}
// void Builder::visit(ast::ORExp &node) {
//     need_load=true;
//     node.lhs->accept(*this);
//     auto lhs=tmp_val;
//     node.rhs->accept(*this);
//     auto rhs=tmp_val;
//     tmp_val=irbuilder_->CreateOr(lhs, rhs);
// }
// void Builder::visit(ast::BinopExpr &node) {
//     need_load=true;
//     node.lhs->accept(*this);
//     auto lhs=tmp_val;
//     node.rhs->accept(*this);
//     auto rhs=tmp_val;
//     need_load=false;
//     if(node.ty->type==type::TypeId::INT){
// 	switch (node.operat) {
// 	case ast::BinOp::PlUS:
// 	    tmp_val=irbuilder_->CreateAdd(lhs, rhs);
// 	    break;
// 	case ast::BinOp::MINUS:
// 	    tmp_val=irbuilder_->CreateSub(lhs, rhs);
// 	    break;
// 	case ast::BinOp::MULTI:
// 	    tmp_val=irbuilder_->CreateMul(lhs, rhs);
// 	    break;
// 	case ast::BinOp::SLASH:
// 	    tmp_val=irbuilder_->CreateSDiv(lhs, rhs);
// 	    break;
// 	case ast::BinOp::MOD:
// 	    tmp_val=irbuilder_->CreateSRem(lhs, rhs);
// 	    break;
// 	default:
// 	    assert(0);
// 	    break;
// 	}
//     }else if(node.ty->type==type::TypeId::UINT){
// 	switch (node.operat) {
// 	case ast::BinOp::PlUS:
// 	    tmp_val=irbuilder_->CreateAdd(lhs, rhs);
// 	    break;
// 	case ast::BinOp::MINUS:
// 	    tmp_val=irbuilder_->CreateSub(lhs, rhs);
// 	    break;
// 	case ast::BinOp::MULTI:
// 	    tmp_val=irbuilder_->CreateMul(lhs, rhs);
// 	    break;
// 	case ast::BinOp::SLASH:
// 	    tmp_val=irbuilder_->CreateUDiv(lhs, rhs);
// 	    break;
// 	case ast::BinOp::MOD:
// 	    tmp_val=irbuilder_->CreateURem(lhs, rhs);
// 	    break;
// 	default:
// 	    assert(0);
// 	    break;
// 	}      
//     }else if(node.ty->type==type::TypeId::FLOAT){
// 	switch (node.operat) {
// 	case ast::BinOp::PlUS:
// 	    tmp_val=irbuilder_->CreateFAdd(lhs, rhs);
// 	    break;
// 	case ast::BinOp::MINUS:
// 	    tmp_val=irbuilder_->CreateFSub(lhs, rhs);
// 	    break;
// 	case ast::BinOp::MULTI:
// 	    tmp_val=irbuilder_->CreateFMul(lhs, rhs);
// 	    break;
// 	case ast::BinOp::SLASH:
// 	    tmp_val=irbuilder_->CreateFDiv(lhs, rhs);
// 	    break;
// 	case ast::BinOp::MOD:
// 	    tmp_val=irbuilder_->CreateFRem(lhs, rhs);
// 	    break;
// 	default:
// 	    assert(0);
// 	    break;
// 	}      
//     }
// }
// void Builder::visit(ast::LvalExpr &node) {assert(0);}
// void Builder::visit(ast::Literal &node) {
//     // auto val=val_table.findValue(node.literal->literal);
//     // auto alloca=llvm::dyn_cast<llvm::AllocaInst>(val);
//     // if(alloca){
//     //     if(need_load){
//     // 	  auto load=irbuilder_->CreateLoad(alloca->getAllocatedType(), alloca);
//     // 	    tmp_val=load;
//     // 	}else{
//     // 	    tmp_val=alloca;
//     // 	}
//     // }
//     auto &str=node.literal->literal;
//     if(node.type==ast::LitType::DOUBLE){
// 	auto f=std::stod(str);
// 	tmp_val=llvm::ConstantFP::get(irbuilder_->getFloatTy(),f);
//     }else if(node.type==ast::LitType::FLOAT){
// 	auto f=std::stod(str);
// 	tmp_val=llvm::ConstantFP::get(irbuilder_->getFloatTy(),f);
//     }else if(node.type==ast::LitType::INT||node.type==ast::LitType::INT_BIN||node.type==ast::LitType::INT_OCTAL||node.type==ast::LitType::INT_HEX){
// 	auto i=std::stoll(str);
// 	tmp_val=llvm::ConstantInt::get(irbuilder_->getInt64Ty(),i);
//     }
  
// }



void Builder::visit(cjir::Module &node){
    for (auto &stmt :node.defs_){
	stmt->accept(*this);
    }
}
void Builder::visit(cjir::FuncDecl &node){
    Scopes.push_back(ScopeType::FUNC);
    llvm::Type* retty;
    if(node.ty_==type_man->getVoid()){
	retty=llvm::Type::getVoidTy(*context_);
    }else{
	retty=getType(node.ty_);
    }
    auto func_ty=llvm::FunctionType::get(retty, false);

    cur_func=::llvm::Function::Create(func_ty, llvm::Function::ExternalLinkage, node.name_, *this->module_);
    entry_bb_of_cur_func= llvm::BasicBlock::Create(*context_,"entry",cur_func);
    cur_bb_of_cur_func=entry_bb_of_cur_func;
    irbuilder_->SetInsertPoint(entry_bb_of_cur_func);
    ret_bb_of_cur_func= llvm::BasicBlock::Create(*context_,"ret",cur_func);
    node.block_->accept(*this);
    Scopes.pop_back();
}
void Builder::visit(cjir::StructDecl&node){
    Scopes.push_back(ScopeType::STRUCT);
    Scopes.pop_back();

}
//   void Builder::visit(cjir::ValDeclStmt &node){assert(0);}
void Builder::visit(cjir::VarDecl &node){
    tmp_type=getType(node.ty_);
    auto allo=this->irbuilder_->CreateAlloca(tmp_type);
    this->val_table.insert({node.name_,IRInfo{allo,node.ty_,tmp_type}});
    if(node.init_){
	if(node.init_->ty_==type_man->int_liter||node.init_->ty_==type_man->float_liter){
	    node.init_->ty_=node.ty_;
	}
	node.init_->accept(*this);
    }
    this->irbuilder_->CreateStore(tmp_val, allo);
}
void Builder::visit(cjir::ExprStmt &node){
    node.expr_->accept(*this);
}
void Builder::visit(cjir::Assign &node){
    // std::cerr<<"assign                    assign"<<endl;
    // assert(0);
    if(node.rhs_->ty_==type_man->int_liter||node.rhs_->ty_==type_man->float_liter){
	node.rhs_->ty_=node.lhs_->ty_;
    }

    node.rhs_->accept(*this);
    auto v=tmp_val;
    require_addr=true;
    node.lhs_->accept(*this);
    irbuilder_->CreateStore(v, tmp_val);
    require_addr=false;    
}
void Builder::visit(cjir::Unary &node){assert(0);}
//   void Builder::visit(cjir::SelectorExpr &node){assert(0);}
void Builder::visit(cjir::Bin &node){
    node.lhs_->accept(*this);
    auto l=tmp_val;
    node.rhs_->accept(*this);
    auto r=tmp_val;
    llvm::Instruction::BinaryOps op=hir_binop2llvm_binop(node.op);
    this->irbuilder_->CreateBinOp(op,l,r);
}
void Builder::visit(cjir::Rel &node){
    node.lhs_->accept(*this);
    auto l=tmp_val;
    node.rhs_->accept(*this);
    auto r=tmp_val;
    // llvm::Instruction::BinaryOps op;
    // this->irbuilder_->CreateBinOp(llvm::Instruction::BinaryOps Opc, Value *LHS, Value *RHS)
    tmp_val=this->irbuilder_->CreateCmp(hir_cmpop2llvm_cmpop(node.op,node.lhs_->ty_->type), l, r);
}

void Builder::visit(cjir::Or &node){
    tmp_val=nullptr;
    node.lhs_->accept(*this);
    auto l=tmp_val;
    tmp_val=nullptr;
    llvm::BasicBlock* or_false=llvm::BasicBlock::Create(*this->context_,"or_false",cur_func);
    tf_bb.push_back({tf_bb.back().first,or_false});
    
    irbuilder_->CreateCondBr(l, tf_bb.back().first,tf_bb.back().second);
    tf_bb.pop_back();
    irbuilder_->SetInsertPoint(or_false);
    tmp_val=nullptr;
    node.rhs_->accept(*this);
    // auto r=std::move(tmp_val);
    // irbuilder_->CreateCondBr(r, tf_bb.back().first,tf_bb.back().second);
    // tf_bb.pop_back();
}

void Builder::visit(cjir::And &node){
    tmp_val=nullptr;
    node.lhs_->accept(*this);
    auto l=tmp_val;
    tmp_val=nullptr;
    llvm::BasicBlock* and_true=llvm::BasicBlock::Create(*this->context_,"and",cur_func);
    tf_bb.push_back({and_true,tf_bb.back().second});
    irbuilder_->CreateCondBr(l, tf_bb.back().first,tf_bb.back().second);
    tf_bb.pop_back();
    irbuilder_->SetInsertPoint(and_true);
    tmp_val=nullptr;
    node.rhs_->accept(*this);
    // auto r=std::move(tmp_val);
    // irbuilder_->CreateCondBr(r, tf_bb.back().first,tf_bb.back().second);
    // tf_bb.pop_back();
}

void Builder::visit(cjir::Lval &node){
    auto info=this->val_table.findValue(node.id_);
    if(require_addr==false){
	tmp_val=this->irbuilder_->CreateLoad(info.contain_ty, info.val);
    }else
	tmp_val=info.val;
}
void Builder::visit(cjir::Lit &node){
    assert(node.ty_->getSize()!=0);
    if(node.ty_->type==type::TypeId::INT){
	int v;
	if(node.ty_->getSize()==1){
	    if(node.lit_=="true"){
		v=1;
	    }else{
		v=0;
	    }
	}else
	    v=std::stoll(node.lit_);

	tmp_val=llvm::ConstantInt::getSigned(irbuilder_->getIntNTy(node.ty_->getSize()), v);
    }else if(node.ty_->type==type::TypeId::UINT){
	uint a=std::stoull(node.lit_);
	tmp_val=llvm::ConstantInt::get(irbuilder_->getIntNTy(node.ty_->getSize()), a);
    }else if(node.ty_->type==type::TypeId::FLOAT){
	auto a=std::stod(node.lit_);
	tmp_val=llvm::ConstantFP::get(getType(node.ty_), a);	
    }else{
	assert(0);
    }
}
//   void Builder::visit(cjir::IntConst &node){assert(0);}
void Builder::visit(cjir::If &node){
    llvm::BasicBlock* t=nullptr,*f=nullptr,*next=nullptr;
    t=llvm::BasicBlock::Create(*this->context_,"if_t",cur_func);
    if(node.else_){
	f=llvm::BasicBlock::Create(*this->context_,"if_f",cur_func);
	next=llvm::BasicBlock::Create(*this->context_,"n",cur_func);
    }else{
	f=llvm::BasicBlock::Create(*this->context_,"n_f",cur_func);
	next=f;
    }
    tf_bb.push_back({t,f});
    tmp_val=nullptr;
    node.cond_->accept(*this);
    if(tmp_val)
	irbuilder_->CreateCondBr(tmp_val,t,f);
    irbuilder_->SetInsertPoint(t);
    node.then_->accept(*this);
    irbuilder_->CreateBr(next);

    if(node.else_){
	irbuilder_->SetInsertPoint(f);
	node.else_->accept(*this);
	irbuilder_->CreateBr(next);
    }
    irbuilder_->SetInsertPoint(next);
}
void Builder::visit(cjir::While &node){
    llvm::BasicBlock* t=nullptr,*f=nullptr;
    t=llvm::BasicBlock::Create(*this->context_,"",cur_func);
    f=llvm::BasicBlock::Create(*this->context_,"",cur_func);
    tf_bb.push_back({t,f});
    // auto size=tf_bb.size();
    node.cond_->accept(*this);
    if(tmp_val!=nullptr)
	irbuilder_->SetInsertPoint(t);
    node.loop_->accept(*this);
    irbuilder_->CreateBr(f);
    irbuilder_->SetInsertPoint(f);
}
void Builder::visit(cjir::Block &node){
    for(auto &stmt:node.stmts_){
	stmt->accept(*this);
    }
    if(node.expr){
	node.expr->accept(*this);
    }
}
void Builder::visit(cjir::RetStmt &node){
    irbuilder_->CreateBr(ret_bb_of_cur_func);
    irbuilder_->SetInsertPoint(ret_bb_of_cur_func);
    node.expr_->accept(*this);
    auto ret=tmp_val;
    irbuilder_->CreateRet(ret);
}


// void Builder::genIR(ast::CompunitNode*node){
//     this->visit(*node);
// }

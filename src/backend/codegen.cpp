#include "backend/codegen.hpp"
#include "backend/scope.hpp"
#include "frontend/node.hpp"
#include "frontend/type.hpp"
#include "frontend/riscv.hpp"
#include <cassert>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>
#include <memory>
#include <utility>
#include <vector>
namespace gen {


static ::llvm::Value* tmp_val;
static ::llvm::Type* tmp_type;
static ::llvm::Function *cur_func;
static ::llvm::BasicBlock *cur_bb_of_cur_func;
static ::llvm::BasicBlock *ret_bb_of_cur_func;
static ::llvm::Value* ret_addr;
struct true_false_BB {
    ::llvm::BasicBlock *trueBB = nullptr;
    ::llvm::BasicBlock *falseBB = nullptr;
};                              //& used for backpatching

static std::vector<true_false_BB> cond_stack; //& used for Cond
static std::vector<true_false_BB> while_stack;         //& used for break and continue

static bool in_func=false;
static bool should_load=false;
static bool require_lvalue_load=false;
llvm::Value* get_from_fp(ast::BinOp op,llvm::Value*lhs,llvm::Value*rhs, ::llvm::IRBuilder<>* const builder_){
    llvm::Value* ret;
    switch(op){
    case::ast::BinOp::PlUS:
        ret=builder_->CreateFAdd(lhs,rhs);
        break;
    case::ast::BinOp::MINUS:
        ret=builder_->CreateFSub(lhs,rhs);
        break;
    case::ast::BinOp::MULTI:
        ret=builder_->CreateFMul(lhs,rhs);
        break;
    case::ast::BinOp::SLASH:
        ret=builder_->CreateFDiv(lhs,rhs);
        break;
    case::ast::BinOp::MOD:
        assert(0);
        break;
    case::ast::BinOp::LT:
        ret=builder_->CreateFCmpOLT(lhs,rhs);
        break;
    case::ast::BinOp::LE:
        ret=builder_->CreateFCmpOLE(lhs,rhs);
        break;
    case::ast::BinOp::GT:
        ret=builder_->CreateFCmpOGT(lhs,rhs);
        break;
    case::ast::BinOp::GE:
        ret=builder_->CreateFCmpOGE(lhs,rhs);
        break;
    case::ast::BinOp::EQ:
        ret=builder_->CreateFCmpOEQ(lhs,rhs);
        break;
    case::ast::BinOp::NOT_EQ:
        ret=builder_->CreateFCmpONE(lhs,rhs);
        break;
    default:
        exit(151);
    }
    return tmp_val;
}
llvm::Value* get_from_int(ast::BinOp op,llvm::Value*lhs,llvm::Value*rhs, ::llvm::IRBuilder<>* const builder_){
    llvm::Value*ret;
    switch(op){
    case::ast::BinOp::PlUS:
        ret=builder_->CreateAdd(lhs,rhs);
        break;
    case::ast::BinOp::MINUS:
        ret=builder_->CreateSub(lhs,rhs);
        break;
    case::ast::BinOp::MULTI:
        ret=builder_->CreateMul(lhs,rhs);
        break;
    case::ast::BinOp::SLASH:
        ret=builder_->CreateSDiv(lhs,rhs);
        break;
    case::ast::BinOp::MOD:
        ret=builder_->CreateSRem(lhs,rhs);
        break;
    case::ast::BinOp::LT:
        ret=builder_->CreateICmpSLT(lhs,rhs);
        break;
    case::ast::BinOp::LE:
        ret=builder_->CreateICmpSLE(lhs,rhs);
        break;
    case::ast::BinOp::GT:
        ret=builder_->CreateICmpSGT(lhs,rhs);
        break;
    case::ast::BinOp::GE:
        ret=builder_->CreateICmpSGE(lhs,rhs);
        break;
    case::ast::BinOp::EQ:
        ret=builder_->CreateICmpEQ(lhs,rhs);
        break;
    case::ast::BinOp::NOT_EQ:
        ret=builder_->CreateICmpNE(lhs,rhs);
        break;
    default:
        exit(151);
    }
    return ret;
}
llvm::Value* get_bin_from(ast::BinOp op,llvm::Value*lhs,llvm::Value*rhs, ::llvm::IRBuilder<>* const builder_){
    if(lhs->getType()->isIntegerTy()){
        return get_from_int(op,lhs,rhs,builder_);
    }else{
        return get_from_fp(op,lhs,rhs,builder_);
    }
    return 0;
}

::llvm::Type*  CodeGen::getType(::type::ValType & front_type){
    ::llvm::Type* type=nullptr;
    if(IS_INT(front_type.t)){
        switch (front_type.size.size) {
        case LONG_SIZE:
            type=builder_->getInt64Ty();
            break;
        case INT_SIZE:
            type=builder_->getInt32Ty();
            break;
        case SHORT_SIZE:
            type=builder_->getInt16Ty();
            break;
        case CHAR_SIZE:
            type=builder_->getInt8Ty();
            break;
        }
    }else if(IS_FLOAT(front_type.t)) {
        type=builder_->getFloatTy();
    }else{
        type=builder_->getVoidTy();
    }
    return type;

}
void CodeGen::visit(ast::CompunitNode &node) {
    for(auto &stmt:node.global_defs){
        stmt->accept(*this);
    }
}
void CodeGen::visit(ast::FuncFParam &node) {
    if(IS_INT(node.type.t))
        tmp_type=builder_->getInt32Ty();
    else{
        tmp_type=builder_->getFloatTy();
    }

}
void CodeGen::visit(ast::FuncDef &node) {
    llvm::Type * ret_type;
    ret_type=getType(node.type);

    std::vector<::llvm::Type *> param_vec;
    for(auto &param:node.func_f_params){
        param->accept(*this);
        param_vec.push_back(tmp_type);
    }
    
    auto func_type=::llvm::FunctionType::get(ret_type,std::move(param_vec),false);
    ::llvm::Function *fun =::llvm::Function::Create(func_type,::llvm::GlobalValue::ExternalLinkage,node.name,module_);
    func_table.insert({node.name,fun});
    cur_func=fun;
    
    ::llvm::BasicBlock *bb_entry=::llvm::BasicBlock::Create(*context_,"entry_bb",fun);
    builder_->SetInsertPoint(bb_entry);
    if(ret_type->isFloatingPointTy() ) 
        ret_addr = builder_->CreateAlloca(builder_->getFloatTy());
    else if(ret_type->isIntegerTy())
        ret_addr = builder_->CreateAlloca(builder_->getInt32Ty());
    ret_bb_of_cur_func=::llvm::BasicBlock::Create(*context_,"ret_bb",fun);
    node.body->accept(*this);

    builder_->SetInsertPoint(ret_bb_of_cur_func);
    auto ret_val = builder_->CreateLoad(ret_type, ret_addr);
    builder_->CreateRet(ret_val);

    cur_bb_of_cur_func=nullptr;
    cur_func=nullptr;
}
void CodeGen::visit(ast::ValDeclStmt &node) {
    for(auto &def:node.var_def_list){
        def->accept(*this);
    }
}
void CodeGen::visit(ast::ValDefStmt &node) {
    auto def=builder_->CreateAlloca(getType(node.type));
    val_table->insert({node.name,def});
    if(node.init_expr!=nullptr){
        node.init_expr->accept(*this);
        builder_->CreateStore(tmp_val,def);
    }
}
void CodeGen::visit(ast::ArrDefStmt &node) {
    builder_->CreateAlloca(getType(node.type));
    if(node.initializers!=nullptr){

    }
    assert(0&&"todo!");
}
void CodeGen::visit(ast::ConstDeclStmt &node){
    auto tmp_type=getType(node.all_type);
    for (auto &def:node.var_def_list) {
        def->accept(*this);
    }
}
void CodeGen::visit(ast::ConstDefStmt &node){
    auto def=builder_->CreateAlloca(getType(node.type));
    val_table->insert({node.name,def});
    if(node.init_expr!=nullptr){
        node.init_expr->accept(*this);
        builder_->CreateStore(tmp_val,def);
    }else{
        assert(0);
    }
}
void CodeGen::visit(ast::ConstArrDefStmt &node) {
    assert(0&&"todo!");
}
void CodeGen::visit(ast::ExprStmt &node) {
    node.expr->accept(*this);
}
void CodeGen::visit(ast::AssignStmt &node) {
    require_lvalue_load=false;
    node.l_val->accept(*this);
    auto lhs=tmp_val;
    require_lvalue_load=true;
    node.expr->accept(*this);
    auto rhs=tmp_val;

    builder_->CreateStore(rhs,lhs);}
void CodeGen::visit(ast::PrefixExpr &node) {
    node.rhs->accept(*this);
    switch (node.operat) {
    case ::ast::UnOp::PLUS:
        break;
    case ::ast::UnOp::MINUS:
        tmp_val=builder_->CreateSub(llvm::ConstantInt::get(builder_->getInt32Ty(),0),tmp_val);
        break;
    case ::ast::UnOp::NOT:
        tmp_val=builder_->CreateICmpEQ( llvm::ConstantInt::get(builder_->getInt32Ty(),0), tmp_val);
        break;
    }
}
void CodeGen::visit(ast::AssignExpr &node) {
    require_lvalue_load=false;
    node.lhs->accept(*this);
    auto lhs=tmp_val;
    require_lvalue_load=true;
    node.rhs->accept(*this);
    auto rhs=tmp_val;

    builder_->CreateStore(rhs,lhs);
}
void CodeGen::visit(ast::AndExp &node) {
    assert(0&&"todo!");
}
void CodeGen::visit(ast::ORExp &node){
    assert(0&&"todo!");
}
void CodeGen::visit(ast::RelopExpr &node){
    require_lvalue_load=true;
    node.lhs->accept(*this);
    auto lhs=tmp_val;
    require_lvalue_load=true;
    node.rhs->accept(*this);
    auto rhs=tmp_val;
    if(lhs->getType()->isFloatTy()&&rhs->getType()->isIntegerTy()){
        rhs=builder_->CreateSIToFP(rhs,lhs->getType());
    }else if(lhs->getType()->isIntegerTy()&&rhs->getType()->isFloatTy()){
        rhs=builder_->CreateSIToFP(lhs,rhs->getType());
    }
    tmp_val=get_bin_from(node.operat,lhs,rhs,builder_);
}
void CodeGen::visit(ast::EqExpr &node){
    require_lvalue_load=true;
    node.lhs->accept(*this);
    auto lhs=tmp_val;
    require_lvalue_load=true;
    node.rhs->accept(*this);
    auto rhs=tmp_val;
    if(lhs->getType()->isFloatTy()&&rhs->getType()->isIntegerTy()){
        rhs=builder_->CreateSIToFP(rhs,lhs->getType());
    }else if(lhs->getType()->isIntegerTy()&&rhs->getType()->isFloatTy()){
        rhs=builder_->CreateSIToFP(lhs,rhs->getType());
    }
    tmp_val=get_bin_from(node.operat,lhs,rhs,builder_);
}
void CodeGen::visit(ast::BinopExpr &node) {
    require_lvalue_load=true;
    node.lhs->accept(*this);
    auto lhs=tmp_val;
    require_lvalue_load=true;
    node.rhs->accept(*this);
    auto rhs=tmp_val;
    if(lhs->getType()->isFloatTy()&&rhs->getType()->isIntegerTy()){
        rhs=builder_->CreateSIToFP(rhs,lhs->getType());
    }else if(lhs->getType()->isIntegerTy()&&rhs->getType()->isFloatTy()){
        rhs=builder_->CreateSIToFP(lhs,rhs->getType());
    }
    tmp_val=get_bin_from(node.operat,lhs,rhs,builder_);
}
void CodeGen::visit(ast::LvalExpr &node){
    auto var =this->val_table->findValue(node.name);
    bool should_return_lvalue_load = require_lvalue_load;
    require_lvalue_load = false;
    if(node.index_num.empty()) {
        if(should_return_lvalue_load) {
            tmp_val = builder_->CreateLoad(builder_->getInt32Ty(),var);
        }else {
            tmp_val = var;
        }
    }

}
void CodeGen::visit(ast::IntConst &node) {
    tmp_val= ::llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context_),node.Value.i);
    // return llvm::ConstantInt::get(llvm::Type::getInt32Ty,node.Value.i,false);
}
void CodeGen::visit(ast::InitializerExpr &node) {
    assert(0&&"todo!");
}
void CodeGen::visit(ast::FloatConst &node){
    tmp_val= ::llvm::ConstantFP::get(llvm::Type::getFloatTy(*context_),node.Value.i);
}
void CodeGen::visit(ast::BlockStmt &node) {
    enterScope();
    for(auto & i:node.block_items){
        i->accept(*this);
    }
    exitScope();
}
void CodeGen::visit(ast::IfStmt &node) {
    auto true_bb = ::llvm::BasicBlock::Create(*context_, "", cur_func);
    auto false_bb = ::llvm::BasicBlock::Create(*context_, "", cur_func);
    auto next_bb = ::llvm::BasicBlock::Create(*context_, "", cur_func);
    node.pred->accept(*this);
    assert(0&&"todo!");
}
void CodeGen::visit(ast::WhileStmt &node){
    assert(0&&"todo!");
}
void CodeGen::visit(ast::CallExpr &node) {
    assert(0&&"todo!");
}
void CodeGen::visit(ast::RetStmt &node) {
    require_lvalue_load=true;
    if(node.expr != nullptr){   //有返回值
        node.expr->accept(*this);
        //int
        if(cur_func->getReturnType()->isIntegerTy()){
            if(tmp_val->getType()->isFloatTy())
                tmp_val=builder_->CreateFPToSI(tmp_val,builder_->getInt32Ty());
        }else if(cur_func->getReturnType()->isFloatTy()){
            if(tmp_val->getType()->isIntegerTy())
                tmp_val=builder_->CreateSIToFP(tmp_val,builder_->getFloatTy());
        }
        builder_->CreateStore(tmp_val,ret_addr);
    }
    builder_->CreateBr(ret_bb_of_cur_func);
}
void CodeGen::visit(ast::ContinueStmt &node){
    assert(0&&"todo!");
}
void CodeGen::visit(ast::BreakStmt &node) {
    assert(0&&"todo!");
}
void CodeGen::visit(ast::EmptyStmt &node) {
}

}


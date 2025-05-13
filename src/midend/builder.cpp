#include "midend/builder.hpp"
#include "midend/hir.hpp"
#include "midend/scope.hpp"
#include "midend/type.hpp"
#include <algorithm>
#include <alloca.h>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <llvm-19/llvm/IR/Constant.h>
#include <llvm-19/llvm/IR/GlobalAlias.h>
#include <llvm-19/llvm/IR/GlobalValue.h>
#include <llvm-19/llvm/IR/GlobalVariable.h>
#include <llvm-19/llvm/IR/IRBuilder.h>
#include <llvm-19/llvm/IRReader/IRReader.h>
#include <llvm-19/llvm/Support/SourceMgr.h>
#include <llvm-19/llvm/Support/TypeSize.h>
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
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <tuple>
#include <utility>
#include <vector>
#include "llvm/Linker/Linker.h"
llvm::Function *Builder::getfunc(string s){
    static bool b=false;
    std::set<string> lib{"geti64","geti32","geti16","geti8","getf64","getf32","puti64","puti32","puti16","puti8","putf64","putf32"};
    if(b==false&&lib.find(s)!=lib.end()){
        llvm::SMDiagnostic err;
        std::unique_ptr<Module> runtimeModule = llvm::parseIRFile("/home/qran/code/sysy-llvm/lib/lib.ll", err, *context_);
        if (llvm::Linker::linkModules(*module_, std::move(runtimeModule),llvm::Linker::Flags::OverrideFromSrc)) {
            llvm::errs() << "Link failed\n";
            return nullptr;
        }
        b=true;
    }
    return module_->getFunction(s);
}
static ::llvm::Value* tmp_value=nullptr;

static  llvm::Value*  consumeVal(){
    assert(tmp_value!=nullptr);
    auto val=tmp_value;
    tmp_value=0;
    return val;
}

static void produceVal(llvm::Value* val){
    assert(tmp_value==nullptr);
    tmp_value=val;
}


static ::llvm::Function* cur_func=nullptr;
static chir::FuncDecl* cur_hir_func=nullptr;
static ::llvm::AllocaInst* ret_addr=nullptr;
static ::llvm::BasicBlock *entry_bb_of_cur_func=nullptr;
static ::llvm::BasicBlock *cur_bb_of_cur_func=nullptr;
static ::llvm::Value *ret_of_cur_func=nullptr;
static ::llvm::BasicBlock *ret_bb_of_cur_func=nullptr;
static std::vector<std::pair<llvm::BasicBlock*,llvm::BasicBlock*>>tf_bb;
static bool left=false;
static bool isselect=false; 
static std::vector<ScopeType> Scopes;
static std::map<chir::FuncDecl*,llvm::Function*> funcsmap;
static std::map<chir::FuncDecl*,llvm::Function*> structsmap;
// static chir::StructDecl*cur_hir_struct=nullptr;
void clearfunc(){
    tmp_value=nullptr;
    cur_func=nullptr;
    cur_hir_func=nullptr;
    ret_addr=nullptr;
    entry_bb_of_cur_func=nullptr;
    cur_bb_of_cur_func=nullptr;
    ret_bb_of_cur_func=nullptr;
    ret_of_cur_func=nullptr;
    tf_bb.clear();
    left=false;
}
static ::llvm::Instruction::BinaryOps hir_binop2llvm_binop(chir::Bin& bin){
    auto op=bin.op;
    ::llvm::Instruction::BinaryOps ret;
    if(bin.lhs_->ty_->isFloat()){
        switch (op) {
            case chir::Bin::Binop::ADD:
            ret=::llvm::Instruction::BinaryOps::FAdd;
            break;
            case chir::Bin::Binop::SUB:
            ret=::llvm::Instruction::BinaryOps::FSub;
            break;
            case chir::Bin::Binop::MULTI:
            ret=::llvm::Instruction::BinaryOps::FMul;
            break;
            case chir::Bin::Binop::SLASH:
            ret=::llvm::Instruction::BinaryOps::FDiv;
            break;
            case chir::Bin::Binop::ASSIGN:
            assert(0);
            break;
            default:
            std::cerr<<op<<std::endl;
            assert(0);
            }
    }else{
        switch (op) {
        case chir::Bin::Binop::ADD:
        ret=::llvm::Instruction::BinaryOps::Add;
        break;
        case chir::Bin::Binop::SUB:
        ret=::llvm::Instruction::BinaryOps::Sub;
        break;
        case chir::Bin::Binop::MULTI:
        ret=::llvm::Instruction::BinaryOps::Mul;
        break;
        case chir::Bin::Binop::SLASH:
        ret=::llvm::Instruction::BinaryOps::SDiv;
        break;
        case chir::Bin::Binop::MOD:
        ret=::llvm::Instruction::BinaryOps::SRem;
        break;
        case chir::Bin::Binop::ASSIGN:
        // ret=::llvm::Instruction::BinaryOps::Add;
        assert(0);
        break;
        // case chir::Bin::Binop::LOR:
        // 	// ret=::llvm::Instruction::BinaryOps::Or;
        // 	assert(0);
        // 	break;
        // case chir::Bin::Binop::LAND:
        // 	// ret=::llvm::Instruction::BinaryOps::Add;
        // 	assert(0);
        // 	break;
        default:
        std::cerr<<op<<std::endl;
        assert(0);
        }
    }
    return ret;
}

static ::llvm::CmpInst::Predicate hir_cmpop2llvm_cmpop(chir::Bin::Binop op,type::TypeId tyid){
    ::llvm::CmpInst::Predicate ret;
    if(tyid==type::TypeId::INT)
	switch (op) {
	case chir::Bin::Binop::EQ:
	    ret=::llvm::CmpInst::Predicate::ICMP_EQ;
	    break;
	case chir::Bin::Binop::NOT_EQ:
	    ret=::llvm::CmpInst::Predicate::ICMP_NE;;
	    break;
	case chir::Bin::Binop::LT:
	    ret=::llvm::CmpInst::Predicate::ICMP_SLT;
	    break;
	case chir::Bin::Binop::LE:
	    ret=::llvm::CmpInst::Predicate::ICMP_SLE;
	    break;
	case chir::Bin::Binop::GT:
	    ret=::llvm::CmpInst::Predicate::ICMP_SGT;
	    break;
	case chir::Bin::Binop::GE:
	    ret=::llvm::CmpInst::Predicate::ICMP_SGE;
	    break;
	default:
	    assert(0);
	}
    else if(tyid==type::TypeId::UINT){
	switch (op) {
	case chir::Bin::Binop::EQ:
	    ret=::llvm::CmpInst::Predicate::ICMP_EQ;
	    break;
	case chir::Bin::Binop::NOT_EQ:
	    ret=::llvm::CmpInst::Predicate::ICMP_NE;;
	    break;
	case chir::Bin::Binop::LT:
	    ret=::llvm::CmpInst::Predicate::ICMP_ULT;
	    break;
	case chir::Bin::Binop::LE:
	    ret=::llvm::CmpInst::Predicate::ICMP_ULE;
	    break;
	case chir::Bin::Binop::GT:
	    ret=::llvm::CmpInst::Predicate::ICMP_UGT;
	    break;
	case chir::Bin::Binop::GE:
	    ret=::llvm::CmpInst::Predicate::ICMP_UGE;
	    break;
	default:
	    assert(0);
	}
    }else if(tyid==type::TypeId::FLOAT){
	switch (op) {
	case chir::Bin::Binop::EQ:
	    ret=::llvm::CmpInst::Predicate::FCMP_OEQ;
	    break;
	case chir::Bin::Binop::NOT_EQ:
	    ret=::llvm::CmpInst::Predicate::FCMP_ONE;;
	    break;
	case chir::Bin::Binop::LT:
	    ret=::llvm::CmpInst::Predicate::FCMP_OLT;
	    break;
	case chir::Bin::Binop::LE:
	    ret=::llvm::CmpInst::Predicate::FCMP_OLE;
	    break;
	case chir::Bin::Binop::GT:
	    ret=::llvm::CmpInst::Predicate::FCMP_OGT;
	    break;
	case chir::Bin::Binop::GE:
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
    if(ty->type==type::TypeId::INT){
	    type=(ty->getSize()==0)?this->irbuilder_->getInt64Ty(): this->irbuilder_->getIntNTy(ty->getSize());
    }else if(ty->type==type::TypeId::UINT){
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
    }else if(ty->isUInt()){
        return irbuilder_->getVoidTy();
    }else if(ty->isPoint()){
        return getType(((type::PointType*)ty)->element_)->getPointerTo();
    }else if(ty->isArray()){
        return this->arraytype;
    }else{
        // 
        return getLLVMStructTy(ty);
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

// using llvm::Function,llvm::FunctionType;
using namespace llvm;
void Builder::visit(chir::Module &node){
    module_->setModuleIdentifier("aaa");
// malloc: i8* malloc(i64)    
    auto i64=irbuilder_->getInt64Ty();
    auto ptr=irbuilder_->getPtrTy();
    FunctionType *mallocType = FunctionType::get(
        Type::getInt8Ty(*context_)->getPointerTo(), // 返回类型 i8*
        { Type::getInt64Ty(*context_) }, // 参数类型 i64
        false
    );
    Function *mallocFunc = Function::Create(
        mallocType,
        Function::ExternalLinkage,
        "malloc",
        *this->module_
    );
    FunctionType *memcpyType = FunctionType::get(
        irbuilder_->getVoidTy(), // 返回类型 i8*
        { ptr,ptr,i64}, // 参数类型 i64
        false
    );
    Function *memcpy = Function::Create(
        memcpyType,
        Function::ExternalLinkage,
        "memcpy",
        *this->module_
    );
    // free: void free(i8*)
    FunctionType *freeType = FunctionType::get(
        Type::getVoidTy(*context_),
        { /*Type::getInt8Ty(*context_)->getPointerTo() */},
        false
    );
    Function *freeFunc = Function::Create(
        freeType,
        Function::ExternalLinkage,
        "free",
        *this->module_
    );


    auto arrays=type_man->getAllArrayType();
    // auto i64=irbuilder_->getInt64Ty();
    // auto ptr=irbuilder_->getPtrTy();
    ::llvm::StructType* __struct =nullptr;
    if(arrays.empty()==false){
        // auto arrayStruct=new chir::StructDecl(nullptr,"CangJieArray");
        // __struct->setBody();
        //size element size
        __struct= ::llvm::StructType::create(*context_,{ptr,i64},"CangJieArray");
        // std::vector<llvm::Type*>vars;
        // __struct->setBody(vars);
        this->arraytype=__struct;
    }
    //init,get,clone
    std::map<type::Type const *, std::tuple<llvm::Function*,llvm::Function*,llvm::Function*>> array_decl;
    for(auto arr:arrays){
        auto llvm_element_ty=getType(arr.first);
        // auto init_ty=FunctionType::get(llvm_element_ty,{ptr,ptr},false);
        // //create arr initfunc
        // {

        //     llvm::DataLayout layout("e-p:64:64:64-i64:64-v128:64:64-a:0:64-n8:16:32:64");

        //     uint64_t sizeInBytes = layout.getTypeAllocSize(__struct);
        //     // uint64_t alignment = layout.getABITypeAlignment(__struct);

        //     llvm::Function *initfunc=llvm::Function::Create(init_ty,llvm::GlobalValue::ExternalLinkage,arr.first->name_+"init");
        //     auto entry= llvm::BasicBlock::Create(*context_,"",initfunc);
        //     irbuilder_->SetInsertPoint(entry);
        //     // auto args=initfunc->arg_begin();
        //     auto this_ptr=initfunc->getArg(0);
        //     auto other_struct=initfunc->getArg(1);
        //     // auto size=other_struct.gets
            
            
        //     auto element_num=irbuilder_->CreateStructGEP(__struct, other_struct,1); 
        //     auto size=irbuilder_->CreateMul(element_num,ConstantInt::get(i64,sizeInBytes));
        //     irbuilder_->CreateCall(memcpy,{this_ptr,other_struct,size});
        //     // auto element_size=irbuilder_->CreateStructGEP(__struct, other_struct,1);
        //     // llvm::ArrayType *arrTy = llvm::ArrayType::get(llvm_element_ty, 4).getel;
        //     // auto dataPtr = irbuilder_->CreateStructGEP(other_struct, vecPtr, 0);
        // }

        auto get=FunctionType::get(llvm_element_ty,ptr,false);

        auto clone=FunctionType::get(ptr/*new */,ptr/*this*/,false);

        array_decl.insert({arr.first,{nullptr,nullptr,nullptr}});
    }


    for (auto &stmt :node.defs_){
	    stmt->accept(*this);
    }
    if(node.main_)
        node.main_->accept(*this);
}
void Builder::visit(chir::FuncDecl &node){
    clearfunc();
    cur_hir_func=&node;
    Scopes.push_back(ScopeType::FUNC);
    val_table.enter();
    llvm::Type* retty;
    if(node.ret_ty_==type_man->getUnit()){
	    retty=llvm::Type::getVoidTy(*context_);
    }else{
	    retty=getType(node.ret_ty_);
    }
    std::vector<llvm::Type*> args;
    string name;
    chir::MemFunc *memf=nullptr;
    if(memf=dynamic_cast<chir::MemFunc*>(&node);memf){
        args.push_back(irbuilder_->getPtrTy());
        name=memf->id_;
    }else {
        name=node.name_;
    }
    for(auto iter:node.params_){
        args.push_back(getType(iter.second));
    }

    auto func_ty=llvm::FunctionType::get(retty, args,false);
    
    cur_func=::llvm::Function::Create(func_ty, llvm::Function::GlobalValue::ExternalLinkage, name, *this->module_);
    funcsmap.insert({&node,cur_func});
    // for(size_t i=0;i<node.params_.size();++i){
    //     val_table.insert({node.params_[i].first,IRInfo{DefTy::FUNC_PARAM,cur_func->getArg(i),node.params_[i].second,cur_func->getArg(i)->getType()}});
    // }

    entry_bb_of_cur_func= llvm::BasicBlock::Create(*context_,"",cur_func);

    cur_bb_of_cur_func=entry_bb_of_cur_func;
    irbuilder_->SetInsertPoint(entry_bb_of_cur_func);
    if(memf){
        for(size_t i=0;i<node.params_.size();++i){
            auto arg_addr=irbuilder_->CreateAlloca(cur_func->getArg(i+1)->getType());
            irbuilder_->CreateStore( cur_func->getArg(i+1),arg_addr);
            val_table.insert({node.params_[i].first,IRInfo{DefTy::LET,arg_addr,node.params_[i].second,cur_func->getArg(i)->getType()}});
        }

    }else{
        for(size_t i=0;i<node.params_.size();++i){
            auto arg_addr=irbuilder_->CreateAlloca(cur_func->getArg(i)->getType());
            irbuilder_->CreateStore( cur_func->getArg(i),arg_addr);
            val_table.insert({node.params_[i].first,IRInfo{DefTy::LET,arg_addr,node.params_[i].second,cur_func->getArg(i)->getType()}});
        }
    }
    if(retty->isVoidTy()==false)
        ret_of_cur_func=irbuilder_->CreateAlloca(getType(node.ret_ty_));

    ret_bb_of_cur_func= llvm::BasicBlock::Create(*context_,"",cur_func);

    node.block_->accept(*this);
    // if(ret_bb_of_cur_func->hasNUses(0)){
    //     llvm::DeleteDeadBlock(ret_bb_of_cur_func);
    // }
    // auto  hasNoReturn=[](llvm::Function &F)->llvm::BasicBlock* {
    //     for (llvm::BasicBlock &BB : F) {
    //       if (llvm::isa<llvm::UnreachableInst>(BB.getTerminator())) {
    //         return &BB;
    //       }
    //     }
    //     return nullptr;
    //   };
    //   if(auto b=hasNoReturn(*cur_func)){

    //   }
    auto b=irbuilder_->GetInsertBlock();
    if(b->empty()){
        b->removeFromParent();
    }
    else if(b->back().isTerminator()==false){
        irbuilder_->CreateBr(ret_bb_of_cur_func);
    }
    if(ret_of_cur_func){
        irbuilder_->SetInsertPoint(ret_bb_of_cur_func);
        irbuilder_->CreateRet(irbuilder_->CreateLoad(retty,ret_of_cur_func));
    }else{
        irbuilder_->SetInsertPoint(ret_bb_of_cur_func);
        irbuilder_->CreateRetVoid();
    }
    val_table.exit();
    Scopes.pop_back();
}
void Builder::visit(chir::StructDecl &node){
    // Scopes.push_back(ScopeType::STRUCT);
    // Scopes.pop_back();
    // LLVMContext& context = ...;
    // cur_hir_struct=&node;
    ::llvm::StructType* __struct = ::llvm::StructType::create(*context_,node.name_);
    // __struct->setBody();
    std::vector<llvm::Type*>vars;
    for(auto &var:node.vars_){
        vars.push_back(getType(var->ty_));
    }

    __struct->setBody(vars);
        this->struct_tys_.push_back({node.ty_,__struct});
    // MyStruct->setBody({ Type::getInt32Ty(context), 
                    // Type::getFloatTy(context), 
                    // Type::getInt8PtrTy(context) });
    for(auto &init:node.inits_){
        init->accept(*this);
    }
    for(auto &fun:node.funcs_){
        fun->accept(*this);
    }
    // cur_hir_struct=nullptr;
}
void Builder::visit(chir::EnumDecl &node){
    auto tagty=irbuilder_->getIntNTy(node.getTagSize());
    std::vector<std::pair<std::string, llvm::Type*>>contexts;
    for(auto &i:node.contexts_){
        ::llvm::StructType* __struct = ::llvm::StructType::create(*context_,i.first);
        vector<llvm::Type*>tys;
        for(auto t:i.second){
            if(t->isNum()==false&&t->isBool()==false&&t->isUnit()==false){
                tys.push_back(irbuilder_->getPtrTy());
            }else
                tys.push_back(getType(t));
        }
        __struct->setBody(tys);
        contexts.push_back({i.first,__struct});
    }
    llvm::DataLayout layout(module_);
    uint  max_size=0;
    for(auto i:contexts){
        uint new_size=layout.getTypeAllocSize(i.second);   
        if(max_size<new_size){
            max_size=new_size;
        }
    }
    Type *payloadTy = ArrayType::get(irbuilder_->getInt8Ty(),max_size);
    StructType *enumty = StructType::create(*this->context_, {tagty, payloadTy}, node.name_);
    this->struct_tys_.push_back({node.ty_,enumty});
    // llvm::GlobalVariable* gTLS = new llvm::GlobalVariable(
    //     *module_,
    //     enumTy,
    //     false,
    //     llvm::GlobalValue::ExternalLinkage,
    //     nullptr,
    //     node.name_
    // );
}
//   void Builder::visit(chir::ValDeclStmt &node){assert(0);}
void Builder::visit(chir::VarDecl &node){
    node.check(*this->type_man);
    auto tmp_type=getType(node.ty_);
    
    if(Scopes.empty()){    
        if(node.init_){
            node.init_->accept(*this);
            auto init=static_cast<llvm::Constant*>(consumeVal());
            llvm::GlobalVariable* gTLS = new llvm::GlobalVariable(
                *module_,
                tmp_type,
                false,
                llvm::GlobalValue::ExternalLinkage,
                init,
                node.name_
            );
        }else{
            llvm::GlobalVariable* gTLS = new llvm::GlobalVariable(
                *module_,
                tmp_type,
                false,
                llvm::GlobalValue::ExternalLinkage,
                nullptr,
                node.name_
            );
        }
        // llvm::GlobalVariable *g=new llvm::GlobalVariable()

    }else{
        auto allo=this->irbuilder_->CreateAlloca(tmp_type);
        // produceVal(allo);
        this->val_table.insert({node.name_,IRInfo{node.mod_,allo,node.ty_,allo->getType()}});
        if(node.init_){
            if(node.init_->ty_==type_man->int_liter||node.init_->ty_==type_man->float_liter){
                node.init_->ty_=node.ty_;
            }
            node.init_->accept(*this);
            // if(tmp_val->getType()!=allo->getType())
            if(node.ty_->isArray()){
                // node.init_->accept(*this);
                // this->;
                auto tmp=consumeVal();
                if(tmp->getType()->isPointerTy()){
                    this->irbuilder_->CreateStore(irbuilder_->CreateLoad(tmp_type,tmp), allo);    
                }else
                    this->irbuilder_->CreateStore(tmp, allo);
                // 先设置 capacity
                // auto sizeTy = llvm::Type::getInt64Ty(*this->context_);
                // auto capValue = llvm::ConstantInt::get(sizeTy, 16);
                // auto capPtr = irbuilder_->CreateStructGEP(getType(node.ty_), allo, 2); // 第2个成员是 capacity
                // irbuilder_->CreateStore(capValue, capPtr);

                // // malloc 元素空间
                // auto allocSize = irbuilder_->CreateMul(capValue, llvm::ConstantInt::get(sizeTy, 4));
                // auto mallocFunc = module_->getFunction("malloc");
                // auto rawDataPtr = irbuilder_->CreateCall(mallocFunc, {ConstantInt::get(irbuilder_->getInt64Ty(),1024)});

                // // 设置 data 指针
                // auto dataPtr = irbuilder_->CreateStructGEP(getType(node.ty_) ,allo, 0); // 第0个成员是 data
                // irbuilder_->CreateStore(rawDataPtr, dataPtr);

                // // size = 0
                // auto sizePtr = irbuilder_->CreateStructGEP(getType(node.ty_), allo, 1);
                // irbuilder_->CreateStore(llvm::ConstantInt::get(sizeTy, 0), sizePtr);
            }
            else
                this->irbuilder_->CreateStore(consumeVal(), allo);
        }

    }
}
void Builder::visit(chir::ExprStmt &node){
    node.expr_->accept(*this);
}
void Builder::visit(chir::Assign &node){
    // std::cerr<<"assign                    assign"<<endl;
    // assert(0);
    if(node.rhs_->ty_==type_man->int_liter||node.rhs_->ty_==type_man->float_liter){
	node.rhs_->ty_=node.lhs_->ty_;
    }

    node.rhs_->accept(*this);
    auto v=consumeVal();
    left=true;
    node.lhs_->accept(*this);
    irbuilder_->CreateStore(v, consumeVal());
    left=false;    
}
void Builder::visit(chir::Unary &node){assert(0);}
void Builder::visit(chir::Bin &node){
    node.lhs_->accept(*this);
    auto l=consumeVal();
    node.rhs_->accept(*this);
    auto r=consumeVal();
    llvm::Instruction::BinaryOps op=hir_binop2llvm_binop(node);
    produceVal(this->irbuilder_->CreateBinOp(op,l,r));
}
void Builder::visit(chir::Rel &node){
    node.lhs_->accept(*this);
    auto l=consumeVal();
    node.rhs_->accept(*this);
    auto r=consumeVal();
    // llvm::Instruction::BinaryOps op;
    // this->irbuilder_->CreateBinOp(llvm::Instruction::BinaryOps Opc, Value *LHS, Value *RHS)
    produceVal(this->irbuilder_->CreateCmp(hir_cmpop2llvm_cmpop(node.op,node.lhs_->ty_->type), l, r));
}

void Builder::visit(chir::Or &node){
    // tmp_val=nullptr;
    assert(tmp_value==nullptr);
    node.lhs_->accept(*this);
    auto l=consumeVal();
    // tmp_val=nullptr;
    llvm::BasicBlock* or_false=llvm::BasicBlock::Create(*this->context_,"",cur_func);
    tf_bb.push_back({tf_bb.back().first,or_false});
    
    irbuilder_->CreateCondBr(l, tf_bb.back().first,tf_bb.back().second);
    tf_bb.pop_back();
    irbuilder_->SetInsertPoint(or_false);
    // tmp_val=nullptr;
    assert(tmp_value==nullptr);
    node.rhs_->accept(*this);
    // auto r=std::move(tmp_val);
    // irbuilder_->CreateCondBr(r, tf_bb.back().first,tf_bb.back().second);
    // tf_bb.pop_back();
}

void Builder::visit(chir::And &node){
    assert(tmp_value==nullptr);
    node.lhs_->accept(*this);
    auto l=consumeVal();
    assert(tmp_value==nullptr);
    llvm::BasicBlock* and_true=llvm::BasicBlock::Create(*this->context_,"",cur_func);
    tf_bb.push_back({and_true,tf_bb.back().second});
    irbuilder_->CreateCondBr(l, tf_bb.back().first,tf_bb.back().second);
    tf_bb.pop_back();
    irbuilder_->SetInsertPoint(and_true);
    assert(tmp_value==nullptr);
    node.rhs_->accept(*this);
    // auto r=std::move(tmp_val);
    // irbuilder_->CreateCondBr(r, tf_bb.back().first,tf_bb.back().second);
    // tf_bb.pop_back();
}

void Builder::visit(chir::Call &node){

    ::llvm::Function* func;
    std::vector<llvm::Value*>args;
    if(auto lva=dynamic_cast<chir::Lval*>(node.lhs_.get())){
        // if(cur_hir_func->name_==lva->id_){
        //     func=cur_func;
        // }
        func=this->getfunc(lva->id_);
    }else if(auto selector=dynamic_cast<chir::Selector*>(node.lhs_.get())){
        auto tmp=isselect;
        isselect=true;
        selector->lhs_->accept(*this);
        isselect=tmp;
        args.push_back(consumeVal());
        auto fun=selector->struct_->findFunc(selector->rhs_);
        func=funcsmap.at(fun);
    }
    
    for(auto& arg:node.args_){
        arg->accept(*this);
        args.push_back(consumeVal());
    }
    produceVal(irbuilder_->CreateCall(func,args));
}

void Builder::visit(chir::ThisSuper &node){
    produceVal(cur_func->getArg(0));
}

void Builder::visit(chir::ArrIndex &node){
    auto tmp=left;
    left=true;
    node.lhs_->accept(*this);
    left=tmp;
    auto array_ptr=consumeVal();
    llvm:Value* ptr=nullptr;
    ssize_t size=node.indexs_.size();
    auto ty=node.arr_ty_;
    for(ssize_t i=0;i<size;++i){
        left=false;
        node.indexs_[i]->accept(*this);
        left=tmp;
        auto offset=consumeVal();

        assert(ty->isArray());
        ty=((type::ArrayType const *)ty)->element_;

        auto PtrGEP = irbuilder_->CreateStructGEP(arraytype,array_ptr, 0, "ptr_gep");
        ptr = irbuilder_->CreateLoad(irbuilder_->getPtrTy(), PtrGEP, "ptr");
        array_ptr=irbuilder_->CreateGEP(getType(ty),ptr,{offset});
    }
    if(node.isleft_){
        produceVal(array_ptr);
    }else{
        produceVal(irbuilder_->CreateLoad(getType(node.ty_),array_ptr));
    }
    // auto tmp=left;
    // left=true;
    // node.lhs_->accept(*this);
    // left=tmp;
    // auto l=consumeVal();
    // Value *dataFieldPtr = irbuilder_->CreateStructGEP(this->arraytype, l, 0, "array.left");
    // dataFieldPtr=irbuilder_->CreateLoad(irbuilder_->getPtrTy(),dataFieldPtr);
    // left=false;
    // llvm::Value *elemPtr ;
    // type::Type const * elety=(type::ArrayType const *)node.arr_ty_;
    // ssize_t size=node.indexs_.size();
    // for(ssize_t i=0;i<size;++i){
    //     auto &index=node.indexs_[i];
    //     auto tmpl=left;
    //     left=false;
    //     index->accept(*this);
    //     left=tmpl;
        
    //     auto offset=consumeVal();
    //     if(elety->isArray()){
    //         elety=((type::ArrayType const *)elety)->element_;
    //     }
    //     elemPtr = irbuilder_->CreateGEP(getType(elety), dataFieldPtr,{offset});    
    //     if(i<size-1){
    //         dataFieldPtr = irbuilder_->CreateStructGEP(this->arraytype, elemPtr, 0, "array.left");
    //         dataFieldPtr=irbuilder_->CreateLoad(irbuilder_->getPtrTy(),dataFieldPtr);
    //     }
    // }
    // left=tmp;
    // if(node.isleft_)
    //     produceVal(elemPtr);
    // else
    //     produceVal(irbuilder_->CreateLoad(getType(node.ty_),elemPtr));
}
void Builder::visit(chir::Selector &node){
    {
        auto tmp=isselect;
        isselect=true;
        node.lhs_->accept(*this);
        isselect=tmp;
    }
    auto val=consumeVal();
    if(val->getType()->isPointerTy()){
        // auto s=getStructTy(llvm::cast<llvm::StructType>());
        // auto decl=s->struct_;
        ssize_t i=node.struct_->findVarOffset(node.rhs_);
        if(i!=-1){
            // tmp_val.
            llvm::Value* ptr = irbuilder_->CreateStructGEP(getType(node.struct_->ty_),val, i);
            
            if(left==false){
                produceVal(irbuilder_->CreateLoad(getType(node.struct_->vars_[i]->ty_),ptr));
            }else
            produceVal(ptr);

        }else{
            produceVal(funcsmap.at(node.struct_->findFunc(node.rhs_)));
        }
    }else{
        // auto s=getStructTy(llvm::cast<llvm::StructType>());
        // auto decl=s->struct_;
        ssize_t i=node.struct_->findVarOffset(node.rhs_);
        if(i!=-1){
            // tmp_val.
            produceVal(irbuilder_->CreateExtractValue(val, i));
        }
    }
}
// chir::FuncDecl* Builder::findNearestFunc(std::vector<unique_ptr<chir::Expr>> const&args,vector<unique_ptr<chir::FuncDecl>>const&funcs){
//     if(funcs.empty())
//         return nullptr;
//     std::vector<std::pair<chir::FuncDecl*,int>> dist_func;
//     for(auto& f:funcs){
//         auto size=args.size();
//         if(f->params_.size()!=size)
//             continue;
//         int dis=0;
//         for(ssize_t i=0;i<size;++i){
//             if(f->params_[i].second!=args[i]->ty_){
//                 if(auto iter=this->distmap.find({f->params_[i].second,args[i]->ty_});iter!=distmap.end()){
//                     dis+=iter->second;
//                 }else{
//                     dis=INT_MIN;
//                     break;
//                 }
//             }
//         }
//         if(dis>=0){
//             dist_func.push_back({f.get(),dis});
//         }
//     }
//     if(dist_func.empty()){
//         return nullptr;
//     }
//     auto ret=dist_func.front();
//     for(auto f:dist_func){
//         if(f.second<ret.second)
//             ret=f;
//     }
//     return ret.first;
// }
// chir::Init *findInit(chir::Struct &node,chir::StructDecl&decl){
//     for(auto &init:decl.inits_){
//         size_t size=node.args_.size();
//         if(init->params_.size()!=size){
//             continue;
//         }
//         for(auto i=0;i<size;++i){
//             if(init->params_[i].second!=node.args_[i]->ty_){
//                 continue;
//             }
//         }
//         return init.get();
//     }
//     return nullptr;
// }
void Builder::visit(chir::Struct &node){
    auto ty=getType(node.ty_);
    auto &structdecl=node.decl_;
    if(structdecl.inits_.empty()){
        if(node.args_.empty()==false){
            auto name=structdecl.name_;
            throw std::logic_error("struct"+name +"have no init function but call a init");
        }
    }

    // auto init=findInit(node, structdecl);
    auto init=node.init_;
    // auto init=findNearestFunc(node.args_,structdecl.inits_);
    if(init!=nullptr){
        auto temp_alloca=irbuilder_->CreateAlloca(ty);
        std::vector<llvm::Value*>args{temp_alloca};
        for(auto& param:node.args_){
            param->accept(*this);
            args.push_back(consumeVal());
        }
        irbuilder_->CreateCall(this->getfunc(init->name_),args);
        produceVal(irbuilder_->CreateLoad(ty,temp_alloca));
    }else{
        auto s=this->getLLVMStructTy(node.ty_);
        auto dt=(type::StructType const*)(node.ty_);
        vector<llvm::Constant*>cons;
        for(auto &i:dt->struct_->vars_){
            i->init_->accept(*this);
            if(auto con=llvm::dyn_cast<llvm::Constant>(consumeVal())){
                cons.push_back(con);
            }
        }
        produceVal(llvm::ConstantStruct::get(s,cons));
    }

}

void Builder::visit(chir::Lval &node){
    auto info=this->val_table.findValue(node.id_);
    if(info.defty==DefTy::FUNC_PARAM){
        if(left==false&&isselect==false){
            produceVal(this->irbuilder_->CreateLoad(getType(info.ty), info.val));
        }else
            produceVal(info.val);
        if(left&&(info.val->getType()->isPointerTy()==false)){
            throw std::logic_error("func param cannot assign");
        }else if(isselect){
            // produceVal(info.val);
        }
    }else{
        if(left==false&&isselect==false){
            produceVal(this->irbuilder_->CreateLoad(getType(info.ty), info.val));
        }else
            produceVal(info.val);
    }
    if(left&&info.ismut()==false){
        std::cerr<<(" cannot assign to a immutable variable "+node.id_)<<std::endl;
        exit(EXIT_FAILURE); 
    }
}
void Builder::visit(chir::ArrayLit &node){
    std::vector<llvm::Value*>vec;
    auto size=node.elements_.size();
    auto tmp=left;
    left=false;
    for(auto &e:node.elements_){
        e->accept(*this);
        vec.push_back(consumeVal());
    }
    left=tmp;
    // auto *llvm_arrayTy = llvm::ArrayType::get(getType(node.elements_.front()->ty_), vec.size());
    auto __struct=this->arraytype;
    auto arrAlloca = irbuilder_->CreateAlloca(__struct, nullptr, "arr");
    uint64_t sizeInBytes = layout.getTypeAllocSize(getType(node.elements_.front()->ty_));
    // auto malloc=irbuilder_->CreateCall(module_->getFunction("malloc"),llvm::ConstantInt::get(irbuilder_->getInt64Ty(), size*sizeInBytes));
    auto malloc=this->createMalloc(size*sizeInBytes);
    // irbuilder_.createm
    for (unsigned i = 0; i < vec.size(); ++i) {
        // 获取元素指针
        llvm::Value *elemPtr = irbuilder_->CreateGEP(vec[i]->getType(), malloc,
                            {irbuilder_->getInt32(i)});
        // 存储值
        irbuilder_->CreateStore(vec[i], elemPtr);
    }

    Value *dataFieldPtr = irbuilder_->CreateStructGEP(this->arraytype, arrAlloca, 0, "array.data");
    irbuilder_->CreateStore(malloc, dataFieldPtr);

    Value *sizeFieldPtr = irbuilder_->CreateStructGEP(this->arraytype,arrAlloca, 1, "array.size");
    irbuilder_->CreateStore(ConstantInt::get(irbuilder_->getInt64Ty(),size), sizeFieldPtr);
    if(left)
        produceVal(arrAlloca);
    else{
        produceVal(irbuilder_->CreateLoad(arraytype,arrAlloca));
    }
        //获取arr的类型
    // assert(arrAlloca->getAllocatedType()->isArrayTy());
    // auto arrty=llvm::dyn_cast<llvm::ArrayType>(arrAlloca->getAllocatedType());
    // assert(arrty->getNumElements()==3);
}
void Builder::visit(chir::Lit &node){
    // assert(node.ty_->getSize()!=0);
    if(node.ty_->type==type::TypeId::INT||node.ty_->type==type::TypeId::BOOL){
        int v;
        if(node.ty_->getSize()==1){
            if(node.lit_=="true"){
                v=1;
            }else{
                v=0;
            }
        }else
            v=std::stoll(node.lit_);
	
        uint size=node.ty_->getSize();
        if(size==0){
            size=64;
        }
        produceVal(llvm::ConstantInt::getSigned(irbuilder_->getIntNTy(size), v));
    }else if(node.ty_->type==type::TypeId::UINT){
	    uint a=std::stoull(node.lit_);
        uint size=node.ty_->getSize();
        if(size==0){
            size=64;
        }
	    produceVal(llvm::ConstantInt::get(irbuilder_->getIntNTy(size), a));
    }else if(node.ty_->type==type::TypeId::FLOAT){
	    auto a=std::stod(node.lit_);
	    produceVal(llvm::ConstantFP::get(getType(node.ty_), a));	
    }else{
	assert(0);
    }
}
//   void Builder::visit(chir::IntConst &node){assert(0);}
void Builder::visit(chir::If &node){
    llvm::BasicBlock* t=nullptr,*f=nullptr,*next=nullptr;
        t=llvm::BasicBlock::Create(*this->context_,"",cur_func);
    if(node.else_){
	    f=llvm::BasicBlock::Create(*this->context_,"",cur_func);
	    next=llvm::BasicBlock::Create(*this->context_,"",cur_func);
    }else{
	    f=llvm::BasicBlock::Create(*this->context_,"",cur_func);
	    next=f;
    }
    tf_bb.push_back({t,f});
    llvm::AllocaInst* allo=nullptr;
    if(node.ty_!=nullptr){
        allo=this->irbuilder_->CreateAlloca(getType(node.ty_));
    }
    // tmp_val=nullptr;
    assert(tmp_value==nullptr);
    node.cond_->accept(*this);
    auto tmp=consumeVal();
    if(tmp)
	    irbuilder_->CreateCondBr(tmp,t,f);
    irbuilder_->SetInsertPoint(t);
    node.then_->accept(*this);
    if(allo!=nullptr)
        irbuilder_->CreateStore(consumeVal(),allo);
    {
        auto then_end=irbuilder_->GetInsertBlock();
        if(then_end->back().isTerminator()==false){
            irbuilder_->CreateBr(next);
        }
    }

    if(node.else_){
        irbuilder_->SetInsertPoint(f);
        node.else_->accept(*this);
        if(allo!=nullptr)
            irbuilder_->CreateStore(consumeVal(),allo);
        {
            auto else_end=irbuilder_->GetInsertBlock();
            if(else_end->back().isTerminator()==false){
                irbuilder_->CreateBr(next);
            }
        }
    }
    irbuilder_->SetInsertPoint(next);
    if(allo)
        produceVal(irbuilder_->CreateLoad(allo->getAllocatedType(),allo,"if"));
}
static vector<std::pair<BasicBlock*, BasicBlock*>> while_cond_next_stack;
void Builder::visit(chir::While &node){
    llvm::BasicBlock* t=nullptr,*f=nullptr;
    auto cond_block=llvm::BasicBlock::Create(*this->context_,"",cur_func);
    t=llvm::BasicBlock::Create(*this->context_,"",cur_func);
    f=llvm::BasicBlock::Create(*this->context_,"",cur_func);
    tf_bb.push_back({t,f});
    while_cond_next_stack.push_back({cond_block,f});
    // auto size=tf_bb.size();
    irbuilder_->CreateBr(cond_block);
    irbuilder_->SetInsertPoint(cond_block);
    node.cond_->accept(*this);
    auto tmp=consumeVal();
    if(tmp)
	    irbuilder_->CreateCondBr(tmp,t,f);
    irbuilder_->SetInsertPoint(t);
    node.loop_->accept(*this);
    {
        auto loop_end=irbuilder_->GetInsertBlock();
        if(loop_end->back().isTerminator()==false){
            irbuilder_->CreateBr(cond_block);
        }
    }

    // irbuilder_->CreateBr(f);
    irbuilder_->SetInsertPoint(f);
}
void Builder::visit(chir::Block &node){
    size_t size=node.stmts_.size();
    for(auto i=0;i<size;++i){
        auto &stmt=node.stmts_[i];
	    stmt->accept(*this);
        if(auto expr_stmt=dynamic_cast<chir::ExprStmt*>(stmt.get())){
            // if(expr_stmt->ret_==false){
            //     // assert(tmp_value==nullptr);
            //     tmp_value=nullptr;
            // }
            if(i<size-1){
                tmp_value=nullptr;
            }
            // if(dynamic_cast<chir::Ret*>(expr_stmt->expr_.get())!=nullptr&&node.bty_!=ScopeType::FUNC){       
            //     irbuilder_->SetInsertPoint(BasicBlock::Create(*context_,"after_ret",cur_func));
            // }
        }
    }
    if(node.bty_==ScopeType::FUNC&&node.stmts_.empty()==false){
        auto stmt=node.stmts_.back().get();
        if(auto expr=dynamic_cast<chir::ExprStmt*>(stmt)){
            if(dynamic_cast<chir::Ret*>(expr->expr_.get())){

            }else if(expr->ret_&&expr->expr_->ty_==cur_hir_func->ret_ty_){
                if(tmp_value!=nullptr)
                    irbuilder_->CreateStore(consumeVal(),ret_of_cur_func);
            
            }
        }
    }
}
void Builder::visit(chir::RetStmt &node){
    assert(0);
    // if(node.expr_){
    //     // irbuilder_->SetInsertPoint(::llvm::BasicBlock::Create(*context_,"",cur_func));
    //     node.expr_->accept(*this);
    //     auto ret=consumeVal();
    //     irbuilder_->CreateStore(ret, ret_of_cur_func);
    // }
    // irbuilder_->CreateBr(ret_bb_of_cur_func);
    // irbuilder_->SetInsertPoint(ret_bb_of_cur_func);
    
}

void Builder::visit(chir::Jump &node) {
    auto tmp=while_cond_next_stack.back();
    llvm::BasicBlock* next;
    if(node.is_continue_){
        next=tmp.first;
    }else{
        next=tmp.second;
    }
    irbuilder_->CreateBr(next);
} 

void Builder::visit(chir::Ret &node) {
    if(node.expr_){
        // irbuilder_->SetInsertPoint(::llvm::BasicBlock::Create(*context_,"",cur_func));
        node.expr_->accept(*this);
        auto ret=consumeVal();
        irbuilder_->CreateStore(ret, ret_of_cur_func);
    }
    irbuilder_->CreateBr(ret_bb_of_cur_func);
    // irbuilder_->SetInsertPoint(BasicBlock::Create(*context_,"after_ret",cur_func));
} 
void Builder::visit(chir::DeclStmt &node) {
    node.decl_->accept(*this);
}

void Builder::visit(chir::IncDec &node) {
    assert(0);
}

void Builder::visit(chir::WCDecl &node) {
    assert(0);
}
void Builder::visit(chir::NumConv &node) {
    node.expr_->accept(*this);
    if(node.ty_==node.expr_->ty_){
        return;
    }
    auto tmp_val=consumeVal();
    auto dest_ty=getType(node.ty_);
    // if(auto ity=llvm::dyn_cast<llvm::IntegerType>(val->getType())){
    //     if(ity->getBitWidth()<node.ty_->getSize()){
    //     }
    // }
    size_t size=0;
    if(node.expr_->ty_->getSize()<node.ty_->getSize()){
        if(node.expr_->ty_->isUInt()){
            tmp_val=irbuilder_->CreateCast(llvm::Instruction::CastOps::ZExt,tmp_val, dest_ty);
        }else if(node.expr_->ty_->isSInt()){
            tmp_val=irbuilder_->CreateCast(llvm::Instruction::CastOps::SExt,tmp_val, dest_ty);            
        }else if(node.expr_->ty_->isFloat()){
            tmp_val=irbuilder_->CreateCast(llvm::Instruction::CastOps::FPExt,tmp_val, dest_ty);            
        }
        size=node.ty_->getSize();
    }else if(node.expr_->ty_->getSize()>node.ty_->getSize()){
        if(node.expr_->ty_->isUInt()){
            tmp_val=irbuilder_->CreateCast(llvm::Instruction::CastOps::Trunc,tmp_val, dest_ty);
        }else if(node.expr_->ty_->isSInt()){
            tmp_val=irbuilder_->CreateCast(llvm::Instruction::CastOps::Trunc,tmp_val, dest_ty);            
        }else if(node.expr_->ty_->isFloat()){
            tmp_val=irbuilder_->CreateCast(llvm::Instruction::CastOps::FPTrunc,tmp_val, dest_ty);            
        }
        size=node.expr_->ty_->getSize();
    }else{
        size=node.ty_->getSize();
    }

    if(node.expr_->ty_->isUInt()){
        if(node.ty_->isUInt()){
        }else if(node.ty_->isSInt()){
            // llvm::Instruction::CastOps op=llvm::Instruction::CastOps::sitoe;
            tmp_val=irbuilder_->CreateCast(llvm::Instruction::SExt,tmp_val, getType(node.ty_));
        }else if(node.ty_->isFloat()){
            tmp_val=irbuilder_->CreateCast(llvm::Instruction::CastOps::UIToFP,tmp_val, getType(node.ty_));
        }
    }else if(node.expr_->ty_->isSInt()){
        if(node.ty_->isUInt()){
            tmp_val=irbuilder_->CreateCast(llvm::Instruction::CastOps::ZExt,tmp_val, getType(node.ty_));
        }else if(node.ty_->isSInt()){
        }else if(node.ty_->isFloat()){
            tmp_val=irbuilder_->CreateCast(llvm::Instruction::CastOps::SIToFP,tmp_val, getType(node.ty_));
        }
    }else  if(node.expr_->ty_->isFloat()){
        if(node.ty_->isUInt()){
            tmp_val=irbuilder_->CreateCast(llvm::Instruction::FPToUI,tmp_val, getType(node.ty_));
        }else if(node.ty_->isSInt()){
            tmp_val=irbuilder_->CreateCast(llvm::Instruction::FPToSI,tmp_val, getType(node.ty_));
        }else if(node.ty_->isFloat()){
        }
    }
    produceVal(tmp_val);
}

void Builder::visit(chir::Init &node){
    clearfunc();
    Scopes.push_back(ScopeType::FUNC);
    val_table.enter();
    // llvm::Type* retty=getType(node.parent_->ty_)->getPointerTo();

    llvm::Type* retty=irbuilder_->getVoidTy();
    llvm::Type* struct_ty=getType(node.parent_->ty_);
    llvm::PointerType* ponit_ty=struct_ty->getPointerTo();

    std::vector<llvm::Type*> args{ ponit_ty};
    for(auto iter:node.params_){
        args.push_back(getType(iter.second));
    }
    auto func_ty=llvm::FunctionType::get(retty, args,false);
    cur_func=::llvm::Function::Create(func_ty, llvm::Function::GlobalValue::ExternalLinkage, node.name_, *this->module_);


    entry_bb_of_cur_func= llvm::BasicBlock::Create(*context_,"",cur_func);
    cur_bb_of_cur_func=entry_bb_of_cur_func;
    irbuilder_->SetInsertPoint(entry_bb_of_cur_func);
    // ret_of_cur_func=irbuilder_->CreateAlloca(getType(node.parent_->ty_));
    auto _this=cur_func->getArg(0);
    
    // val_table.insert({"this",IRInfo{DefTy::FUNC_PARAM,_this,node.parent_->ty_,_this->getType()}});
    
    // val_table.insert({node.params_[0].first,IRInfo{DefTy::FUNC_PARAM,cur_func->getArg(0),node.params_[1].second,cur_func->getArg(i)->getType()}});
    for(size_t i=0;i<node.params_.size();++i){
        auto arg_addr=irbuilder_->CreateAlloca(cur_func->getArg(i+1)->getType());
        irbuilder_->CreateStore( cur_func->getArg(i+1),arg_addr);
        val_table.insert({node.params_[i].first,IRInfo{DefTy::FUNC_PARAM,arg_addr,node.params_[i].second,cur_func->getArg(i+1)->getType()}});
    }
    // for(size_t i=0;i<node.params_.size();++i){
    //     val_table.insert({node.params_[i].first,IRInfo{DefTy::FUNC_PARAM,cur_func->getArg(i+1),node.params_[i].second,cur_func->getArg(i)->getType()}});
    // }

    auto &vars=node.parent_->vars_;
    // assert(getType(node.parent_->ty_)->getPointerTo()==_this->getType());
    for(size_t i=0;i<vars.size();++i){
        if(vars[i]->init_){
            vars[i]->init_->accept(*this);
            llvm::Value* xPtr = irbuilder_->CreateStructGEP(struct_ty, _this, i);
            irbuilder_->CreateStore( consumeVal(), xPtr);
        }
    }


    ret_bb_of_cur_func= llvm::BasicBlock::Create(*context_,"",cur_func);

    node.block_->accept(*this);
    auto b=irbuilder_->GetInsertBlock();
    if(b->back().isTerminator()==false){
        irbuilder_->CreateBr(ret_bb_of_cur_func);
    }
    // if(ret_of_cur_func){
        irbuilder_->SetInsertPoint(ret_bb_of_cur_func);
        // irbuilder_->CreateRet(ret_of_cur_func);
        irbuilder_->CreateRetVoid();
        // }
    val_table.exit();
    Scopes.pop_back();
}
// void Builder::genIR(ast::CompunitNode*node){
//     this->visit(*node);
// }

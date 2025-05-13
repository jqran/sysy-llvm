#ifndef BUILDER_HPP
#define BUILDER_HPP
#include "midend/type.hpp"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include <cstdint>
#include <llvm-19/llvm/IR/Constants.h>
#include <llvm-19/llvm/IR/DataLayout.h>
#include <llvm-19/llvm/IR/DerivedTypes.h>
#include <llvm-19/llvm/IR/Function.h>
#include <llvm-19/llvm/IR/Instructions.h>
#include <map>
#include <set>
#include <utility>
#include <vector>
#include "midend/hir.hpp"
#include "scope.hpp"
using llvm::LLVMContext;
using llvm::Module;
using llvm::IRBuilder;

class Builder:public chir::HirVisitor{
    LLVMContext* const context_;
    Module* const module_;
    IRBuilder<>* const irbuilder_;
    TableManager val_table,func_table;
    unique_ptr<type::TypeManager>type_man;
    ::llvm::Type*  getType(::type::Type const* ty);
    ::llvm::Type* arraytype;
    llvm::DataLayout layout=llvm::DataLayout("e-p:64:64:64-i64:64-v128:64:64-a:0:64-n8:16:32:64");

    // chir::FuncDecl* findNearestFunc(std::vector<unique_ptr<chir::Expr>> const&args,vector<unique_ptr<chir::FuncDecl>>const&funcs);
    std::vector<std::pair<::type::Type const *, ::llvm::StructType*>> struct_tys_{};
    llvm::CallInst* createMalloc(int64_t size){
        return irbuilder_->CreateCall(module_->getFunction("malloc"), llvm::ConstantInt::get(irbuilder_->getInt64Ty(),size));
    }
    ::llvm::StructType*getLLVMStructTy(type::Type const *ty){
        for(auto i:struct_tys_){
            if(i.first==ty){
                return i.second;
            }
        }
        return nullptr;
    }
    ::type::Type const *getStructTy(::llvm::StructType const *ty){
        for(auto i:struct_tys_){
            if(i.second==ty){
                return i.first;
            }
        }
        return nullptr;
    }
    llvm::Function* findNearestFunc(string name,std::vector<unique_ptr<chir::Expr>> const&args);
    std::pair<chir::FuncDecl*, llvm::Function*> findBestMatch(
    const chir::Call& call,
    const std::vector<std::pair<chir::FuncDecl*, llvm::Function*>>& candidates
    );
    std::pair<chir::FuncDecl*, llvm::Function*>getfunc(chir::Call *call);
    virtual void visit(chir::Module &node) ;
    virtual void visit(chir::FuncDecl &node) ;
    virtual void visit(chir::StructDecl&node) ;
    virtual void visit(chir::EnumDecl&node) ;
    // virtual void visit(chir::ValDeclStmt &node) ;
    virtual void visit(chir::VarDecl &node) ;
    virtual void visit(chir::ExprStmt &node) ;
    virtual void visit(chir::Assign &node) ;
    virtual void visit(chir::Unary &node) ;
    virtual void visit(chir::IncDec &node) ;
    // virtual void visit(chir::SelectorExpr &node) ;
    virtual void visit(chir::Bin &node) ;
    virtual void visit(chir::Rel &node) ;
    virtual void visit(chir::Or &node) ;
    virtual void visit(chir::And &node) ;
    virtual void visit(chir::Lval &node) ;
    virtual void visit(chir::ThisSuper &node) ;
    virtual void visit(chir::ArrIndex &node) ;
    virtual void visit(chir::Selector &node) ;
    virtual void visit(chir::Call &node) ;
    virtual void visit(chir::Struct &node) ;
    virtual void visit(chir::ArrayLit &node) ;
    virtual void visit(chir::Lit &node) ;
    // virtual void visit(chir::IntConst &node) ;
    virtual void visit(chir::If &node) ; 
    virtual void visit(chir::WCDecl &node) ;
    virtual void visit(chir::Jump &node) ; 
    virtual void visit(chir::Ret &node) ; 
    virtual void visit(chir::NumConv &node) ;
    virtual void visit(chir::While &node) ;
    virtual void visit(chir::Block &node) ;
    virtual void visit(chir::Init &node) ;
    virtual void visit(chir::DeclStmt &node)  ; 
    // virtual void visit(chir::FloatConst &node) ;
    // virtual void visit(chir::AssignStmt &node) ;
    virtual void visit(chir::RetStmt &node) ;
    // virtual void visit(chir::ContinueStmt &node) ;
    // virtual void visit(chir::BreakStmt &node) ;
    // virtual void visit(chir::EmptyStmt &node) ;


public:
    void genIR(chir::Module*node){this->visit(*node);}
    // Builder(Checker&&checker):type_man(checker.moveTypeMan()),scope(nullptr){}
    Builder(::llvm::LLVMContext *const context,::llvm::IRBuilder<> *const irbuilder,::llvm::Module* const module, unique_ptr<type::TypeManager> tyman):context_(context) ,irbuilder_(irbuilder),module_(module) ,type_man(std::move(tyman)),val_table(),func_table(){}
};
#endif

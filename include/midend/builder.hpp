#ifndef BUILDER_HPP
#define BUILDER_HPP
#include "checker.hpp"
#include "frontend/node.hpp"
#include "frontend/type.hpp"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "midend/hir.hpp"
#include "scope.hpp"
using llvm::LLVMContext;
using llvm::Module;
using llvm::IRBuilder;

class Builder:public cjir::HirVisitor{
    LLVMContext* const context_;
    Module* const module_;
    IRBuilder<>* const irbuilder_;
    IRScope val_table,func_table;
    unique_ptr<type::TypeManager>type_man;
    ::llvm::Type*  getType(::type::Type const* ty);

    virtual void visit(cjir::Module &node) ;
    virtual void visit(cjir::FuncDecl &node) ;
    virtual void visit(cjir::StructDecl&node) ;
    // virtual void visit(cjir::ValDeclStmt &node) ;
    virtual void visit(cjir::VarDecl &node) ;
    virtual void visit(cjir::ExprStmt &node) ;
    virtual void visit(cjir::Assign &node) ;
    virtual void visit(cjir::Unary &node) ;
    // virtual void visit(cjir::SelectorExpr &node) ;
    virtual void visit(cjir::Bin &node) ;
    virtual void visit(cjir::Rel &node) ;
    virtual void visit(cjir::Or &node) ;
    virtual void visit(cjir::And &node) ;
    virtual void visit(cjir::Lval &node) ;
    virtual void visit(cjir::Lit &node) ;
    // virtual void visit(cjir::IntConst &node) ;
    virtual void visit(cjir::If &node) ; 
    virtual void visit(cjir::While &node) ;
    virtual void visit(cjir::Block &node) ;
    // virtual void visit(cjir::FloatConst &node) ;
    // virtual void visit(cjir::AssignStmt &node) ;
    virtual void visit(cjir::RetStmt &node) ;
    // virtual void visit(cjir::ContinueStmt &node) ;
    // virtual void visit(cjir::BreakStmt &node) ;
    // virtual void visit(cjir::EmptyStmt &node) ;


public:
    void genIR(cjir::Module*node){this->visit(*node);}
    // Builder(Checker&&checker):type_man(checker.moveTypeMan()),scope(nullptr){}
    Builder(::llvm::LLVMContext *const context,::llvm::IRBuilder<> *const irbuilder,::llvm::Module* const module, Checker&&checker):context_(context) ,irbuilder_(irbuilder),module_(module) ,type_man(checker.moveTypeMan()),val_table(nullptr),func_table(nullptr){}
};
#endif

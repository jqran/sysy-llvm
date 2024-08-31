#ifndef CODEGEN_HPP
#define CODEGEN_HPP
#include "frontend/node.hpp"
#include "scope.hpp"
#include <cassert>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
namespace gen {
class CodeGen:public ::ast::ASTVisitor{    
private:
    ::llvm::Module  * const module_;
    ::llvm::IRBuilder<>* const builder_;
    ::llvm::LLVMContext * const context_;

    
    std::map<string,::llvm::Function*> func_table;
    Scope* val_table;

    ::llvm::Type*  getType(::type::ValType &);
    void enterScope(){
        auto tmp=val_table;
        val_table->enter();
    }
    void exitScope(){
        auto tmp=val_table->getParent();
        val_table=tmp;
        assert(val_table!=0);
    }

private:
    virtual void visit(ast::CompunitNode &node) override;
    virtual void visit(ast::FuncFParam &node) override;
    virtual void visit(ast::FuncDef &node) override;
    virtual void visit(ast::ValDeclStmt &node) override;
    virtual void visit(ast::ValDefStmt &node) override;
    virtual void visit(ast::ArrDefStmt &node) override;
    virtual void visit(ast::ConstDeclStmt &node)override;
    virtual void visit(ast::ConstDefStmt &node)override;
    virtual void visit(ast::ConstArrDefStmt &node) override;
    virtual void visit(ast::ExprStmt &node) override;
    virtual void visit(ast::AssignStmt &node) override;
    virtual void visit(ast::PrefixExpr &node) override;
    virtual void visit(ast::AssignExpr &node) override;
    virtual void visit(ast::AndExp &node) override;
    virtual void visit(ast::ORExp &node)override;
    virtual void visit(ast::RelopExpr &node) override;
    virtual void visit(ast::EqExpr &node) override;
    virtual void visit(ast::BinopExpr &node) override;
    virtual void visit(ast::LvalExpr &node)override;
    virtual void visit(ast::IntConst &node) override;
    virtual void visit(ast::InitializerExpr &node) override;
    virtual void visit(ast::FloatConst &node)override;
    virtual void visit(ast::BlockStmt &node) override;
    virtual void visit(ast::IfStmt &node) override;
    virtual void visit(ast::WhileStmt &node)override;
    virtual void visit(ast::CallExpr &node) override;
    virtual void visit(ast::RetStmt &node) override;
    virtual void visit(ast::ContinueStmt &node)override;
    virtual void visit(ast::BreakStmt &node) override;
    virtual void visit(ast::EmptyStmt &node) override;

public:
    CodeGen(::llvm::Module *m,::llvm::IRBuilder<> *builder,::llvm::LLVMContext * context):
    module_(m),
    builder_(builder),
    context_(context),
    val_table(new Scope()){}


};

}
#endif
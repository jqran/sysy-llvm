#include "frontend/node.hpp"
#include "frontend/type.hpp"
#include "midend/hir.hpp"
#include <array>
#include <cstddef>
#include <vector>
// struct Scope{
//     // enum Type{

//     // };
//     ast::BlockExpr*const block;
//     std::map<std::string,type::Type const*> defs;
//     ScopeType const type;
//     Scope(ast::BlockExpr* block,ScopeType type):block(block),type(type){}
// };
enum class ScopeType:int{
    GLOBAL=1,
    FUNC,
    STRUCT,
    IF,
    ELSE,
    LOOP,
    TRY,
    CATCH,
    EMPTY,
};
class Checker:public ast::ASTVisitor{
    std::vector<std::pair<std::map<std::string,type::Type const*>,ScopeType>> scopes;
    unique_ptr<type::TypeManager> type_man;
    std::array<type::BuiltinType*, 10> builtin;
    cjir::Module*module_=nullptr;
    virtual void visit(ast::CompunitNode &node) ;
    virtual void visit(ast::FuncFParam &node) ;
    virtual void visit(ast::FuncDef &node) ;
    virtual void visit(ast::StructDeclStmt&node) ;
    // virtual void visit(ast::ValDeclStmt &node) ;
    virtual void visit(ast::ValDefStmt &node) ;
    virtual void visit(ast::ArrDefStmt &node) ;
    // virtual void visit(ast::ConstDeclStmt &node) ;
    // virtual void visit(ast::ConstDefStmt &node) ;
    // virtual void visit(ast::ConstArrDefStmt &node) ;
    virtual void visit(ast::ExprStmt &node) ;
    virtual void visit(ast::AssignStmt &node) ;
    virtual void visit(ast::PrefixExpr &node) ;
    virtual void visit(ast::SelectorExpr &node) ;
    // virtual void visit(ast::InfixExpr &node) ;
    virtual void visit(ast::AssignExpr &node) ;
    virtual void visit(ast::RelopExpr &node) ;
    virtual void visit(ast::EqExpr &node) ;
    virtual void visit(ast::AndExp &node) ;
    virtual void visit(ast::ORExp &node) ;
    virtual void visit(ast::BinopExpr &node) ;
    virtual void visit(ast::LvalExpr &node) ;
    virtual void visit(ast::Literal &node) ;
    // virtual void visit(ast::IntConst &node) ;
    virtual void visit(ast::InitializerExpr &node) ;
    virtual void visit(ast::IfExpr &node) ; 
    virtual void visit(ast::WhileExpr &node) ;
    virtual void visit(ast::BlockExpr &node) ;
    // virtual void visit(ast::FloatConst &node) ;
    // virtual void visit(ast::AssignStmt &node) ;
    virtual void visit(ast::BlockStmt &node) ;
    // virtual void visit(ast::IfStmt &node) ;
    virtual void visit(ast::WhileStmt &node) ;
    virtual void visit(ast::CallExpr &node) ;
    virtual void visit(ast::RetStmt &node) ;
    virtual void visit(ast::ContinueStmt &node) ;
    virtual void visit(ast::BreakStmt &node) ;
    virtual void visit(ast::EmptyStmt &node) ;
public:
	Checker();
	type::Type const * findDef(string&s);
	unique_ptr<type::TypeManager> moveTypeMan(){return std::move(this->type_man);}
private:
	type::Type const * get_binexpr_type(type::Type const *const lhs,type::Type const *const rhs);
	bool auto_type_conversion(type::Type const *const type,type::Type const *const target_type);
};

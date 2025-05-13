#include "midend/AntlrChecker.hpp"
#include "CangjieParser.h"
#include "midend/scope.hpp"
#include "midend/type.hpp"
#include "midend/hir.hpp"
#include "support/Any.h"
#include <algorithm>
#include <cassert>
#include <cstddef>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <set>
#include <stack>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <utility>
#include <variant>
#include <vector>

using ExprDecls=std::vector<std::unique_ptr<chir::Stmt>>;
using std::make_unique ,::std::unique_ptr;
static  std::stack<ExprDecls> ExprDeclsS;
static chir::FuncDecl  *cur_func=nullptr;
static chir::StructDecl *cur_struct=nullptr;
static unique_ptr<chir::Expr>  tmp_expr=nullptr;
static unique_ptr<chir::Decl>  tmp_decl=nullptr;
static unique_ptr<chir::Stmt>  tmp_stmt=nullptr;
static type::Type const *tmp_type=nullptr;
static type::Type const *target_type=nullptr;
static type::Type const * bool_type=nullptr;
static ::std::stack<chir::Block*> blocks{};
static ::std::map<chir::Expr*,vector<chir::Expr*>> parent_lits;
static ::std::map<chir::Decl*,type::Type const *>decl_tys;
static ::std::vector<type::Type const*>cur_func_tys;
static type::Type const *  INT_LIT_TY=nullptr;
static type::Type const *  FLOAT_LIT_TY=nullptr;
static std::set<chir::Expr*> cur_func_lit;
std::set<chir::Decl*>has_no_ty;
// template <typename T1,typename T2>
// static void produce(T1& var,T2 val){
//     assert(var==nullptr);
//     assert(val!=nullptr);
//     var=std::move(val);
// }
// template <typename T>
// static  T consume(T &var){
//     assert(var!=nullptr);
//     auto ret=std::move(var);
//     var=nullptr;
//     return ret;
// }
auto CangjieChecker::findDecl(string id){
    auto ret= scope_man_.findDecl(id);
    if(ret!=nullptr){
        return ret;
    }else if(cur_func->name_==id){
        return cur_func->ty_;
    }else if(cur_struct->name_==id){
        return cur_struct->ty_;
    }else{
        return ret;
    }
}
static  unique_ptr<chir::Stmt> consumeStmt(){
    assert(tmp_stmt!=nullptr);
    return std::move(tmp_stmt);
}

static  unique_ptr<chir::Decl> consumeDecl(){
    assert(tmp_decl!=nullptr);
    return std::move(tmp_decl);
}
static  unique_ptr<chir::Expr> consumeExpr(){
    assert(tmp_expr!=nullptr);
    return std::move(tmp_expr);
}
static  auto consumeType(){
    assert(tmp_type!=nullptr);
    auto ty=tmp_type;
    tmp_type=0;
    return ty;
}

static void produceStmt(unique_ptr<chir::Stmt> stmt){
    assert(tmp_stmt==nullptr);
    swap(tmp_stmt,stmt);
}

static void  produceDecl(unique_ptr<chir::Decl> decl){
    assert(tmp_decl==nullptr);
    swap(tmp_decl,decl);
}
static void  produceExpr(unique_ptr<chir::Expr> expr){
    assert(tmp_expr==nullptr);
    std::swap(tmp_expr,expr);
}
static void produceType(type::Type const* ty){
    assert(tmp_type==nullptr);
    std::swap(tmp_type,ty);
}


template  <typename target_ty>
bool is_ty(chir::Node  *const ptr){
    return static_cast<target_ty*>(ptr);
}


template <typename To, typename From>
std::unique_ptr<To> unique_dynamic_cast(std::unique_ptr<From>& from) {
    if (To* cast = dynamic_cast<To*>(from.get())) {
        from.release();
        return std::unique_ptr<To>(cast);
    }
    return nullptr;
}
type::Type const* CangjieChecker::find(chir::Expr* expr){
    if(auto lval=dynamic_cast<chir::Lval*>(expr)){
        return findDecl(lval->id_);
    }else if(auto sele=dynamic_cast<chir::Selector*>(expr)){
        type::Type const* sty=find(sele->lhs_.get());
        auto ty=(type::StructType const* )(sty);
        return ty->struct_->find(sele->rhs_)->ty_;
    }else if(auto _this=dynamic_cast<chir::ThisSuper*>(expr)){
        return cur_struct->ty_;
    }else{
        throw std::logic_error("unknow expression");
    }
}
void CangjieChecker::CheckBin(chir::Bin*bin){
    if(bin->lhs_->ty_==type_man->int_liter||bin->lhs_->ty_==type_man->float_liter){
        if(bin->rhs_->ty_!=bin->lhs_->ty_){
            assert(bin->rhs_->ty_->type==bin->lhs_->ty_->type);
            bin->lhs_->ty_=bin->rhs_->ty_;
        }
    }else if(bin->rhs_->ty_==type_man->int_liter||bin->rhs_->ty_==type_man->float_liter){
        if(bin->rhs_->ty_!=bin->lhs_->ty_){
            assert(bin->rhs_->ty_->type==bin->lhs_->ty_->type);
            bin->rhs_->ty_=bin->lhs_->ty_;
        }
    }else {
        assert(bin->lhs_->ty_==bin->rhs_->ty_);
    }
}

chir::FuncDecl* CangjieChecker::findNearestFunc(std::vector<unique_ptr<chir::Expr>> const&args,vector<unique_ptr<chir::FuncDecl>>const&funcs){
    if(funcs.empty())
        return nullptr;
    std::vector<std::pair<chir::FuncDecl*,int>> dist_func;
    for(auto& f:funcs){
        auto size=args.size();
        if(f->params_.size()!=size)
            continue;
        int dis=0;
        for(ssize_t i=0;i<size;++i){
            if(f->params_[i].second!=args[i]->ty_){
                auto d=this->type_man->findDist(f->params_[i].second,args[i]->ty_);
                if(d<0){break;}
                else{dis+=d;}
            }
        }
        if(dis>=0){
            dist_func.push_back({f.get(),dis});
        }
    }
    if(dist_func.empty()){
        return nullptr;
    }
    auto ret=dist_func.front();
    for(auto f:dist_func){
        if(f.second<ret.second)
            ret=f;
    }

    auto size=args.size();
    for(ssize_t i=0;i<size;++i){
        args[i]->ty_=ret.first->params_[i].second;
    }

    return ret.first;
}


//return str;
antlrcpp::Any CangjieChecker::visitIdentifier(CangjieParser::IdentifierContext *context) {
    return context->Identifier()->toString();
}

antlrcpp::Any CangjieChecker::visitTranslationUnit(CangjieParser::TranslationUnitContext *context) {
    this->type_man->module_=this->module_;
    this->scope_man_.insert({"geti64",type_man->getFuncTy(type_man->getI64(), {})});
    this->scope_man_.insert({"geti32",type_man->getFuncTy(type_man->getI32(), {})});
    this->scope_man_.insert({"geti16",type_man->getFuncTy(type_man->getI16(), {})});
    this->scope_man_.insert({"geti8",type_man->getFuncTy(type_man->getI8(), {})});
    this->scope_man_.insert({"getf64",type_man->getFuncTy(type_man->getF64(), {})});
    this->scope_man_.insert({"getf32",type_man->getFuncTy(type_man->getF32(), {})});
    
    this->scope_man_.insert({"puti64",type_man->getFuncTy(type_man->getUnit(),{type_man->getI64()})});
    this->scope_man_.insert({"puti32",type_man->getFuncTy(type_man->getUnit(),{type_man->getI32()})});
    this->scope_man_.insert({"puti16",type_man->getFuncTy(type_man->getUnit(),{type_man->getI16()})});
    this->scope_man_.insert({"puti8",type_man->getFuncTy(type_man->getUnit(),{type_man->getI8()})});
    this->scope_man_.insert({"putf64",type_man->getFuncTy(type_man->getUnit(),{type_man->getF64()})});
    this->scope_man_.insert({"putf32",type_man->getFuncTy(type_man->getUnit(),{type_man->getF32()})});


    std::vector<CangjieParser::TopLevelObjectContext *> tops=context->topLevelObject();
    for(auto top:tops){
        top->accept(this);
        this->module_->defs_.push_back(consumeDecl());
    }
    auto maindef=context->mainDefinition();
    if(maindef){
        maindef->accept(this);
        auto main=consumeDecl();
        module_->main_=unique_dynamic_cast<chir::FuncDecl>(main);
    }

    return nullptr;
}

antlrcpp::Any CangjieChecker::visitEnd(CangjieParser::EndContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitPreamble(CangjieParser::PreambleContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitPackageHeader(CangjieParser::PackageHeaderContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitPackageNameIdentifier(CangjieParser::PackageNameIdentifierContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitImportList(CangjieParser::ImportListContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitImportAllOrSpecified(CangjieParser::ImportAllOrSpecifiedContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitImportSpecified(CangjieParser::ImportSpecifiedContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitImportAll(CangjieParser::ImportAllContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitImportAlias(CangjieParser::ImportAliasContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitTopLevelObject(CangjieParser::TopLevelObjectContext *context) {
    if(auto func=context->functionDefinition()){
        func->accept(this);
    }else if(auto var=context->variableDeclaration()){
        var->accept(this);
    }else if(auto struct_decl=context->structDefinition()){
        struct_decl->accept(this);
    }else if(auto _enum=context->enumDefinition()){
        _enum->accept(this);
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitClassDefinition(CangjieParser::ClassDefinitionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitSuperClassOrInterfaces(CangjieParser::SuperClassOrInterfacesContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitClassModifierList(CangjieParser::ClassModifierListContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitClassModifier(CangjieParser::ClassModifierContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitTypeParameters(CangjieParser::TypeParametersContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitSuperClass(CangjieParser::SuperClassContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitClassType(CangjieParser::ClassTypeContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitTypeArguments(CangjieParser::TypeArgumentsContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitSuperInterfaces(CangjieParser::SuperInterfacesContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitInterfaceType(CangjieParser::InterfaceTypeContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitGenericConstraints(CangjieParser::GenericConstraintsContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitUpperBounds(CangjieParser::UpperBoundsContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitClassBody(CangjieParser::ClassBodyContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitClassMemberDeclaration(CangjieParser::ClassMemberDeclarationContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitClassInit(CangjieParser::ClassInitContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitStaticInit(CangjieParser::StaticInitContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitClassPrimaryInit(CangjieParser::ClassPrimaryInitContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitClassName(CangjieParser::ClassNameContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitClassPrimaryInitParamLists(CangjieParser::ClassPrimaryInitParamListsContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitClassUnnamedInitParamList(CangjieParser::ClassUnnamedInitParamListContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitClassNamedInitParamList(CangjieParser::ClassNamedInitParamListContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitClassUnnamedInitParam(CangjieParser::ClassUnnamedInitParamContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitClassNamedInitParam(CangjieParser::ClassNamedInitParamContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitClassNonStaticMemberModifier(CangjieParser::ClassNonStaticMemberModifierContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitInterfaceDefinition(CangjieParser::InterfaceDefinitionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitInterfaceBody(CangjieParser::InterfaceBodyContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitInterfaceMemberDeclaration(CangjieParser::InterfaceMemberDeclarationContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitInterfaceModifierList(CangjieParser::InterfaceModifierListContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitInterfaceModifier(CangjieParser::InterfaceModifierContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitFunctionDefinition(CangjieParser::FunctionDefinitionContext *context) {
    antlrcpp::Any const id=context->identifier()->accept(this);
    auto name=id.as<string>();
    
    type::Type const* ty;

    if(auto __ty=context->type()){
        __ty->accept(this);
        ty=consumeType();
    }else{
        ty=type_man->getUnit();
    }
    
    scope_man_.enter();


    std::vector<std::pair<string,type::Type const*>> params=std::move(context->functionParameters()->accept(this)
    .as<std::vector<std::pair<string,type::Type const*>>>());
    unique_ptr<chir::FuncDecl> ufunc;
    if(cur_struct!=nullptr){
        auto func=std::make_unique<chir::MemFunc>(type_man.get(),ty,name,std::move(params));
        func->id_=ty->name_+cur_struct->name_+func->name_;
        func->parent_=cur_struct;
        ufunc=std::move(func);
    }else{
        ufunc=std::make_unique<chir::FuncDecl>(type_man.get(),ty,name,std::move(params));
    }
    cur_func=ufunc.get();
    
    if(cur_struct!=nullptr){
        cur_struct->funcs_.push_back(std::move(ufunc));
    }

    auto b=context->block();
    b->accept(this);
    auto expr=consumeExpr();
    auto block=unique_dynamic_cast<chir::Block>(expr);
    if(block){
        block->bty_=ScopeType::FUNC;
        cur_func->block_=std::move(block);
    }else{assert(0);}

    scope_man_.exit();
    scope_man_.insert({cur_func->name_,cur_func->ty_});
    if(cur_struct==nullptr)
        produceDecl(std::move(ufunc));
    cur_func=nullptr;
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitOperatorFunctionDefinition(CangjieParser::OperatorFunctionDefinitionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitFunctionParameters(CangjieParser::FunctionParametersContext *context) {
    std::vector<std::pair<string,type::Type const*>>params{};
    auto paramlist=context->unnamedParameterList();
    if(paramlist==nullptr){
        return params;
    }
    auto vec_p=paramlist->unnamedParameter();
    if(vec_p.empty()){
        return params;
    }
    for(auto n:vec_p){
        // n->accept(this);
        auto id=n->identifier()->accept(this).as<string>();
        n->type()->accept(this);
        auto ty=consumeType();
        params.push_back({id,ty});
        scope_man_.insert({id,ty});
    }
    assert(context->namedParameterList()==nullptr);
    return params;
}

antlrcpp::Any CangjieChecker::visitNondefaultParameterList(CangjieParser::NondefaultParameterListContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitUnnamedParameterList(CangjieParser::UnnamedParameterListContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitUnnamedParameter(CangjieParser::UnnamedParameterContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitNamedParameterList(CangjieParser::NamedParameterListContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitNamedParameter(CangjieParser::NamedParameterContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitDefaultParameter(CangjieParser::DefaultParameterContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitFunctionModifierList(CangjieParser::FunctionModifierListContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitFunctionModifier(CangjieParser::FunctionModifierContext *context) {assert(0);}

using PatternsMaybe=std::variant<string>;
antlrcpp::Any CangjieChecker::visitVariableDeclaration(CangjieParser::VariableDeclarationContext *context) {
    auto p=context->patternsMaybeIrrefutable()->accept(this);
    PatternsMaybe pattern=p.as<PatternsMaybe>();
    string id=std::get<string>(pattern);
    type::Type const *ty;
    if(auto _ty=context->type()){
        _ty->accept(this);
        ty=consumeType();
    }else{
        ty=nullptr;
    }
    unique_ptr<chir::Expr> init=nullptr;
    if(auto epxr=context->expression()){
        epxr->accept(this);
        init=consumeExpr();
    }
    auto get_mutable=[context](){
        if(context->CONST())
            return DefTy::CONST;
        else if(context->LET()){
            return DefTy::LET;
        }else{
            return DefTy::VAR;
        }
    };
    auto decl=make_unique<chir::VarDecl>(ty,id,std::move(init),get_mutable());
    decl->check(*type_man);
    if(id!="_"){
        this->insertDecl(decl.get());
    }
    // decl->check(*type_man);
    produceDecl(std::move(decl));
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitVariableModifier(CangjieParser::VariableModifierContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitEnumDefinition(CangjieParser::EnumDefinitionContext *context) {
    auto id=context->identifier()->accept(this).as<string>();
    auto decl=make_unique<chir::EnumDecl>(nullptr,id);
    type_man->addEnumType(decl.get());
    auto body=context->enumBody();
    for(auto _case:body->caseBody()){
        auto __case=_case->accept(this).as<std::pair<string,vector<type::Type const*>>>();
        decl->contexts_.push_back(__case);
    }
    produceDecl(std::move(decl));
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitEnumBody(CangjieParser::EnumBodyContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitCaseBody(CangjieParser::CaseBodyContext *context) {
    std::pair<string, vector<type::Type const*>> ret;
    auto id=context->identifier()->accept(this).as<string>();
    auto tys=context->type();
    std::vector<type::Type const*> types;
    for(auto ty:tys ){
        ty->accept(this);
        types.push_back(consumeType());
    }
    ret={id,std::move(types)};
    return ret;
}

antlrcpp::Any CangjieChecker::visitEnumModifier(CangjieParser::EnumModifierContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitStructDefinition(CangjieParser::StructDefinitionContext *context) {
    auto id=context->identifier()->accept(this).as<string>();
    scope_man_.enter();
    auto body=context->structBody();
    // std::vector<std::pair<string, unique_ptr<chir::Decl>>> decl=body->accept(this).as<std::vector<std::pair<string, unique_ptr<chir::Decl>>>>();
    auto sdecl=make_unique<chir::StructDecl>(nullptr,id);
    type_man->addStructType(sdecl.get());
    scope_man_.insertGlobal(sdecl.get());
    cur_struct=sdecl.get();

    for(auto decl:context->structBody()->structMemberDeclaration()){
        decl->accept(this);
        if(tmp_decl==nullptr){
            continue;
        }
        auto tmp=std::move(consumeDecl());
        auto name=tmp->name_;
        if(auto va=unique_dynamic_cast<chir::VarDecl>(tmp)){
            sdecl->vars_.push_back(std::move(va)); 
        }else if(auto func=unique_dynamic_cast<chir::FuncDecl>(tmp)){
            if(auto init=unique_dynamic_cast<chir::Init>(func)){
                init->parent_=sdecl.get();
                init->name_=sdecl->name_+init->name_;
                sdecl->inits_.push_back(std::move(init));
            }else
                sdecl->funcs_.push_back(std::move(func)); 
        }
    }
    scope_man_.exit();
    cur_struct=nullptr;
    produceDecl(std::move(sdecl));
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitStructBody(CangjieParser::StructBodyContext *context) {
    // auto decls=context->structMemberDeclaration();
    // for(auto decl:decls){
    //     decl->accept(this);
    // }
    // return nullptr
    assert(0);
}

antlrcpp::Any CangjieChecker::visitStructMemberDeclaration(CangjieParser::StructMemberDeclarationContext *context) {
    if(auto var=context->variableDeclaration()){
        var->accept(this);
    }else if(auto fn=context->functionDefinition()){
        fn->accept(this);
    }else if(auto init=context->structInit()){
        init->accept(this);
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitStructInit(CangjieParser::StructInitContext *context) {
    scope_man_.enter();
    auto params=context->functionParameters()->accept(this).as<std::vector<std::pair<string,type::Type const*>>>();

    context->block()->accept(this);
    auto expr=consumeExpr();
    // auto block=unique_dynamic_cast<chir::Block>(expr);
    auto name=context->INIT()->getText();
    for(auto param:params){
        name+=param.second->name_;
    }

    auto init=make_unique<chir::Init>(name,params);
    init->block_=unique_dynamic_cast<chir::Block>(expr);
    produceDecl(std::move(init));
    scope_man_.exit();
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitStructPrimaryInit(CangjieParser::StructPrimaryInitContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitStructName(CangjieParser::StructNameContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitStructPrimaryInitParamLists(CangjieParser::StructPrimaryInitParamListsContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitStructUnnamedInitParamList(CangjieParser::StructUnnamedInitParamListContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitStructNamedInitParamList(CangjieParser::StructNamedInitParamListContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitStructUnnamedInitParam(CangjieParser::StructUnnamedInitParamContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitStructNamedInitParam(CangjieParser::StructNamedInitParamContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitStructModifier(CangjieParser::StructModifierContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitStructNonStaticMemberModifier(CangjieParser::StructNonStaticMemberModifierContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitTypeAlias(CangjieParser::TypeAliasContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitTypeModifier(CangjieParser::TypeModifierContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitExtendDefinition(CangjieParser::ExtendDefinitionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitExtendType(CangjieParser::ExtendTypeContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitExtendBody(CangjieParser::ExtendBodyContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitExtendMemberDeclaration(CangjieParser::ExtendMemberDeclarationContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitForeignDeclaration(CangjieParser::ForeignDeclarationContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitForeignBody(CangjieParser::ForeignBodyContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitForeignMemberDeclaration(CangjieParser::ForeignMemberDeclarationContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitAnnotationList(CangjieParser::AnnotationListContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitAnnotation(CangjieParser::AnnotationContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitAnnotationArgumentList(CangjieParser::AnnotationArgumentListContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitAnnotationArgument(CangjieParser::AnnotationArgumentContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitMacroDefinition(CangjieParser::MacroDefinitionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitMacroWithoutAttrParam(CangjieParser::MacroWithoutAttrParamContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitMacroWithAttrParam(CangjieParser::MacroWithAttrParamContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitMacroInputDecl(CangjieParser::MacroInputDeclContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitMacroAttrDecl(CangjieParser::MacroAttrDeclContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitPropertyDefinition(CangjieParser::PropertyDefinitionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitPropertyBody(CangjieParser::PropertyBodyContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitPropertyMemberDeclaration(CangjieParser::PropertyMemberDeclarationContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitPropertyModifier(CangjieParser::PropertyModifierContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitMainDefinition(CangjieParser::MainDefinitionContext *context) {
    type::Type const* ty;

    if(auto __ty=context->type()){
        __ty->accept(this);
        ty=consumeType();
    }else{
        ty=type_man->getUnit();
    }
    auto ufunc=std::make_unique<chir::FuncDecl>(type_man.get(),ty,"main",std::vector<std::pair<string,type::Type const*>>{});
    cur_func=ufunc.get();

    // auto params=;
    auto params=context->functionParameters()->accept(this).as<std::vector<std::pair<string,type::Type const*>>>();
    
    scope_man_.enter();

    auto b=context->block();
    b->accept(this);
    auto expr=consumeExpr();
    auto block=unique_dynamic_cast<chir::Block>(expr);
    if(block){
        block->bty_=ScopeType::FUNC;
        ufunc->block_=std::move(block);
    }else{assert(0);}
    cur_func=nullptr;
    scope_man_.exit();
    produceDecl(std::move(ufunc));
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitType(CangjieParser::TypeContext *context) {
    auto atty=context->atomicType();
    atty->accept(this);
    // auto ty=consume(tmp_type);
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitArrowType(CangjieParser::ArrowTypeContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitArrowParameters(CangjieParser::ArrowParametersContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitTupleType(CangjieParser::TupleTypeContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitPrefixType(CangjieParser::PrefixTypeContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitPrefixTypeOperator(CangjieParser::PrefixTypeOperatorContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitAtomicType(CangjieParser::AtomicTypeContext *context) {
    auto char_lang_ty=context->charLangTypes();
    if(char_lang_ty){
        char_lang_ty->accept(this);
    }else if(auto user_ty=context->userType();user_ty!=nullptr){
        user_ty->accept(this);
    }else if(auto ps_ty=context->parenthesizedType();ps_ty!=nullptr){
        ps_ty->accept(this);
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitCharLangTypes(CangjieParser::CharLangTypesContext *context) {
    auto num_ty=context->numericTypes();
    if(num_ty){
        num_ty->accept(this);
    }else if(auto unit=context->UNIT()){
        produceType(type_man->getUnit());
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitNumericTypes(CangjieParser::NumericTypesContext *context) {
    if(context->INT8()){
        produceType(this->type_man->getI8());
    }else if(context->INT16()){
        produceType(this->type_man->getI16());
    }else if(context->INT32()){
        produceType(this->type_man->getI32());
    }else if(context->INT64()){
        produceType(this->type_man->getI64());
    }else if(context->INTNATIVE()){
        // tmp_type=this->type_man->getI64();
        assert(0);
    }else if(context->UINT8()){
        produceType(this->type_man->getU8());
    }else if(context->UINT16()){
        produceType(this->type_man->getU16());
    }else if(context->UINT32()){
        produceType(this->type_man->getU32());
    }else if(context->UINT64()){
        produceType(this->type_man->getU64());
    }else if(context->UINTNATIVE()){
        // tmp_type=this->type_man->getI64();
        assert(0);
    }else if(context->FLOAT16()){
        produceType(this->type_man->getF16());
    }else if(context->FLOAT32()){
        produceType(this->type_man->getF32());
    }else if(context->FLOAT64()){
        produceType(this->type_man->getF64());
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitUserType(CangjieParser::UserTypeContext *context) {
    auto id=context->identifier().front()->accept(this).as<string>();
    if(auto arg=context->typeArguments()){
        arg->type().front()->accept(this);
        auto ty=consumeType();
        produceType(type_man->getArrayTy(ty));
    }else
        produceType(type_man->getType(id));
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitParenthesizedType(CangjieParser::ParenthesizedTypeContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitExpression(CangjieParser::ExpressionContext *context) {
    context->assignmentExpression()->accept(this);
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitAssignmentExpression(CangjieParser::AssignmentExpressionContext *context) {
    // context->
    if(auto lval=context->leftValueExpression()){
        lval->accept(this);
        auto expr=consumeExpr();
        if(auto flow=context->flowExpression()){
            flow->accept(this);
        }
        produceExpr(make_unique<chir::Assign>(std::move(expr),std::move(consumeExpr())));
    }else if(auto lval_w=context->leftValueExpressionWithoutWildCard()){
        lval_w->accept(this);
        auto expr=consumeExpr();
        if(auto flow=context->flowExpression()){
            flow->accept(this);
        }
        produceExpr(make_unique<chir::Assign>(std::move(expr),std::move(consumeExpr())));
    }else if(auto flow=context->flowExpression()){
        flow->accept(this);
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitTupleLeftValueExpression(CangjieParser::TupleLeftValueExpressionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitLeftValueExpression(CangjieParser::LeftValueExpressionContext *context) {
    if(auto wc=context->WILDCARD()){
        assert(0);
    }else
        context->leftValueExpressionWithoutWildCard()->accept(this);
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitLeftValueExpressionWithoutWildCard(CangjieParser::LeftValueExpressionWithoutWildCardContext *context) {
    if(auto aux=context->leftAuxExpression()){
        // assert(context->assignableSuffix()==nullptr);
        aux->accept(this);
        if(auto suff=context->assignableSuffix()){
            suff->accept(this);
        }
    }else{
        auto str=context->identifier()->accept(this).as<string>();
        produceExpr(make_unique<chir::Lval>(str,this->findDecl(str)));
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitLeftAuxExpression(CangjieParser::LeftAuxExpressionContext *context) {
    if(auto aux=context->leftAuxExpression()){
        aux->accept(this);

        if(auto __id=context->identifier()){
            auto id=__id->accept(this).as<string>();
            produceExpr(make_unique<chir::Selector>(consumeExpr(),id,true));
        }else if(auto _index=context->indexAccess()){
        auto lhs=consumeExpr();
            _index->expression()->accept(this);
            auto index=consumeExpr();
            if(auto l=unique_dynamic_cast<chir::ArrIndex>(lhs)){
                l->addIndex(std::move(index));
                produceExpr(std::move(l));
            }else{
                auto arr=make_unique<chir::ArrIndex>(lhs->ty_,std::move(lhs),std::move(index),true);
                produceExpr(std::move(arr));
            }
        }else{
            assert(0);
        }
    }else if(auto __id=context->identifier()){
        auto id=__id->accept(this).as<string>();
        produceExpr(make_unique<chir::Lval>(id,this->findDecl(id)));
    }else if(auto this_super=context->thisSuperExpression()){
        this_super->accept(this);
    }else{
        assert(0);
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitAssignableSuffix(CangjieParser::AssignableSuffixContext *context) {
    if(auto field=context->fieldAccess()){
        auto id=context->fieldAccess()->identifier()->accept(this).as<string>();
        produceExpr(make_unique<chir::Selector>(consumeExpr(),id,true));
    }else if(auto _index=context->indexAccess()){
        auto lhs=consumeExpr();
        _index->expression()->accept(this);
        auto index=consumeExpr();
        if(auto l=unique_dynamic_cast<chir::ArrIndex>(lhs)){
            l->addIndex(std::move(index));
            produceExpr(std::move(l));
        }else{
            auto arr=make_unique<chir::ArrIndex>(lhs->ty_,std::move(lhs),std::move(index),true);
            produceExpr(std::move(arr));
        }
    }

    return nullptr;
}

antlrcpp::Any CangjieChecker::visitFieldAccess(CangjieParser::FieldAccessContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitFlowExpression(CangjieParser::FlowExpressionContext *context) {
    auto coalescings=context->coalescingExpression();
    assert(coalescings.size()==1);
    coalescings.front()->accept(this);
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitCoalescingExpression(CangjieParser::CoalescingExpressionContext *context) {
    auto logicdis=context->logicDisjunctionExpression();
    logicdis.front()->accept(this);
    if(logicdis.size()==2){
        assert(0);
        auto lhs=consumeExpr();
        logicdis[1]->accept(this);
        auto rhs=consumeExpr();
        // produceExpr(make_unique<chir::Or>(chir::Bin::LOR,std::move(lhs),std::move(rhs)));
    }else{
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitLogicDisjunctionExpression(CangjieParser::LogicDisjunctionExpressionContext *context) {
    auto exprs=context->logicConjunctionExpression();
    exprs.front()->accept(this);
    auto ops=context->OR();
    ssize_t op_size=ops.size();
    auto last=consumeExpr();
    for(ssize_t i=0;i<op_size;++i){
        auto op=ops[i]->accept(this).as<chir::Bin::Binop>();
        exprs[i+1]->accept(this);
        auto new_expr=consumeExpr();
        last=make_unique<chir::Or>(type_man->getBool(),std::move(last),std::move(new_expr)) ;
    }
    produceExpr(std::move(last));
    // if(exprs.size()==2){
    //     auto lhs=consumeExpr();
    //     exprs[1]->accept(this);
    //     auto rhs=consumeExpr();
    //     produceExpr(make_unique<chir::And>(context->shiftingOperator()->accept(this).as<chir::Bin::Binop>(),std::move(lhs),std::move(rhs)));
    // }else{
        
    // }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitLogicConjunctionExpression(CangjieParser::LogicConjunctionExpressionContext *context) {    
    auto exprs=context->rangeExpression();
    exprs.front()->accept(this);
    auto shift_ops=context->AND();
    ssize_t op_size=shift_ops.size();
    auto last=consumeExpr();
    for(ssize_t i=0;i<op_size;++i){
        // auto op=shift_ops[i]->accept(this).as<chir::Bin::Binop>();
        exprs[i+1]->accept(this);
        auto new_expr=consumeExpr();
        last=make_unique<chir::And>(type_man->getBool(),std::move(last),std::move(new_expr)) ;
    }
    produceExpr(std::move(last));
    // if(exprs.size()==2){
    //     auto lhs=consumeExpr();
    //     exprs[1]->accept(this);
    //     auto rhs=consumeExpr();
    //     produceExpr(make_unique<chir::And>(context->shiftingOperator()->accept(this).as<chir::Bin::Binop>(),std::move(lhs),std::move(rhs)));
    // }else{
        
    // }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitRangeExpression(CangjieParser::RangeExpressionContext *context) {
    auto bw=context->bitwiseDisjunctionExpression();
    assert(bw.size()==1);
    bw.front()->accept(this);
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitBitwiseDisjunctionExpression(CangjieParser::BitwiseDisjunctionExpressionContext *context) {
    auto exprs=context->bitwiseXorExpression();
    exprs.front()->accept(this);
    auto shift_ops=context->BITOR();
    ssize_t op_size=shift_ops.size();
    auto last=consumeExpr();
    for(ssize_t i=0;i<op_size;++i){
        // auto op=shift_ops[i]->accept(this).as<chir::Bin::Binop>();
        exprs[i+1]->accept(this);
        auto new_expr=consumeExpr();
        auto temp=make_unique<chir::Bin>(chir::Bin::BIT_OR,std::move(last),std::move(new_expr)) ;
        CheckBin(temp.get());
        last=std::move(temp);
    }
    produceExpr(std::move(last));
    // if(exprs.size()==2){
    //     auto lhs=consumeExpr();
    //     exprs[1]->accept(this);
    //     auto rhs=consumeExpr();
    //     produceExpr(make_unique<chir::And>(context->shiftingOperator()->accept(this).as<chir::Bin::Binop>(),std::move(lhs),std::move(rhs)));
    // }else{
        
    // }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitBitwiseXorExpression(CangjieParser::BitwiseXorExpressionContext *context) {
    auto exprs=context->bitwiseConjunctionExpression();
    exprs.front()->accept(this);
    auto shift_ops=context->BITXOR();
    ssize_t op_size=shift_ops.size();
    auto last=consumeExpr();
    for(ssize_t i=0;i<op_size;++i){
        // auto op=shift_ops[i]->accept(this).as<chir::Bin::Binop>();
        exprs[i+1]->accept(this);
        auto new_expr=consumeExpr();
        auto temp=make_unique<chir::Bin>(chir::Bin::BIT_XOR,std::move(last),std::move(new_expr)) ;
        CheckBin(temp.get());
        last=std::move(temp);
    }
    produceExpr(std::move(last));
    // if(exprs.size()==2){
    //     auto lhs=consumeExpr();
    //     exprs[1]->accept(this);
    //     auto rhs=consumeExpr();
    //     produceExpr(make_unique<chir::And>(context->shiftingOperator()->accept(this).as<chir::Bin::Binop>(),std::move(lhs),std::move(rhs)));
    // }else{
        
    // }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitBitwiseConjunctionExpression(CangjieParser::BitwiseConjunctionExpressionContext *context) {    
    auto exprs=context->equalityComparisonExpression();
    exprs.front()->accept(this);
    auto shift_ops=context->BITAND();
    ssize_t op_size=shift_ops.size();
    auto last=consumeExpr();
    for(ssize_t i=0;i<op_size;++i){
        // auto op=shift_ops[i]->accept(this).as<chir::Bin::Binop>();
        exprs[i+1]->accept(this);
        auto new_expr=consumeExpr();
        auto temp=make_unique<chir::Bin>(chir::Bin::BIT_AND,std::move(last),std::move(new_expr)) ;
        CheckBin(temp.get());
        last=std::move(temp);
    }
    produceExpr(std::move(last));
    // if(exprs.size()==2){
    //     auto lhs=consumeExpr();
    //     exprs[1]->accept(this);
    //     auto rhs=consumeExpr();
    //     produceExpr(make_unique<chir::And>(context->shiftingOperator()->accept(this).as<chir::Bin::Binop>(),std::move(lhs),std::move(rhs)));
    // }else{
        
    // }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitEqualityComparisonExpression(CangjieParser::EqualityComparisonExpressionContext *context) {
    auto exprs=context->comparisonOrTypeExpression();
    exprs.front()->accept(this);
    if(exprs.size()==2){
        auto lhs=consumeExpr();
        exprs[1]->accept(this);
        auto rhs=consumeExpr();
        produceExpr(make_unique<chir::Rel>(type_man->getBool(),context->equalityOperator()->accept(this).as<chir::Bin::Binop>(),std::move(lhs),std::move(rhs)));
    }else{
        
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitComparisonOrTypeExpression(CangjieParser::ComparisonOrTypeExpressionContext *context) {    
    auto exprs=context->shiftingExpression();
    exprs.front()->accept(this);
    if(exprs.size()==2){
        auto lhs=consumeExpr();
        exprs[1]->accept(this);
        auto rhs=consumeExpr();
        produceExpr(make_unique<chir::Rel>(type_man->getBool(),context->comparisonOperator()->accept(this).as<chir::Bin::Binop>(),std::move(lhs),std::move(rhs)));
    }else{
        
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitShiftingExpression(CangjieParser::ShiftingExpressionContext *context) {
    auto exprs=context->additiveExpression();
    exprs.front()->accept(this);
    auto shift_ops=context->shiftingOperator();
    ssize_t op_size=shift_ops.size();
    auto last=consumeExpr();
    for(ssize_t i=0;i<op_size;++i){
        auto op=shift_ops[i]->accept(this).as<chir::Bin::Binop>();
        exprs[i+1]->accept(this);
        auto new_expr=consumeExpr();
        auto temp=make_unique<chir::Bin>(op,std::move(last),std::move(new_expr)) ;
        CheckBin(temp.get());
        last=std::move(temp);
    }
    produceExpr(std::move(last));
    // if(exprs.size()==2){
    //     auto lhs=consumeExpr();
    //     exprs[1]->accept(this);
    //     auto rhs=consumeExpr();
    //     produceExpr(make_unique<chir::And>(context->shiftingOperator()->accept(this).as<chir::Bin::Binop>(),std::move(lhs),std::move(rhs)));
    // }else{
        
    // }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitAdditiveExpression(CangjieParser::AdditiveExpressionContext *context) {
    auto exprs=context->multiplicativeExpression();
    exprs.front()->accept(this);
    auto add_ops=context->additiveOperator();
    ssize_t op_size=add_ops.size();
    auto last=consumeExpr();
    for(ssize_t i=0;i<op_size;++i){
        auto op=add_ops[i]->accept(this).as<chir::Bin::Binop>();
        exprs[i+1]->accept(this);
        auto new_expr=consumeExpr();
        auto temp=make_unique<chir::Bin>(op,std::move(last),std::move(new_expr)) ;
        CheckBin(temp.get());
        last=std::move(temp);
    }
    produceExpr(std::move(last));
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitMultiplicativeExpression(CangjieParser::MultiplicativeExpressionContext *context) {
    auto exprs=context->exponentExpression();
    exprs.front()->accept(this);
    auto mul_ops=context->multiplicativeOperator();
    ssize_t op_size=mul_ops.size();
    auto last=consumeExpr();
    for(ssize_t i=0;i<op_size;++i){
        auto op=mul_ops[i]->accept(this).as<chir::Bin::Binop>();
        exprs[i+1]->accept(this);
        auto new_expr=consumeExpr();
        auto temp=make_unique<chir::Bin>(op,std::move(last),std::move(new_expr)) ;
        CheckBin(temp.get());
        last=std::move(temp);
    }
    produceExpr(std::move(last));
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitExponentExpression(CangjieParser::ExponentExpressionContext *context) {
    auto exprs=context->prefixUnaryExpression();
    exprs.front()->accept(this);
    auto exp_ops=context->exponentOperator();
    ssize_t op_size=exp_ops.size();
    auto last=consumeExpr();
    for(ssize_t i=0;i<op_size;++i){
        auto op=exp_ops[i]->accept(this).as<chir::Bin::Binop>();
        exprs[i+1]->accept(this);
        auto new_expr=consumeExpr();
        auto temp=make_unique<chir::Bin>(op,std::move(last),std::move(new_expr)) ;
        CheckBin(temp.get());
        last=std::move(temp);
    }
    produceExpr(std::move(last));
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitPrefixUnaryExpression(CangjieParser::PrefixUnaryExpressionContext *context) {
    auto inc=context->incAndDecExpression();
    inc->accept(this);
    auto ops=context->prefixUnaryOperator();
    ssize_t op_size=ops.size();
    auto last=consumeExpr();
    for(ssize_t i=0;i<op_size;++i){
        auto op=ops[i]->accept(this).as<chir::Unary::UnOp>();
        // auto new_expr=consumeExpr();
        last=make_unique<chir::Unary>(op,std::move(last)) ;
    }
    produceExpr(std::move(last));
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitIncAndDecExpression(CangjieParser::IncAndDecExpressionContext *context) {
    context->postfixExpression()->accept(this);
    if(context->INC()){
        produceExpr(make_unique<chir::IncDec>(true,consumeExpr()));
    }else if(context->DEC()){
        produceExpr(make_unique<chir::IncDec>(false,consumeExpr()));
    }else{
    }
    return  nullptr;
}

antlrcpp::Any CangjieChecker::visitPostfixExpression(CangjieParser::PostfixExpressionContext *context) {
    auto procCall=[this](string &id,type::Type const*decl,CangjieParser::CallSuffixContext*_call){
        auto args=_call->valueArgument();
        if(decl->type==type::TypeId::FUNC){
            auto func=(type::FuncType const*)decl;
            auto call=make_unique<chir::Call>(func->ret_ty_,make_unique<chir::Lval>(id,func));
            for(auto arg:args){
                arg->accept(this);
                call->args_.push_back(consumeExpr());
            }
            produceExpr(std::move(call));
        }else if(decl->type==type::TypeId::STRUCT){
            auto _struct=make_unique<chir::Struct>(*((type::StructType const*)decl)->struct_);
            for(auto arg:args){
                arg->accept(this);
                _struct->args_.push_back(consumeExpr());
            }
            _struct->init_=findNearestFunc(_struct->args_,((type::StructType const*)decl)->struct_->inits_);
            produceExpr(std::move(_struct));
        }else{
            assert(0);
        }

    };
    auto q=context->questSeperatedItems();
    if(auto atomic=context->atomicExpression())
        atomic->accept(this);
    else if(auto post=context->postfixExpression()){
        post->accept(this);
        if(auto _call=context->callSuffix()){
            auto expr=consumeExpr();
            // if()
            auto ident=dynamic_cast<chir::Lval*>(expr.get());
            if(ident){
                auto id=ident->id_;
                expr.reset();
                auto decl=this->findDecl(id);
                procCall(id,decl,_call);
            }else{
                auto ty=find(expr.get());
                if(ty->isFunc()){
                    auto fty=(type::FuncType const*)ty;
                    ty=fty->ret_ty_;
                }else{
                    throw  std::logic_error("require function ty");
                }
                auto call=make_unique<chir::Call>(ty,std::move(expr));
                auto args=_call->valueArgument();
                for(auto arg:args){
                    arg->accept(this);
                    call->args_.push_back(consumeExpr());
                }
                produceExpr(std::move(call));
            }
            
        }else if(auto _index=context->indexAccess()){
            auto expr=consumeExpr();    
            _index->expression()->accept(this);
            auto index=consumeExpr();
            if(auto l=unique_dynamic_cast<chir::ArrIndex>(expr)){
                l->addIndex(std::move(index));
                produceExpr(std::move(l));
            }else{
                auto arr=make_unique<chir::ArrIndex>(expr->ty_,std::move(expr),std::move(index),false);
                produceExpr(std::move(arr));
            }
        }else if(auto _id=context->identifier()){
            auto lhs=consumeExpr();
            produceExpr(make_unique<chir::Selector>(std::move(lhs),_id->accept(this).as<string>(),false));
        }else if(q.empty()==false){
            assert(0);
        }else{
            std::cerr<<context->toStringTree()<<std::endl;
            throw std::logic_error("");
        }
    }else if(auto __id=context->identifier()){
        auto id=__id->accept(this).as<string>();
        auto decl=findDecl(id);
        if(auto _call=context->callSuffix()){
            procCall(id,decl,_call);
        }
    }

    return  nullptr;

}

antlrcpp::Any CangjieChecker::visitQuestSeperatedItems(CangjieParser::QuestSeperatedItemsContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitQuestSeperatedItem(CangjieParser::QuestSeperatedItemContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitItemAfterQuest(CangjieParser::ItemAfterQuestContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitCallSuffix(CangjieParser::CallSuffixContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitValueArgument(CangjieParser::ValueArgumentContext *context) {
    if(auto expr=context->expression()){
        expr->accept(this);
    }else if(auto id=context->identifier()){
        auto str=id->accept(this).as<string>();
        produceExpr(make_unique<chir::Lval>(str,findDecl(std::move(str))));
    }else{
        assert(0);
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitRefTransferExpression(CangjieParser::RefTransferExpressionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitIndexAccess(CangjieParser::IndexAccessContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitRangeElement(CangjieParser::RangeElementContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitAtomicExpression(CangjieParser::AtomicExpressionContext *context) {
    if(auto id=context->identifier()){
        auto str=id->accept(this).as<std::string>();
        produceExpr(make_unique<chir::Lval>(str,findDecl(str)));
    }else if(auto lit=context->literalConstant()){
        lit->accept(this);
    }else if(auto _if=context->ifExpression()){
        _if->accept(this);
    }else if(auto loop=context->loopExpression()){
        loop->accept(this);
    }else if(auto jump=context->jumpExpression()){
        jump->accept(this);
    }else if(auto conv=context->numericTypeConvExpr()){
        conv->accept(this);
    }else if(auto p=context->parenthesizedExpression()){
        p->accept(this);
    }else if(auto c=context->collectionLiteral()){
        c->accept(this);
    }else if(auto _this=context->thisSuperExpression()){
        _this->accept(this);
    }else{
        auto s=context->toString();
        assert(0);
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitLiteralConstant(CangjieParser::LiteralConstantContext *context) {
    if(auto ilit=context->IntegerLiteral()){
        produceExpr(make_unique<chir::Lit>(ilit->toString(),type_man->int_liter));
    }else if(auto flit=context->FloatLiteral()){
        produceExpr(make_unique<chir::Lit>(flit->toString(),type_man->float_liter));
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitBooleanLiteral(CangjieParser::BooleanLiteralContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitStringLiteral(CangjieParser::StringLiteralContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitLineStringContent(CangjieParser::LineStringContentContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitLineStringLiteral(CangjieParser::LineStringLiteralContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitLineStringExpression(CangjieParser::LineStringExpressionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitMultiLineStringContent(CangjieParser::MultiLineStringContentContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitMultiLineStringLiteral(CangjieParser::MultiLineStringLiteralContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitMultiLineStringExpression(CangjieParser::MultiLineStringExpressionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitCollectionLiteral(CangjieParser::CollectionLiteralContext *context) {
    context->arrayLiteral()->accept(this);
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitArrayLiteral(CangjieParser::ArrayLiteralContext *context) {
    auto elements=context->elements();
    vector<unique_ptr<chir::Expr>> exprs;
    for(auto e:elements->element()){
        e->accept(this);
        exprs.push_back(consumeExpr());
    }
    produceExpr(make_unique<chir::ArrayLit>(std::move(exprs)));
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitElements(CangjieParser::ElementsContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitElement(CangjieParser::ElementContext *context) {
    if(auto expr=context->expressionElement()){
        expr->accept(this);
    }else {
        throw  std::logic_error("todo ! spread");
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitExpressionElement(CangjieParser::ExpressionElementContext *context) {
    context->expression()->accept(this);
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitSpreadElement(CangjieParser::SpreadElementContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitTupleLiteral(CangjieParser::TupleLiteralContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitUnitLiteral(CangjieParser::UnitLiteralContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitIfExpression(CangjieParser::IfExpressionContext *context) {
    context->expression()->accept(this);
    auto cond=consumeExpr();
    auto blocks=context->block();
    blocks.front()->accept(this);
    auto then=consumeExpr();
    // auto ty=then->ty_;
    if(auto elseexpr=context->ELSE()){
        if(auto _if=context->ifExpression()){
            _if->accept(this);
        }else{
            blocks.back()->accept(this);
        }
        auto else_expr=consumeExpr();
        produceExpr(make_unique<chir::If>(std::move(cond),std::move(then),std::move(else_expr)));
    }else{
        produceExpr(make_unique<chir::If>(std::move(cond),std::move(then),nullptr));
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitDeconstructPattern(CangjieParser::DeconstructPatternContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitMatchExpression(CangjieParser::MatchExpressionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitMatchCase(CangjieParser::MatchCaseContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitPatternGuard(CangjieParser::PatternGuardContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitPattern(CangjieParser::PatternContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitConstantPattern(CangjieParser::ConstantPatternContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitWildcardPattern(CangjieParser::WildcardPatternContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitVarBindingPattern(CangjieParser::VarBindingPatternContext *context) {
    return context->identifier()->accept(this);
}

antlrcpp::Any CangjieChecker::visitTuplePattern(CangjieParser::TuplePatternContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitTypePattern(CangjieParser::TypePatternContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitEnumPattern(CangjieParser::EnumPatternContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitEnumPatternParameters(CangjieParser::EnumPatternParametersContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitLoopExpression(CangjieParser::LoopExpressionContext *context) {
    if(auto _while=context->whileExpression()){
        _while->accept(this);
    }else if(auto do_while=context->doWhileExpression()){
        do_while->accept(this);
    }else{
        assert(0);
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitForInExpression(CangjieParser::ForInExpressionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitPatternsMaybeIrrefutable(CangjieParser::PatternsMaybeIrrefutableContext *context) {
    PatternsMaybe s;
    if(context->wildcardPattern()){
        s="_";
    }else if(auto var=context->varBindingPattern()){
        s=var->accept(this).as<string>();
    }
    return s;
}

antlrcpp::Any CangjieChecker::visitWhileExpression(CangjieParser::WhileExpressionContext *context) {
    context->expression()->accept(this);
    auto cond=consumeExpr();
    scope_man_.enter();
    auto block=context->block()->accept(this);
    auto then=consumeExpr();
    auto ty=then->ty_;
    scope_man_.exit();
    produceExpr(make_unique<chir::While>(ty,std::move(cond),unique_dynamic_cast<chir::Block>(then)));
    assert(then==nullptr);
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitDoWhileExpression(CangjieParser::DoWhileExpressionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitTryExpression(CangjieParser::TryExpressionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitCatchPattern(CangjieParser::CatchPatternContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitExceptionTypePattern(CangjieParser::ExceptionTypePatternContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitResourceSpecifications(CangjieParser::ResourceSpecificationsContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitResourceSpecification(CangjieParser::ResourceSpecificationContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitJumpExpression(CangjieParser::JumpExpressionContext *context) {
    if(auto ret =context->RETURN()){
        if(auto ret_expr=context->expression()){
            ret_expr->accept(this);
            produceExpr(make_unique<chir::Ret>(consumeExpr()));
        }else{
            produceExpr(make_unique<chir::Ret>(nullptr));   
        }
    }else{
        bool is_continue=true;
        if(context->BREAK()){
            is_continue=false;
        }
        produceExpr(make_unique<chir::Jump>(type_man->getUnit(),is_continue));
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitNumericTypeConvExpr(CangjieParser::NumericTypeConvExprContext *context) {
    context->numericTypes()->accept(this);
    auto ty=consumeType();
    context->expression()->accept(this);
    produceExpr(make_unique<chir::NumConv>(ty,consumeExpr()));
    return nullptr;

}

antlrcpp::Any CangjieChecker::visitThisSuperExpression(CangjieParser::ThisSuperExpressionContext *context) {
    produceExpr(make_unique<chir::ThisSuper>(cur_struct,false));
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitLambdaExpression(CangjieParser::LambdaExpressionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitTrailingLambdaExpression(CangjieParser::TrailingLambdaExpressionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitLambdaParameters(CangjieParser::LambdaParametersContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitLambdaParameter(CangjieParser::LambdaParameterContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitSpawnExpression(CangjieParser::SpawnExpressionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitSynchronizedExpression(CangjieParser::SynchronizedExpressionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitParenthesizedExpression(CangjieParser::ParenthesizedExpressionContext *context) {
    auto expr=context->expression();
    expr->accept(this);
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitBlock(CangjieParser::BlockContext *context) {
    auto p=context->parent;
    // if(std::any_cast<CangjieParser::MainDefinitionContext*>(p)||std::any_cast<CangjieParser::FunctionDefinitionContext*>(p)){
    // }
    auto expr_decl=context->expressionOrDeclarations();
    
    expr_decl->accept(this);
    produceExpr(std::make_unique<chir::Block>(std::move(ExprDeclsS.top())));
    ExprDeclsS.pop();
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitUnsafeExpression(CangjieParser::UnsafeExpressionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitExpressionOrDeclarations(CangjieParser::ExpressionOrDeclarationsContext *context) {
    ExprDecls expr_decls;
    auto stmts=context->expressionOrDeclarationAndEnd();

    for(size_t i=0;i<stmts.size();++i){
        auto stmt=stmts[i];
        stmt->accept(this);
        expr_decls.push_back(consumeStmt());
    }
    if(auto expr_decl=context->expressionOrDeclaration()){
        expr_decl->accept(this);
        if(expr_decl->expression()){
            auto expr=consumeExpr();
            if(auto ret=dynamic_cast<chir::Ret*>(expr.get())){
                ret->check(*type_man,cur_func->ret_ty_);
            }else{
                expr->check(*type_man,nullptr);
            }
            
            expr_decls.push_back(std::make_unique<chir::ExprStmt>(std::move(expr),true));
        }else{
            auto decl=consumeDecl();
            if(auto var=dynamic_cast<chir::VarDecl*>(decl.get())){
                var->check(*type_man);
            }
            expr_decls.push_back( std::make_unique<chir::DeclStmt>(std::move(decl)));
        }
    }
    ExprDeclsS.push(std::move(expr_decls));
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitExpressionOrDeclarationAndEnd(CangjieParser::ExpressionOrDeclarationAndEndContext *context){
    auto expr=context->expression();
    if(expr!=nullptr){
        bool ret;
        if(context->end()->NL()){
            ret=true;
        }else{
            ret=false;
        }
        expr->accept(this);
        auto _expr=consumeExpr();
        if(auto ret=dynamic_cast<chir::Ret*>(_expr.get())){
            ret->check(*type_man,cur_func->ret_ty_);
        }else{
            _expr->check(*type_man,nullptr);
        }
        produceStmt(make_unique<chir::ExprStmt>(std::move(_expr),ret));
    }else {
        auto decl=context->varOrfuncDeclaration();
        decl->accept(this);
        auto _decl=consumeDecl();
        if(auto var=dynamic_cast<chir::VarDecl*>(_decl.get())){
            var->check(*type_man);
        }
        produceStmt(make_unique<chir::DeclStmt>(std::move(_decl)));
    }
    
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitExpressionOrDeclaration(CangjieParser::ExpressionOrDeclarationContext *context) {
    auto decl=context->varOrfuncDeclaration();
    if(decl!=nullptr){
        decl->accept(this);
    }else if(auto expr=context->expression();expr!=nullptr){
        expr->accept(this);
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitVarOrfuncDeclaration(CangjieParser::VarOrfuncDeclarationContext *context) {
    auto var_decl=context->variableDeclaration();
    if(var_decl){
        var_decl->accept(this);
    }else if(auto fun_def=context->functionDefinition()){
        fun_def->accept(this);
    }
    return nullptr;
}

antlrcpp::Any CangjieChecker::visitQuoteExpression(CangjieParser::QuoteExpressionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitQuoteExpr(CangjieParser::QuoteExprContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitQuoteParameters(CangjieParser::QuoteParametersContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitQuoteToken(CangjieParser::QuoteTokenContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitQuoteInterpolate(CangjieParser::QuoteInterpolateContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitMacroExpression(CangjieParser::MacroExpressionContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitMacroAttrExpr(CangjieParser::MacroAttrExprContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitMacroInputExprWithoutParens(CangjieParser::MacroInputExprWithoutParensContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitMacroInputExprWithParens(CangjieParser::MacroInputExprWithParensContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitMacroTokens(CangjieParser::MacroTokensContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitAssignmentOperator(CangjieParser::AssignmentOperatorContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitEqualityOperator(CangjieParser::EqualityOperatorContext *context) {
    if(context->EQUAL()){
        return chir::Bin::EQ;
    }else{
        return chir::Bin::NOT_EQ;
    }
}

antlrcpp::Any CangjieChecker::visitComparisonOperator(CangjieParser::ComparisonOperatorContext *context) {
    if(context->LT()){
        return chir::Bin::LT;
    }else if(context->LE()){
        return chir::Bin::LE;
    }else if(context->GT()){
        return chir::Bin::GT;
    }else if(context->GE()){
        return chir::Bin::GE;
    }else{
        assert(0);
    }

}

antlrcpp::Any CangjieChecker::visitShiftingOperator(CangjieParser::ShiftingOperatorContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitFlowOperator(CangjieParser::FlowOperatorContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitAdditiveOperator(CangjieParser::AdditiveOperatorContext *context) {
    if(context->ADD()){
        return chir::Bin::ADD;
    }else{
        return chir::Bin::SUB;
    }
}

antlrcpp::Any CangjieChecker::visitExponentOperator(CangjieParser::ExponentOperatorContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitMultiplicativeOperator(CangjieParser::MultiplicativeOperatorContext *context) {
    if(context->DIV()){
        return chir::Bin::Binop::SLASH;
    }else if(context->MUL()){
        return chir::Bin::Binop::MULTI;
    }else{
        return chir::Bin::Binop::MOD;
    }
}

antlrcpp::Any CangjieChecker::visitPrefixUnaryOperator(CangjieParser::PrefixUnaryOperatorContext *context) {assert(0);}

antlrcpp::Any CangjieChecker::visitOverloadedOperators(CangjieParser::OverloadedOperatorsContext *context) {assert(0);}
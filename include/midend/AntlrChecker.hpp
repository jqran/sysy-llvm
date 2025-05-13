#include "CangjieParser.h"
#include "CangjieVisitor.h"
#include "midend/type.hpp"
#include "midend/hir.hpp"
#include <memory>
#include <utility>
struct HirScope{
    HirScope *const parent_;
    std::vector<std::pair<string,type::Type const*>>decls_;
    std::vector<std::unique_ptr<HirScope>> children_;
    HirScope(HirScope*parent):parent_(parent){}
    type::Type const* findDecl(string id){
        for(auto [s,decl]:decls_){
            if(s==id){
                return decl;
            }
        }
        if(parent_!=nullptr){
            return parent_->findDecl(id);
        }else{
            for(auto [s,decl]:decls_){
                if(s==id){
                    return decl;
                }
            }
            return nullptr;
        }
            
    }
};
struct ScopeManager{
    unique_ptr<HirScope> const global_;
    HirScope* curr_;
    ScopeManager():global_(std::make_unique<HirScope>(nullptr)),curr_(global_.get()){}
    HirScope* enter(){
        auto new_=std::make_unique<HirScope>(curr_);
        auto newp=new_.get();
        curr_->children_.push_back(std::move(new_));
        curr_=newp;
        return curr_;
    }
    void exit(){
        curr_=curr_->parent_;
    }
    type::Type const *findDecl(string id){
        return curr_->findDecl(id);
    }
    void insertGlobal(chir::Decl*decl){
        global_->decls_.push_back({decl->name_,decl->ty_});
    }
    void insertDecl(chir::Decl*decl){
        curr_->decls_.push_back({decl->name_,decl->ty_});
    }
    void insert(std::pair<string,type::Type const*> pair){
        curr_->decls_.push_back(pair);
    }
};
class CangjieChecker : public CangjieVisitor {
public:
    virtual antlrcpp::Any visitTranslationUnit(CangjieParser::TranslationUnitContext *context);    
    CangjieChecker():CangjieVisitor(),module_(new chir::Module),type_man(std::make_unique<type::TypeManager>()){}
    unique_ptr<chir::Module>getmodule(){
        auto module=unique_ptr<chir::Module>(module_);
        module_=nullptr;
        return module;
    }
    unique_ptr<type::TypeManager>getTypeMan(){
        return std::move(type_man);
    }
private:
    unique_ptr<chir::Selector> makeSelector(unique_ptr<chir::Expr> l,string id,bool isleft);
    chir::FuncDecl* findNearestFunc(std::vector<unique_ptr<chir::Expr>> const&args,vector<unique_ptr<chir::FuncDecl>>const&funcs);

    ScopeManager scope_man_;
    auto findDecl(string id);
    type::Type const* find(chir::Expr* expr);
    void insertDecl(chir::Decl*decl){scope_man_.insertDecl(decl);}
    HirScope* enter(){return scope_man_.enter();}
    void exit(){scope_man_.enter();}
    void CheckBin(chir::Bin*bin);
    std::vector<std::pair<std::map<std::string,type::Type const*>,ScopeType>> scopes;
    unique_ptr<type::TypeManager> type_man;
    // std::array<type::BuiltinType*, 10> builtin;
    chir::Module*module_=nullptr;

    virtual antlrcpp::Any visitIdentifier(CangjieParser::IdentifierContext *context);

    // virtual antlrcpp::Any visitTranslationUnit(CangjieParser::TranslationUnitContext *context);

    virtual antlrcpp::Any visitEnd(CangjieParser::EndContext *context);

    virtual antlrcpp::Any visitPreamble(CangjieParser::PreambleContext *context);

    virtual antlrcpp::Any visitPackageHeader(CangjieParser::PackageHeaderContext *context);

    virtual antlrcpp::Any visitPackageNameIdentifier(CangjieParser::PackageNameIdentifierContext *context);

    virtual antlrcpp::Any visitImportList(CangjieParser::ImportListContext *context);

    virtual antlrcpp::Any visitImportAllOrSpecified(CangjieParser::ImportAllOrSpecifiedContext *context);

    virtual antlrcpp::Any visitImportSpecified(CangjieParser::ImportSpecifiedContext *context);

    virtual antlrcpp::Any visitImportAll(CangjieParser::ImportAllContext *context);

    virtual antlrcpp::Any visitImportAlias(CangjieParser::ImportAliasContext *context);

    virtual antlrcpp::Any visitTopLevelObject(CangjieParser::TopLevelObjectContext *context);

    virtual antlrcpp::Any visitClassDefinition(CangjieParser::ClassDefinitionContext *context);

    virtual antlrcpp::Any visitSuperClassOrInterfaces(CangjieParser::SuperClassOrInterfacesContext *context);

    virtual antlrcpp::Any visitClassModifierList(CangjieParser::ClassModifierListContext *context);

    virtual antlrcpp::Any visitClassModifier(CangjieParser::ClassModifierContext *context);

    virtual antlrcpp::Any visitTypeParameters(CangjieParser::TypeParametersContext *context);

    virtual antlrcpp::Any visitSuperClass(CangjieParser::SuperClassContext *context);

    virtual antlrcpp::Any visitClassType(CangjieParser::ClassTypeContext *context);

    virtual antlrcpp::Any visitTypeArguments(CangjieParser::TypeArgumentsContext *context);

    virtual antlrcpp::Any visitSuperInterfaces(CangjieParser::SuperInterfacesContext *context);

    virtual antlrcpp::Any visitInterfaceType(CangjieParser::InterfaceTypeContext *context);

    virtual antlrcpp::Any visitGenericConstraints(CangjieParser::GenericConstraintsContext *context);

    virtual antlrcpp::Any visitUpperBounds(CangjieParser::UpperBoundsContext *context);

    virtual antlrcpp::Any visitClassBody(CangjieParser::ClassBodyContext *context);

    virtual antlrcpp::Any visitClassMemberDeclaration(CangjieParser::ClassMemberDeclarationContext *context);

    virtual antlrcpp::Any visitClassInit(CangjieParser::ClassInitContext *context);

    virtual antlrcpp::Any visitStaticInit(CangjieParser::StaticInitContext *context);

    virtual antlrcpp::Any visitClassPrimaryInit(CangjieParser::ClassPrimaryInitContext *context);

    virtual antlrcpp::Any visitClassName(CangjieParser::ClassNameContext *context);

    virtual antlrcpp::Any visitClassPrimaryInitParamLists(CangjieParser::ClassPrimaryInitParamListsContext *context);

    virtual antlrcpp::Any visitClassUnnamedInitParamList(CangjieParser::ClassUnnamedInitParamListContext *context);

    virtual antlrcpp::Any visitClassNamedInitParamList(CangjieParser::ClassNamedInitParamListContext *context);

    virtual antlrcpp::Any visitClassUnnamedInitParam(CangjieParser::ClassUnnamedInitParamContext *context);

    virtual antlrcpp::Any visitClassNamedInitParam(CangjieParser::ClassNamedInitParamContext *context);

    virtual antlrcpp::Any visitClassNonStaticMemberModifier(CangjieParser::ClassNonStaticMemberModifierContext *context);

    virtual antlrcpp::Any visitInterfaceDefinition(CangjieParser::InterfaceDefinitionContext *context);

    virtual antlrcpp::Any visitInterfaceBody(CangjieParser::InterfaceBodyContext *context);

    virtual antlrcpp::Any visitInterfaceMemberDeclaration(CangjieParser::InterfaceMemberDeclarationContext *context);

    virtual antlrcpp::Any visitInterfaceModifierList(CangjieParser::InterfaceModifierListContext *context);

    virtual antlrcpp::Any visitInterfaceModifier(CangjieParser::InterfaceModifierContext *context);

    virtual antlrcpp::Any visitFunctionDefinition(CangjieParser::FunctionDefinitionContext *context);

    virtual antlrcpp::Any visitOperatorFunctionDefinition(CangjieParser::OperatorFunctionDefinitionContext *context);

    virtual antlrcpp::Any visitFunctionParameters(CangjieParser::FunctionParametersContext *context);

    virtual antlrcpp::Any visitNondefaultParameterList(CangjieParser::NondefaultParameterListContext *context);

    virtual antlrcpp::Any visitUnnamedParameterList(CangjieParser::UnnamedParameterListContext *context);

    virtual antlrcpp::Any visitUnnamedParameter(CangjieParser::UnnamedParameterContext *context);

    virtual antlrcpp::Any visitNamedParameterList(CangjieParser::NamedParameterListContext *context);

    virtual antlrcpp::Any visitNamedParameter(CangjieParser::NamedParameterContext *context);

    virtual antlrcpp::Any visitDefaultParameter(CangjieParser::DefaultParameterContext *context);

    virtual antlrcpp::Any visitFunctionModifierList(CangjieParser::FunctionModifierListContext *context);

    virtual antlrcpp::Any visitFunctionModifier(CangjieParser::FunctionModifierContext *context);

    virtual antlrcpp::Any visitVariableDeclaration(CangjieParser::VariableDeclarationContext *context);

    virtual antlrcpp::Any visitVariableModifier(CangjieParser::VariableModifierContext *context);

    virtual antlrcpp::Any visitEnumDefinition(CangjieParser::EnumDefinitionContext *context);

    virtual antlrcpp::Any visitEnumBody(CangjieParser::EnumBodyContext *context);

    virtual antlrcpp::Any visitCaseBody(CangjieParser::CaseBodyContext *context);

    virtual antlrcpp::Any visitEnumModifier(CangjieParser::EnumModifierContext *context);

    virtual antlrcpp::Any visitStructDefinition(CangjieParser::StructDefinitionContext *context);

    virtual antlrcpp::Any visitStructBody(CangjieParser::StructBodyContext *context);

    virtual antlrcpp::Any visitStructMemberDeclaration(CangjieParser::StructMemberDeclarationContext *context);

    virtual antlrcpp::Any visitStructInit(CangjieParser::StructInitContext *context);

    virtual antlrcpp::Any visitStructPrimaryInit(CangjieParser::StructPrimaryInitContext *context);

    virtual antlrcpp::Any visitStructName(CangjieParser::StructNameContext *context);

    virtual antlrcpp::Any visitStructPrimaryInitParamLists(CangjieParser::StructPrimaryInitParamListsContext *context);

    virtual antlrcpp::Any visitStructUnnamedInitParamList(CangjieParser::StructUnnamedInitParamListContext *context);

    virtual antlrcpp::Any visitStructNamedInitParamList(CangjieParser::StructNamedInitParamListContext *context);

    virtual antlrcpp::Any visitStructUnnamedInitParam(CangjieParser::StructUnnamedInitParamContext *context);

    virtual antlrcpp::Any visitStructNamedInitParam(CangjieParser::StructNamedInitParamContext *context);

    virtual antlrcpp::Any visitStructModifier(CangjieParser::StructModifierContext *context);

    virtual antlrcpp::Any visitStructNonStaticMemberModifier(CangjieParser::StructNonStaticMemberModifierContext *context);

    virtual antlrcpp::Any visitTypeAlias(CangjieParser::TypeAliasContext *context);

    virtual antlrcpp::Any visitTypeModifier(CangjieParser::TypeModifierContext *context);

    virtual antlrcpp::Any visitExtendDefinition(CangjieParser::ExtendDefinitionContext *context);

    virtual antlrcpp::Any visitExtendType(CangjieParser::ExtendTypeContext *context);

    virtual antlrcpp::Any visitExtendBody(CangjieParser::ExtendBodyContext *context);

    virtual antlrcpp::Any visitExtendMemberDeclaration(CangjieParser::ExtendMemberDeclarationContext *context);

    virtual antlrcpp::Any visitForeignDeclaration(CangjieParser::ForeignDeclarationContext *context);

    virtual antlrcpp::Any visitForeignBody(CangjieParser::ForeignBodyContext *context);

    virtual antlrcpp::Any visitForeignMemberDeclaration(CangjieParser::ForeignMemberDeclarationContext *context);

    virtual antlrcpp::Any visitAnnotationList(CangjieParser::AnnotationListContext *context);

    virtual antlrcpp::Any visitAnnotation(CangjieParser::AnnotationContext *context);

    virtual antlrcpp::Any visitAnnotationArgumentList(CangjieParser::AnnotationArgumentListContext *context);

    virtual antlrcpp::Any visitAnnotationArgument(CangjieParser::AnnotationArgumentContext *context);

    virtual antlrcpp::Any visitMacroDefinition(CangjieParser::MacroDefinitionContext *context);

    virtual antlrcpp::Any visitMacroWithoutAttrParam(CangjieParser::MacroWithoutAttrParamContext *context);

    virtual antlrcpp::Any visitMacroWithAttrParam(CangjieParser::MacroWithAttrParamContext *context);

    virtual antlrcpp::Any visitMacroInputDecl(CangjieParser::MacroInputDeclContext *context);

    virtual antlrcpp::Any visitMacroAttrDecl(CangjieParser::MacroAttrDeclContext *context);

    virtual antlrcpp::Any visitPropertyDefinition(CangjieParser::PropertyDefinitionContext *context);

    virtual antlrcpp::Any visitPropertyBody(CangjieParser::PropertyBodyContext *context);

    virtual antlrcpp::Any visitPropertyMemberDeclaration(CangjieParser::PropertyMemberDeclarationContext *context);

    virtual antlrcpp::Any visitPropertyModifier(CangjieParser::PropertyModifierContext *context);

    virtual antlrcpp::Any visitMainDefinition(CangjieParser::MainDefinitionContext *context);

    virtual antlrcpp::Any visitType(CangjieParser::TypeContext *context);

    virtual antlrcpp::Any visitArrowType(CangjieParser::ArrowTypeContext *context);

    virtual antlrcpp::Any visitArrowParameters(CangjieParser::ArrowParametersContext *context);

    virtual antlrcpp::Any visitTupleType(CangjieParser::TupleTypeContext *context);

    virtual antlrcpp::Any visitPrefixType(CangjieParser::PrefixTypeContext *context);

    virtual antlrcpp::Any visitPrefixTypeOperator(CangjieParser::PrefixTypeOperatorContext *context);

    virtual antlrcpp::Any visitAtomicType(CangjieParser::AtomicTypeContext *context);

    virtual antlrcpp::Any visitCharLangTypes(CangjieParser::CharLangTypesContext *context);

    virtual antlrcpp::Any visitNumericTypes(CangjieParser::NumericTypesContext *context);

    virtual antlrcpp::Any visitUserType(CangjieParser::UserTypeContext *context);

    virtual antlrcpp::Any visitParenthesizedType(CangjieParser::ParenthesizedTypeContext *context);

    virtual antlrcpp::Any visitExpression(CangjieParser::ExpressionContext *context);

    virtual antlrcpp::Any visitAssignmentExpression(CangjieParser::AssignmentExpressionContext *context);

    virtual antlrcpp::Any visitTupleLeftValueExpression(CangjieParser::TupleLeftValueExpressionContext *context);

    virtual antlrcpp::Any visitLeftValueExpression(CangjieParser::LeftValueExpressionContext *context);

    virtual antlrcpp::Any visitLeftValueExpressionWithoutWildCard(CangjieParser::LeftValueExpressionWithoutWildCardContext *context);

    virtual antlrcpp::Any visitLeftAuxExpression(CangjieParser::LeftAuxExpressionContext *context);

    virtual antlrcpp::Any visitAssignableSuffix(CangjieParser::AssignableSuffixContext *context);

    virtual antlrcpp::Any visitFieldAccess(CangjieParser::FieldAccessContext *context);

    virtual antlrcpp::Any visitFlowExpression(CangjieParser::FlowExpressionContext *context);

    virtual antlrcpp::Any visitCoalescingExpression(CangjieParser::CoalescingExpressionContext *context);

    virtual antlrcpp::Any visitLogicDisjunctionExpression(CangjieParser::LogicDisjunctionExpressionContext *context);

    virtual antlrcpp::Any visitLogicConjunctionExpression(CangjieParser::LogicConjunctionExpressionContext *context);

    virtual antlrcpp::Any visitRangeExpression(CangjieParser::RangeExpressionContext *context);

    virtual antlrcpp::Any visitBitwiseDisjunctionExpression(CangjieParser::BitwiseDisjunctionExpressionContext *context);

    virtual antlrcpp::Any visitBitwiseXorExpression(CangjieParser::BitwiseXorExpressionContext *context);

    virtual antlrcpp::Any visitBitwiseConjunctionExpression(CangjieParser::BitwiseConjunctionExpressionContext *context);

    virtual antlrcpp::Any visitEqualityComparisonExpression(CangjieParser::EqualityComparisonExpressionContext *context);

    virtual antlrcpp::Any visitComparisonOrTypeExpression(CangjieParser::ComparisonOrTypeExpressionContext *context);

    virtual antlrcpp::Any visitShiftingExpression(CangjieParser::ShiftingExpressionContext *context);

    virtual antlrcpp::Any visitAdditiveExpression(CangjieParser::AdditiveExpressionContext *context);

    virtual antlrcpp::Any visitMultiplicativeExpression(CangjieParser::MultiplicativeExpressionContext *context);

    virtual antlrcpp::Any visitExponentExpression(CangjieParser::ExponentExpressionContext *context);

    virtual antlrcpp::Any visitPrefixUnaryExpression(CangjieParser::PrefixUnaryExpressionContext *context);

    virtual antlrcpp::Any visitIncAndDecExpression(CangjieParser::IncAndDecExpressionContext *context);

    virtual antlrcpp::Any visitPostfixExpression(CangjieParser::PostfixExpressionContext *context);

    virtual antlrcpp::Any visitQuestSeperatedItems(CangjieParser::QuestSeperatedItemsContext *context);

    virtual antlrcpp::Any visitQuestSeperatedItem(CangjieParser::QuestSeperatedItemContext *context);

    virtual antlrcpp::Any visitItemAfterQuest(CangjieParser::ItemAfterQuestContext *context);

    virtual antlrcpp::Any visitCallSuffix(CangjieParser::CallSuffixContext *context);

    virtual antlrcpp::Any visitValueArgument(CangjieParser::ValueArgumentContext *context);

    virtual antlrcpp::Any visitRefTransferExpression(CangjieParser::RefTransferExpressionContext *context);

    virtual antlrcpp::Any visitIndexAccess(CangjieParser::IndexAccessContext *context);

    virtual antlrcpp::Any visitRangeElement(CangjieParser::RangeElementContext *context);

    virtual antlrcpp::Any visitAtomicExpression(CangjieParser::AtomicExpressionContext *context);

    virtual antlrcpp::Any visitLiteralConstant(CangjieParser::LiteralConstantContext *context);

    virtual antlrcpp::Any visitBooleanLiteral(CangjieParser::BooleanLiteralContext *context);

    virtual antlrcpp::Any visitStringLiteral(CangjieParser::StringLiteralContext *context);

    virtual antlrcpp::Any visitLineStringContent(CangjieParser::LineStringContentContext *context);

    virtual antlrcpp::Any visitLineStringLiteral(CangjieParser::LineStringLiteralContext *context);

    virtual antlrcpp::Any visitLineStringExpression(CangjieParser::LineStringExpressionContext *context);

    virtual antlrcpp::Any visitMultiLineStringContent(CangjieParser::MultiLineStringContentContext *context);

    virtual antlrcpp::Any visitMultiLineStringLiteral(CangjieParser::MultiLineStringLiteralContext *context);

    virtual antlrcpp::Any visitMultiLineStringExpression(CangjieParser::MultiLineStringExpressionContext *context);

    virtual antlrcpp::Any visitCollectionLiteral(CangjieParser::CollectionLiteralContext *context);

    virtual antlrcpp::Any visitArrayLiteral(CangjieParser::ArrayLiteralContext *context);

    virtual antlrcpp::Any visitElements(CangjieParser::ElementsContext *context);

    virtual antlrcpp::Any visitElement(CangjieParser::ElementContext *context);

    virtual antlrcpp::Any visitExpressionElement(CangjieParser::ExpressionElementContext *context);

    virtual antlrcpp::Any visitSpreadElement(CangjieParser::SpreadElementContext *context);

    virtual antlrcpp::Any visitTupleLiteral(CangjieParser::TupleLiteralContext *context);

    virtual antlrcpp::Any visitUnitLiteral(CangjieParser::UnitLiteralContext *context);

    virtual antlrcpp::Any visitIfExpression(CangjieParser::IfExpressionContext *context);

    virtual antlrcpp::Any visitDeconstructPattern(CangjieParser::DeconstructPatternContext *context);

    virtual antlrcpp::Any visitMatchExpression(CangjieParser::MatchExpressionContext *context);

    virtual antlrcpp::Any visitMatchCase(CangjieParser::MatchCaseContext *context);

    virtual antlrcpp::Any visitPatternGuard(CangjieParser::PatternGuardContext *context);

    virtual antlrcpp::Any visitPattern(CangjieParser::PatternContext *context);

    virtual antlrcpp::Any visitConstantPattern(CangjieParser::ConstantPatternContext *context);

    virtual antlrcpp::Any visitWildcardPattern(CangjieParser::WildcardPatternContext *context);

    virtual antlrcpp::Any visitVarBindingPattern(CangjieParser::VarBindingPatternContext *context);

    virtual antlrcpp::Any visitTuplePattern(CangjieParser::TuplePatternContext *context);

    virtual antlrcpp::Any visitTypePattern(CangjieParser::TypePatternContext *context);

    virtual antlrcpp::Any visitEnumPattern(CangjieParser::EnumPatternContext *context);

    virtual antlrcpp::Any visitEnumPatternParameters(CangjieParser::EnumPatternParametersContext *context);

    virtual antlrcpp::Any visitLoopExpression(CangjieParser::LoopExpressionContext *context);

    virtual antlrcpp::Any visitForInExpression(CangjieParser::ForInExpressionContext *context);

    virtual antlrcpp::Any visitPatternsMaybeIrrefutable(CangjieParser::PatternsMaybeIrrefutableContext *context);

    virtual antlrcpp::Any visitWhileExpression(CangjieParser::WhileExpressionContext *context);

    virtual antlrcpp::Any visitDoWhileExpression(CangjieParser::DoWhileExpressionContext *context);

    virtual antlrcpp::Any visitTryExpression(CangjieParser::TryExpressionContext *context);

    virtual antlrcpp::Any visitCatchPattern(CangjieParser::CatchPatternContext *context);

    virtual antlrcpp::Any visitExceptionTypePattern(CangjieParser::ExceptionTypePatternContext *context);

    virtual antlrcpp::Any visitResourceSpecifications(CangjieParser::ResourceSpecificationsContext *context);

    virtual antlrcpp::Any visitResourceSpecification(CangjieParser::ResourceSpecificationContext *context);

    virtual antlrcpp::Any visitJumpExpression(CangjieParser::JumpExpressionContext *context);

    virtual antlrcpp::Any visitNumericTypeConvExpr(CangjieParser::NumericTypeConvExprContext *context);

    virtual antlrcpp::Any visitThisSuperExpression(CangjieParser::ThisSuperExpressionContext *context);

    virtual antlrcpp::Any visitLambdaExpression(CangjieParser::LambdaExpressionContext *context);

    virtual antlrcpp::Any visitTrailingLambdaExpression(CangjieParser::TrailingLambdaExpressionContext *context);

    virtual antlrcpp::Any visitLambdaParameters(CangjieParser::LambdaParametersContext *context);

    virtual antlrcpp::Any visitLambdaParameter(CangjieParser::LambdaParameterContext *context);

    virtual antlrcpp::Any visitSpawnExpression(CangjieParser::SpawnExpressionContext *context);

    virtual antlrcpp::Any visitSynchronizedExpression(CangjieParser::SynchronizedExpressionContext *context);

    virtual antlrcpp::Any visitParenthesizedExpression(CangjieParser::ParenthesizedExpressionContext *context);

    virtual antlrcpp::Any visitBlock(CangjieParser::BlockContext *context);

    virtual antlrcpp::Any visitUnsafeExpression(CangjieParser::UnsafeExpressionContext *context);

    virtual antlrcpp::Any visitExpressionOrDeclarations(CangjieParser::ExpressionOrDeclarationsContext *context);

    virtual antlrcpp::Any visitExpressionOrDeclarationAndEnd(CangjieParser::ExpressionOrDeclarationAndEndContext *context);

    virtual antlrcpp::Any visitExpressionOrDeclaration(CangjieParser::ExpressionOrDeclarationContext *context);

    virtual antlrcpp::Any visitVarOrfuncDeclaration(CangjieParser::VarOrfuncDeclarationContext *context);

    virtual antlrcpp::Any visitQuoteExpression(CangjieParser::QuoteExpressionContext *context);

    virtual antlrcpp::Any visitQuoteExpr(CangjieParser::QuoteExprContext *context);

    virtual antlrcpp::Any visitQuoteParameters(CangjieParser::QuoteParametersContext *context);

    virtual antlrcpp::Any visitQuoteToken(CangjieParser::QuoteTokenContext *context);

    virtual antlrcpp::Any visitQuoteInterpolate(CangjieParser::QuoteInterpolateContext *context);

    virtual antlrcpp::Any visitMacroExpression(CangjieParser::MacroExpressionContext *context);

    virtual antlrcpp::Any visitMacroAttrExpr(CangjieParser::MacroAttrExprContext *context);

    virtual antlrcpp::Any visitMacroInputExprWithoutParens(CangjieParser::MacroInputExprWithoutParensContext *context);

    virtual antlrcpp::Any visitMacroInputExprWithParens(CangjieParser::MacroInputExprWithParensContext *context);

    virtual antlrcpp::Any visitMacroTokens(CangjieParser::MacroTokensContext *context);

    virtual antlrcpp::Any visitAssignmentOperator(CangjieParser::AssignmentOperatorContext *context);

    virtual antlrcpp::Any visitEqualityOperator(CangjieParser::EqualityOperatorContext *context);

    virtual antlrcpp::Any visitComparisonOperator(CangjieParser::ComparisonOperatorContext *context);

    virtual antlrcpp::Any visitShiftingOperator(CangjieParser::ShiftingOperatorContext *context);

    virtual antlrcpp::Any visitFlowOperator(CangjieParser::FlowOperatorContext *context);

    virtual antlrcpp::Any visitAdditiveOperator(CangjieParser::AdditiveOperatorContext *context);

    virtual antlrcpp::Any visitExponentOperator(CangjieParser::ExponentOperatorContext *context);

    virtual antlrcpp::Any visitMultiplicativeOperator(CangjieParser::MultiplicativeOperatorContext *context);

    virtual antlrcpp::Any visitPrefixUnaryOperator(CangjieParser::PrefixUnaryOperatorContext *context);

    virtual antlrcpp::Any visitOverloadedOperators(CangjieParser::OverloadedOperatorsContext *context);
   };
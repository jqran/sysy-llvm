grammar Cangjie;
import Lex;

identifier
    : Identifier
    | PUBLIC
    | PRIVATE
    | PROTECTED
    | OVERRIDE
    | ABSTRACT
    | OPEN
    | REDEF
    | GET
    | SET
    ;


TRIPLE_QUOTE_CLOSE
    : MultiLineStringQuote? '"""' ;


translationUnit
    : NL* preamble end* topLevelObject* NL* (topLevelObject (end+ topLevelObject?)*)?  (end+ mainDefinition)? end*  EOF
    ;

end
    : NL | SEMI
    ;


preamble
    : packageHeader? importList*
    ;

packageHeader
    : PACKAGE (NL)* packageNameIdentifier end+
    ;

packageNameIdentifier
    : identifier (NL* DOT NL* identifier)*
    ;

importList
    : (FROM NL* identifier)? NL* IMPORT NL* importAllOrSpecified
    (NL* COMMA NL* importAllOrSpecified)* end+
    ;

importAllOrSpecified
    : importAll
    | importSpecified (NL* importAlias)?
    ;

importSpecified
    : (identifier NL* DOT NL*)+ identifier
    ;

importAll
    : (identifier NL* DOT NL*)+ MUL
    ;

importAlias
    : AS NL* identifier
    ;

topLevelObject
    : classDefinition
    | interfaceDefinition
    | functionDefinition
    | variableDeclaration
    | enumDefinition
    | structDefinition
    | typeAlias
    | extendDefinition
    | foreignDeclaration
    | macroDefinition
    | macroExpression
    ;


classDefinition
    : (classModifierList NL*)? CLASS NL* identifier
    (NL* typeParameters NL*)?
    (NL* UPPERBOUND NL* superClassOrInterfaces)?
    (NL* genericConstraints)?
    NL* classBody
    ;

superClassOrInterfaces
    : superClass (NL* BITAND NL* superInterfaces)?
    | superInterfaces
    ;

classModifierList
    : classModifier+
    ;

classModifier
    : PUBLIC
    | PROTECTED
    | INTERNAL
    | PRIVATE
    | ABSTRACT
    | OPEN
    ;

typeParameters
    : LT NL* identifier (NL* COMMA NL* identifier)* NL* GT
    ;

superClass
    : classType
    ;

classType

    : (identifier NL* DOT  NL*)*  identifier (NL* typeParameters)?
    ;

typeArguments
    : LT NL* type (NL* COMMA NL* type)* NL* GT
    ;

superInterfaces
    : interfaceType (NL* COMMA NL* interfaceType )*
    ;

interfaceType
    : classType
    ;

genericConstraints
    : WHERE NL* (identifier | THISTYPE) NL* UPPERBOUND NL* upperBounds (NL* COMMA NL* (identifier | THISTYPE) NL* UPPERBOUND NL* upperBounds)*
    ;

upperBounds
    : type (NL* BITAND NL* type)*
    ;

classBody
    : LCURL end*
         classMemberDeclaration* NL*
         classPrimaryInit? NL* 
         classMemberDeclaration* end* RCURL
    ;

classMemberDeclaration
    : (classInit
    | staticInit
    | variableDeclaration
    | functionDefinition
    | operatorFunctionDefinition
    | macroExpression
    | propertyDefinition
    ) end*
    ;

classInit
    : (classNonStaticMemberModifier | CONST NL*)? INIT NL* functionParameters NL* block
    ;

staticInit
    : STATIC INIT LPAREN RPAREN
    LCURL
    expressionOrDeclarations
    RCURL
    ;

classPrimaryInit
    : (classNonStaticMemberModifier | CONST NL*)?  className NL* LPAREN NL*  
          classPrimaryInitParamLists 
       NL* RPAREN NL*
 	  LCURL NL*
 	       (SUPER callSuffix)? end
 	       expressionOrDeclarations
 	  NL* RCURL
    ;

className
    : identifier
    ;

classPrimaryInitParamLists
    : unnamedParameterList (NL* COMMA NL* namedParameterList)? (NL* COMMA NL* classNamedInitParamList)?
    | unnamedParameterList (NL* COMMA NL* classUnnamedInitParamList)? (NL* COMMA NL* classNamedInitParamList)?
    | classUnnamedInitParamList (NL* COMMA NL* classNamedInitParamList)?
    | namedParameterList (NL* COMMA NL* classNamedInitParamList)?
    | classNamedInitParamList
    ;

classUnnamedInitParamList
    : classUnnamedInitParam (NL* COMMA NL* classUnnamedInitParam)*
    ;

classNamedInitParamList
    : classNamedInitParam (NL* COMMA NL* classNamedInitParam)*
    ;

classUnnamedInitParam
    : (classNonStaticMemberModifier NL*)? (LET | VAR) NL* identifier NL* COLON NL* type
    ;

classNamedInitParam
    : (classNonStaticMemberModifier NL*)? (LET | VAR) NL* identifier NL* NOT NL* COLON NL* type (NL* ASSIGN NL* expression)?
    ;

classNonStaticMemberModifier
    : PUBLIC
    | PRIVATE
    | PROTECTED
    | INTERNAL
    ;



interfaceDefinition
    : (interfaceModifierList NL*)? INTERFACE NL* identifier
    (NL* typeParameters NL*)?
    (NL* UPPERBOUND NL* superInterfaces)?
    (NL* genericConstraints)?
    (NL* interfaceBody)
    ;

interfaceBody
    : LCURL end* interfaceMemberDeclaration* end* RCURL
    ;

interfaceMemberDeclaration
    : (functionDefinition
    | operatorFunctionDefinition
    | macroExpression
    | propertyDefinition) end*
    ;

interfaceModifierList
    : (interfaceModifier NL*)+
    ;

interfaceModifier
    : PUBLIC
    | PROTECTED
    | INTERNAL
    | PRIVATE
    | OPEN
    ;


functionDefinition
    :(functionModifierList NL*)? FUNC
    NL* identifier
    (NL* typeParameters NL*)?
    NL* functionParameters
    (NL* COLON NL* type)?
    (NL* genericConstraints)?
    (NL* block)?
    ;

operatorFunctionDefinition
    : (functionModifierList NL*)? OPERATOR NL* FUNC
    NL* overloadedOperators
    (NL* typeParameters NL*)?
    NL* functionParameters
    (NL* COLON NL* type)?
    (NL* genericConstraints)?
    (NL* block)?
    ;

functionParameters
    : (LPAREN (NL* unnamedParameterList ( NL* COMMA NL* namedParameterList)? )? 
        NL* RPAREN NL*)
    | (LPAREN NL* (namedParameterList NL*)? RPAREN NL*)
    ;

nondefaultParameterList
    : unnamedParameter (NL* COMMA NL* unnamedParameter)* 
        (NL* COMMA NL*  namedParameter)*
    | namedParameter (NL* COMMA NL* namedParameter)*
    ;

unnamedParameterList
    : unnamedParameter (NL* COMMA NL*  unnamedParameter)*
    ;

unnamedParameter
    : (identifier | WILDCARD) NL* COLON NL* type
    ;

namedParameterList
    : (namedParameter | defaultParameter)
      (NL* COMMA NL* (namedParameter | defaultParameter))*
    ;
    
namedParameter
    : identifier NL* NOT NL* COLON NL* type
    ;

defaultParameter
    : identifier NL* NOT NL* COLON NL* type NL* ASSIGN NL* expression
    ;

functionModifierList
    : (functionModifier NL*)+
    ;

functionModifier
    : PUBLIC
    | PRIVATE
    | PROTECTED
    | INTERNAL
    | STATIC
    | OPEN
    | OVERRIDE
    | OPERATOR
    | REDEF
    | MUT
    | UNSAFE
    | CONST
    ;

variableDeclaration
    : variableModifier* NL* (LET | VAR | CONST) NL* patternsMaybeIrrefutable ( (NL* COLON NL* type)? (NL* ASSIGN NL* expression)
                                                                     | (NL* COLON NL* type)
                                                                     )
    ;

variableModifier
    : PUBLIC
    | PRIVATE
    | PROTECTED
    | INTERNAL
    | STATIC
    ;

enumDefinition
    : (enumModifier NL*)? ENUM NL* identifier (NL* typeParameters NL*)? 
    (NL* UPPERBOUND NL* superInterfaces)? 
    (NL* genericConstraints)? NL* LCURL end* enumBody end* RCURL
    ;

enumBody
    : (BITOR NL*)? caseBody (NL* BITOR NL* caseBody)*     
    (NL* 
    ( functionDefinition 
    | operatorFunctionDefinition 
    | propertyDefinition
    | macroExpression
    ))*
    ;

caseBody
    : identifier ( NL* LPAREN NL* type (NL* COMMA NL* type)* NL* RPAREN)?
    ;

enumModifier
    : PUBLIC
    | PROTECTED
    | INTERNAL
    | PRIVATE
    ;

structDefinition
    : (structModifier NL*)? STRUCT NL* identifier (NL* typeParameters NL*)? 
    (NL* UPPERBOUND NL* superInterfaces)?
    (NL* genericConstraints)? NL* structBody
    ;

structBody
    : LCURL end*
        structMemberDeclaration* NL*
        structPrimaryInit? NL*
        structMemberDeclaration*
      end* RCURL
    ; 

structMemberDeclaration
    : (structInit
    | staticInit
    | variableDeclaration
    | functionDefinition
    | operatorFunctionDefinition
    | macroExpression
    | propertyDefinition
    ) end*
    ;

structInit
    : (structNonStaticMemberModifier | CONST NL*)? INIT NL* functionParameters NL* block
    ;

// staticInit
//     : STATIC INIT LPAREN RPAREN
//     LCURL
//     expressionOrDeclarations?
//     RCURL
//     ;

structPrimaryInit
    : (structNonStaticMemberModifier | CONST NL*)? structName NL* LPAREN NL* structPrimaryInitParamLists? NL* RPAREN NL*
 	  LCURL NL*
 	       expressionOrDeclarations
 	  NL* RCURL
    ;

structName
    : identifier
    ;

structPrimaryInitParamLists
    : unnamedParameterList (NL* COMMA NL* namedParameterList)? (NL* COMMA NL* structNamedInitParamList)?
    | unnamedParameterList (NL* COMMA NL* structUnnamedInitParamList)? (NL* COMMA NL* structNamedInitParamList)?
    | structUnnamedInitParamList (NL* COMMA NL* structNamedInitParamList)?
    | namedParameterList (NL* COMMA NL* structNamedInitParamList)?
    | structNamedInitParamList
    ;

structUnnamedInitParamList
    : structUnnamedInitParam (NL* COMMA NL* structUnnamedInitParam)*
    ;

structNamedInitParamList
    : structNamedInitParam (NL* COMMA NL*  structNamedInitParam)*
    ;

structUnnamedInitParam
    : (structNonStaticMemberModifier NL*)? (LET | VAR) NL* identifier NL* COLON NL* type
    ;

structNamedInitParam
    : (structNonStaticMemberModifier NL*)? (LET | VAR) NL* identifier NL* NOT NL* COLON NL* type (NL* ASSIGN NL* expression)?
    ;

structModifier
    : PUBLIC
    | PROTECTED
    | INTERNAL
    | PRIVATE
    ;

structNonStaticMemberModifier
    : PUBLIC
    | PROTECTED
    | INTERNAL
    | PRIVATE
    ;

typeAlias
    : (typeModifier NL*)? TYPE_ALIAS NL* identifier (NL* typeParameters)? NL* ASSIGN NL* type end*
    ;

typeModifier
    : PUBLIC
    | PROTECTED
    | INTERNAL
    | PRIVATE
    ;

extendDefinition
    : EXTEND NL* extendType
    (NL* UPPERBOUND NL* superInterfaces)? (NL* genericConstraints)?
    NL* extendBody
    ;

extendType
    : (typeParameters)? (identifier NL* DOT  NL*)* identifier (NL* typeArguments)?
    | INT8
    | INT16
    | INT32
    | INT64
    | INTNATIVE
    | UINT8
    | UINT16
    | UINT32
    | UINT64
    | UINTNATIVE
    | FLOAT16
    | FLOAT32
    | FLOAT64
    | RUNE
    | BOOLEAN
    | NOTHING
    | UNIT
    ;

extendBody
    : LCURL end* extendMemberDeclaration* end* RCURL
    ;

extendMemberDeclaration
    : (functionDefinition
    | operatorFunctionDefinition
    | macroExpression
    | propertyDefinition
    ) end*
    ;

foreignDeclaration
    : FOREIGN NL* (foreignBody | foreignMemberDeclaration)
    ;

foreignBody
    : LCURL end* foreignMemberDeclaration* end* RCURL
    ;

foreignMemberDeclaration
    : (classDefinition
    | interfaceDefinition
    | functionDefinition
    | macroExpression
    | variableDeclaration) end*
    ;

annotationList: annotation+;

annotation
    : AT (identifier NL* DOT)* identifier (LSQUARE NL* annotationArgumentList NL* RSQUARE)?
    ;

annotationArgumentList
    : annotationArgument (NL* COMMA NL* annotationArgument)* NL* COMMA?
    ;

annotationArgument
    : identifier NL* COLON NL* expression
    | expression
    ;

macroDefinition
    : PUBLIC NL* MACRO NL* identifier NL*
    (macroWithoutAttrParam | macroWithAttrParam) NL*
    (COLON NL* identifier NL*)?
    (ASSIGN NL* expression | block)
    ;

macroWithoutAttrParam
    : LPAREN NL* macroInputDecl NL* RPAREN
    ;

macroWithAttrParam
    : LPAREN NL* macroAttrDecl NL* COMMA NL* macroInputDecl NL* RPAREN
    ;

macroInputDecl
    : identifier NL* COLON NL* identifier
    ;

macroAttrDecl
    : identifier NL* COLON NL* identifier
    ;

propertyDefinition
    : propertyModifier* NL* PROP NL* identifier NL* COLON NL* type NL* propertyBody?
    ;

propertyBody
    : LCURL end* propertyMemberDeclaration+ end* RCURL
    ;

propertyMemberDeclaration
    : GET NL* LPAREN RPAREN NL* block end*
    | SET NL* LPAREN identifier RPAREN NL* block end*
    ;

propertyModifier
    : PUBLIC
    | PRIVATE
    | PROTECTED
    | INTERNAL
    | STATIC
    | OPEN
    | OVERRIDE
    | REDEF
    | MUT
    ;

mainDefinition
    : MAIN
      NL* functionParameters
      (NL* COLON NL* type)?
      NL* block
    ;

type
    : arrowType
    | tupleType
    | prefixType
    | atomicType
    ;

arrowType
    : arrowParameters NL* ARROW NL* type
    ;

arrowParameters
    : LPAREN NL* (type (NL* COMMA NL* type)* NL*)? RPAREN
    ;

tupleType
    : LPAREN NL* type (NL* COMMA NL* type)+ NL* RPAREN
    ;

prefixType
    : prefixTypeOperator type
    ;

prefixTypeOperator
    : QUEST
    ;

atomicType
    : charLangTypes
    | userType
    | parenthesizedType
    ;

charLangTypes
    : numericTypes
    | RUNE
    | BOOLEAN
    | NOTHING
    | UNIT
    | THISTYPE
    ;

numericTypes
    : INT8
    | INT16
    | INT32
    | INT64
    | INTNATIVE
    | UINT8
    | UINT16
    | UINT32
    | UINT64
    | UINTNATIVE
    | FLOAT16
    | FLOAT32
    | FLOAT64
    ;

userType
    : (identifier NL* DOT NL*)* identifier ( NL* typeArguments)?
    ;

parenthesizedType
    : LPAREN NL* type NL* RPAREN
    ;


expression
    : assignmentExpression
    ;

assignmentExpression
    : leftValueExpressionWithoutWildCard NL* assignmentOperator NL* flowExpression
    | leftValueExpression NL* ASSIGN NL* flowExpression
    | tupleLeftValueExpression NL* ASSIGN NL* flowExpression
    | flowExpression
    ;

tupleLeftValueExpression
    : LPAREN NL* (leftValueExpression | tupleLeftValueExpression) (NL* COMMA NL* (leftValueExpression | tupleLeftValueExpression))+ NL* COMMA? NL* RPAREN
    ;

leftValueExpression
    : leftValueExpressionWithoutWildCard
    | WILDCARD
    ;

leftValueExpressionWithoutWildCard
    : identifier
    | leftAuxExpression QUEST? NL* assignableSuffix
    ;

leftAuxExpression
    : identifier (NL* typeArguments)?
    | type
    | thisSuperExpression
    | leftAuxExpression QUEST? NL* DOT NL* identifier (NL* typeArguments)?
    | leftAuxExpression QUEST? callSuffix
    | leftAuxExpression QUEST? indexAccess
    ;

assignableSuffix
    : fieldAccess
    | indexAccess
    ;

fieldAccess
    : NL* DOT NL* identifier
    ;

flowExpression
    : coalescingExpression (NL* flowOperator NL* coalescingExpression)*
    ;

coalescingExpression
    : logicDisjunctionExpression (NL* QUEST QUEST NL* logicDisjunctionExpression)*
    ;

logicDisjunctionExpression
    : logicConjunctionExpression (NL* OR NL* logicConjunctionExpression)*
    ;

logicConjunctionExpression
    : rangeExpression (NL* AND NL* rangeExpression)*
    ;

rangeExpression
    : bitwiseDisjunctionExpression NL* (CLOSEDRANGEOP | RANGEOP) NL* bitwiseDisjunctionExpression (NL* COLON NL* bitwiseDisjunctionExpression)?
    | bitwiseDisjunctionExpression
    ;

bitwiseDisjunctionExpression
    : bitwiseXorExpression (NL* BITOR NL* bitwiseXorExpression)*
    ;

bitwiseXorExpression
    : bitwiseConjunctionExpression (NL* BITXOR NL* bitwiseConjunctionExpression)*
    ;

bitwiseConjunctionExpression
    : equalityComparisonExpression (NL* BITAND NL* equalityComparisonExpression)*
    ;

equalityComparisonExpression
    : comparisonOrTypeExpression (NL* equalityOperator NL* comparisonOrTypeExpression)?
    ;

comparisonOrTypeExpression
    : shiftingExpression (NL* comparisonOperator NL* shiftingExpression)?
    | shiftingExpression (NL* IS NL* type)?
    | shiftingExpression (NL* AS NL* type)?
    ;

shiftingExpression
    : additiveExpression (NL* shiftingOperator NL* additiveExpression)*
    ;

additiveExpression
    : multiplicativeExpression (NL* additiveOperator NL* multiplicativeExpression)*
    ;

multiplicativeExpression
    : exponentExpression (NL* multiplicativeOperator NL* exponentExpression)*
    ;

exponentExpression
    : prefixUnaryExpression (NL* exponentOperator NL* prefixUnaryExpression)*
    ;

prefixUnaryExpression
    : prefixUnaryOperator* incAndDecExpression
    ;

incAndDecExpression
    : postfixExpression (INC | DEC )?
    ;

postfixExpression
     : atomicExpression
     | type NL* DOT NL* identifier
     | postfixExpression NL* DOT NL* identifier (NL* typeArguments)?
     | postfixExpression callSuffix
     | postfixExpression indexAccess
     | postfixExpression NL* DOT NL* identifier callSuffix? trailingLambdaExpression
     | identifier callSuffix? trailingLambdaExpression
     | postfixExpression (QUEST questSeperatedItems)+
     ;

questSeperatedItems
     : questSeperatedItem+
     ;

questSeperatedItem
     : itemAfterQuest (callSuffix | callSuffix? trailingLambdaExpression | indexAccess)?
     ;

itemAfterQuest
     : DOT identifier (NL* typeArguments)?
     | callSuffix
     | indexAccess
     | trailingLambdaExpression
     ;

callSuffix
    : LPAREN NL* (valueArgument (NL* COMMA NL* valueArgument)* NL*)? RPAREN
    ;

valueArgument
    : identifier NL* COLON NL* expression
    | expression
    | refTransferExpression
    ;

refTransferExpression
    : INOUT (expression DOT)? identifier
    ;

indexAccess
    : LSQUARE NL* (expression | rangeElement) NL* RSQUARE
    ;

rangeElement
    :  RANGEOP
    | ( CLOSEDRANGEOP | RANGEOP ) NL* expression
    | expression NL* RANGEOP
    ;

atomicExpression
    : literalConstant
    | collectionLiteral
    | tupleLiteral
    | identifier (NL* typeArguments)?
    | unitLiteral
    | ifExpression
    | matchExpression
    | loopExpression
    | tryExpression
    | jumpExpression
    | numericTypeConvExpr
    | thisSuperExpression
    | spawnExpression
    | synchronizedExpression
    | parenthesizedExpression
    | lambdaExpression
    | quoteExpression
    | macroExpression
    | unsafeExpression
    ;

literalConstant
    : IntegerLiteral
    | FloatLiteral
    | RuneLiteral
    | ByteLiteral
    | booleanLiteral
    | stringLiteral
    | ByteStringArrayLiteral
    | unitLiteral
    ;

booleanLiteral
    : TRUE
    | FALSE
    ;

stringLiteral
    : lineStringLiteral
    | multiLineStringLiteral
    | MultiLineRawStringLiteral
    ;

lineStringContent
    :  LineStrText
    ;

lineStringLiteral
    : QUOTE_OPEN_CLOSE (lineStringExpression | lineStringContent)* QUOTE_OPEN_CLOSE
    ;

lineStringExpression
    : LineStrExprStart SEMI* (expressionOrDeclaration (SEMI+ expressionOrDeclaration?)*) SEMI* RCURL
    ;

multiLineStringContent
    : MultiLineStrText
    ;

multiLineStringLiteral
    : TRIPLE_QUOTE_OPEN (multiLineStringExpression | multiLineStringContent)* TRIPLE_QUOTE_CLOSE
    ;

multiLineStringExpression
    : LineStrExprStart end* (expressionOrDeclaration (end+ expressionOrDeclaration?)*) end* RCURL
    ;

collectionLiteral
    : arrayLiteral
    ;

arrayLiteral 
    : LSQUARE (NL* elements)? NL* RSQUARE 
    ;

elements    
    : element ( NL* COMMA NL* element )* 
    ;

element
    : expressionElement
    | spreadElement
    ;

expressionElement 
    : expression
    ;

spreadElement
    : MUL expression
    ;

tupleLiteral
    : LPAREN NL* expression (NL* COMMA NL*expression)+ NL* RPAREN
    ;

unitLiteral
    : LPAREN NL* RPAREN
    ;

ifExpression
    : IF NL* LPAREN NL* (LET NL* deconstructPattern NL* BACKARROW NL*)? expression NL* RPAREN NL* block
    (NL* ELSE (NL* ifExpression | NL* block))?
    ;

deconstructPattern
    : constantPattern
    | wildcardPattern
    | varBindingPattern
    | tuplePattern
    | enumPattern
    ;

matchExpression
    : MATCH NL* LPAREN NL* expression NL* RPAREN NL* LCURL NL* matchCase+ NL* RCURL
    | MATCH NL* LCURL NL* (CASE NL* (expression | WILDCARD) NL* DOUBLE_ARROW NL* expressionOrDeclaration (end+ expressionOrDeclaration?)*)+ NL* RCURL
    ;

matchCase
    : CASE NL* pattern NL* patternGuard? NL* DOUBLE_ARROW NL* expressionOrDeclaration (end+ expressionOrDeclaration?)*
    ;

patternGuard
    : WHERE NL* expression
    ;

pattern
   : constantPattern
   | wildcardPattern
   | varBindingPattern
   | tuplePattern
   | typePattern
   | enumPattern
   ;

constantPattern
   : literalConstant NL* ( NL* BITOR NL* literalConstant)*
   ;

wildcardPattern
   : WILDCARD
   ;

varBindingPattern
   : identifier
   ;

tuplePattern
   : LPAREN NL* pattern (NL* COMMA NL* pattern)+ NL* RPAREN
   ;

typePattern
   : (WILDCARD | identifier) NL* COLON NL* type
   ;

enumPattern
   : NL* ((userType NL* DOT NL*)? identifier enumPatternParameters?) (NL* BITOR NL* ((userType NL* DOT NL*)? identifier enumPatternParameters?))*
   ;

enumPatternParameters
   : LPAREN NL* pattern (NL* COMMA NL* pattern)* NL* RPAREN
   ;

loopExpression
    : forInExpression
    | whileExpression
    | doWhileExpression
    ;

forInExpression
    : FOR NL* LPAREN NL* patternsMaybeIrrefutable NL* IN NL* expression NL* patternGuard? NL* RPAREN NL* block
    ;

patternsMaybeIrrefutable
    : wildcardPattern
    | varBindingPattern
    | tuplePattern
    | enumPattern
    ;

whileExpression
    : WHILE NL* LPAREN NL* (LET NL* deconstructPattern NL* BACKARROW NL*)? expression NL* RPAREN NL* block
    ;

doWhileExpression
    : DO NL* block NL* WHILE NL* LPAREN NL* expression NL* RPAREN
    ;

tryExpression
    : TRY NL* block NL* FINALLY NL* block
    | TRY NL* block (NL* CATCH NL* LPAREN NL* catchPattern NL* RPAREN NL* block)+ (NL* FINALLY NL* block)?
    | TRY NL* LPAREN NL* resourceSpecifications NL* RPAREN NL* block
    (NL* CATCH NL* LPAREN NL* catchPattern NL* RPAREN NL* block)* (NL* FINALLY NL* block)?
    ;

catchPattern
    : wildcardPattern
    | exceptionTypePattern
    ;

exceptionTypePattern
    : (WILDCARD | identifier) NL* COLON NL* type (NL* BITOR NL* type)*
    ;

resourceSpecifications
    : resourceSpecification (NL* COMMA NL* resourceSpecification)*
    ;

resourceSpecification
    : identifier (NL* COLON NL* classType)? NL* ASSIGN NL* expression
    ;

jumpExpression
    : THROW NL* expression
    | RETURN (NL* expression)?
    | CONTINUE
    | BREAK
    ;

numericTypeConvExpr
    : numericTypes LPAREN NL* expression NL* RPAREN
    ;

thisSuperExpression
    : THIS
    | SUPER
    ;

lambdaExpression
    : LCURL NL* lambdaParameters? NL* DOUBLE_ARROW NL* expressionOrDeclarations RCURL
    ;

trailingLambdaExpression
    : LCURL NL* (lambdaParameters? NL* DOUBLE_ARROW NL*)? expressionOrDeclarations RCURL
    ;

lambdaParameters
    : lambdaParameter (NL* COMMA NL* lambdaParameter)*
    ;

lambdaParameter
    : (identifier | WILDCARD) (NL* COLON NL* type)?
    ;

spawnExpression
    : SPAWN (LPAREN NL* expression NL* RPAREN)? NL* trailingLambdaExpression
    ;

synchronizedExpression
    : SYNCHRONIZED LPAREN NL* expression NL* RPAREN NL* block
    ;

parenthesizedExpression
    : LPAREN NL* expression NL* RPAREN
    ;

block
    : LCURL expressionOrDeclarations RCURL
    ;

unsafeExpression
    : UNSAFE NL* block
    ;

expressionOrDeclarations
    : end* (expressionOrDeclarationAndEnd end*)* expressionOrDeclaration?
    ;

expressionOrDeclarationAndEnd
    : ( expression | varOrfuncDeclaration ) end
    ;

expressionOrDeclaration
    : expression
    | varOrfuncDeclaration
    ;

varOrfuncDeclaration
    : functionDefinition
    | variableDeclaration
    ;

quoteExpression
    : QUOTE quoteExpr
    ;

quoteExpr
    : LPAREN NL* quoteParameters NL* RPAREN
    ;

quoteParameters
    : (NL* quoteToken | NL* quoteInterpolate | NL* macroExpression)+
    ;

quoteToken
    : DOT | COMMA | LPAREN | RPAREN | LSQUARE | RSQUARE | LCURL | RCURL | EXP | MUL | MOD | DIV | ADD | SUB
    | PIPELINE | COMPOSITION
    | INC | DEC | AND | OR | NOT | BITAND | BITOR | BITXOR | LSHIFT | RSHIFT | COLON | SEMI
    | ASSIGN | ADD_ASSIGN | SUB_ASSIGN | MUL_ASSIGN | EXP_ASSIGN | DIV_ASSIGN | MOD_ASSIGN
    | AND_ASSIGN | OR_ASSIGN | BITAND_ASSIGN | BITOR_ASSIGN | BITXOR_ASSIGN | LSHIFT_ASSIGN | RSHIFT_ASSIGN
    | ARROW | BACKARROW | DOUBLE_ARROW | ELLIPSIS | CLOSEDRANGEOP | RANGEOP | HASH | AT | QUEST | UPPERBOUND | LT | GT | LE | GE
    | NOTEQUAL | EQUAL | WILDCARD | BACKSLASH | QUOTESYMBOL | DOLLAR
    | INT8 | INT16 | INT32 | INT64 | INTNATIVE | UINT8 | UINT16 | UINT32 | UINT64 | UINTNATIVE | FLOAT16
    | FLOAT32 | FLOAT64 | RUNE | BOOLEAN | UNIT | NOTHING | STRUCT | ENUM | THIS
    | PACKAGE | IMPORT | CLASS | INTERFACE | FUNC | LET | VAR | CONST | TYPE_ALIAS
    | INIT | THIS | SUPER | IF | ELSE | CASE | TRY | CATCH | FINALLY
    | FOR | DO | WHILE | THROW | RETURN | CONTINUE | BREAK | AS | IN
    | MATCH | FROM | WHERE | EXTEND | SPAWN | SYNCHRONIZED | MACRO | QUOTE | TRUE | FALSE
    | STATIC | PUBLIC | PRIVATE | PROTECTED
    | OVERRIDE | ABSTRACT | OPEN | OPERATOR | FOREIGN
    | Identifier | DollarIdentifier
    | literalConstant
    ;

quoteInterpolate
    : DOLLAR LPAREN NL* expression NL* RPAREN
    ;

macroExpression
    : AT Identifier macroAttrExpr? NL* (macroInputExprWithoutParens | macroInputExprWithParens)
    ;

macroAttrExpr
    : LSQUARE NL* quoteToken* NL* RSQUARE
    ;

macroInputExprWithoutParens
    : functionDefinition
    | operatorFunctionDefinition
    | staticInit
    | structDefinition
    | structPrimaryInit
    | structInit
    | enumDefinition
    | caseBody
    | classDefinition
    | classPrimaryInit
    | classInit
    | interfaceDefinition
    | variableDeclaration
    | propertyDefinition
    | extendDefinition
    | macroExpression
    ;

macroInputExprWithParens
    : LPAREN NL* macroTokens NL* RPAREN
    ;

macroTokens
    : (quoteToken | macroExpression)*
    ;

assignmentOperator
    : ASSIGN
    | ADD_ASSIGN
    | SUB_ASSIGN
    | EXP_ASSIGN
    | MUL_ASSIGN
    | DIV_ASSIGN
    | MOD_ASSIGN
    | AND_ASSIGN
    | OR_ASSIGN
    | BITAND_ASSIGN
    | BITOR_ASSIGN
    | BITXOR_ASSIGN
    | LSHIFT_ASSIGN
    | RSHIFT_ASSIGN
    ;

equalityOperator
    : NOTEQUAL
    | EQUAL
    ;

comparisonOperator
    : LT
    | GT
    | LE
    | GE
    ;

shiftingOperator
    : LSHIFT | RSHIFT
    ;

flowOperator
    : PIPELINE | COMPOSITION
    ;

additiveOperator
    : ADD | SUB
    ;

exponentOperator
    : EXP
    ;

multiplicativeOperator
    : MUL
    | DIV
    | MOD
    ;

prefixUnaryOperator
    : SUB
    | NOT
    ;

overloadedOperators
    : LSQUARE RSQUARE
    | NOT
    | ADD
    | SUB
    | EXP
    | MUL
    | DIV
    | MOD
    | LSHIFT
    | RSHIFT
    | LT
    | GT
    | LE
    | GE
    | EQUAL
    | NOTEQUAL
    | BITAND
    | BITXOR
    | BITOR
    ;

#include "frontend/lex.hpp"
#include "frontend/parser.hpp"
#include "frontend/node.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <memory>
#include <fstream>
#include <stack>
#include <utility>
#include <variant>
#include <vector>
#include "frontend/type.hpp"
#include "cassert"
// Parser::Parser(string s):lex(std::make_unique<Lexer>(s)),comp(make_unique<ast::CompunitNode>()),cur_pos(0,0){
//     curTok=lex->nextToken();
//     peekTok=lex->nextToken();
// }
static std::stack<ScopeType> btys;
const std::map<tokenType, parserOpPrec>Parser::precedences= {
    {tokenType::ASSIGN,parserOpPrec::OP_ASSIGN},
    {tokenType::D_OR,parserOpPrec::OP_DOR},
    {tokenType::D_ESPERLUTTE,parserOpPrec::OP_DESP},
    {tokenType::OR,parserOpPrec::OP_OR},
    {tokenType::ESPERLUTTE,parserOpPrec::OP_ESP},
    {tokenType::EQUAL,parserOpPrec::OP_EQUALS},
    {tokenType::NOTEQUAL,parserOpPrec::OP_EQUALS},
    {tokenType::LT,parserOpPrec::OP_LESSGREATER},
    {tokenType::GT,parserOpPrec::OP_LESSGREATER},
    {tokenType::LE,parserOpPrec::OP_LESSGREATER},
    {tokenType::GE,parserOpPrec::OP_LESSGREATER},
    {tokenType::PLUS,parserOpPrec::OP_SUMS},
    {tokenType::MINUS,parserOpPrec::OP_SUMS},
    {tokenType::SLASH,parserOpPrec::OP_PRODUCTS},
    {tokenType::ASTERISK,parserOpPrec::OP_PRODUCTS},
    {tokenType::MOD,parserOpPrec::OP_PRODUCTS},
};
Parser::Parser(std::string filename):file_name(filename),comp(make_unique<ast::CompunitNode>()),last_pos_(0,0){
  std::ifstream sysy_file;
    sysy_file.open(filename,std::ios::in);
    if (!sysy_file.is_open()){
        std::cerr << "read fail." << endl;
    }
	string content( (std::istreambuf_iterator<char>(sysy_file) ),
					 (std::istreambuf_iterator<char>() ) );

	sysy_file.close();
    lex= make_unique<Lexer>(content);
    
    curTok=lex->nextToken();
    peekTok=lex->nextToken();
}
[[deprecated]]
void Parser::reParser(string filename){
    this->file_name=filename;
    this->comp.reset();
    this->comp=make_unique<ast::CompunitNode>();
    last_pos_={0,0};
    std::ifstream sysy_file;
    sysy_file.open(filename,std::ios::in);
    if (!sysy_file.is_open()){
        std::cerr << "read fail." << endl;
    }
	string content( (std::istreambuf_iterator<char>(sysy_file) ),
					 (std::istreambuf_iterator<char>() ) );

	sysy_file.close();
    lex.reset();
    lex= make_unique<Lexer>(content);
    
    curTok=lex->nextToken();
    peekTok=lex->nextToken();
}
::ast::BinOp strToBinop(string &str){
    static std::map<std::string,::ast::BinOp> str_to_binop={
        {"+",ast::BinOp::PlUS},
        {"-",ast::BinOp::MINUS},
        {"*",ast::BinOp::MULTI},
        {"/",ast::BinOp::SLASH},
        {"%",ast::BinOp::MOD},
	{"=",ast::BinOp::ASSIGN},
	{"==",ast::BinOp::EQ},
        {"!=",ast::BinOp::NOT_EQ},
        {"||",ast::BinOp::DOR},
        {"&&",ast::BinOp::DAND},
        {"<",ast::BinOp::LT},
        {"<=",ast::BinOp::LE},
        {">",ast::BinOp::GT},
        {">=",ast::BinOp::GE},
    };
    auto i=str_to_binop.find(str);
    if(i!=str_to_binop.end())
        return i->second;
    return ast::BinOp::ILLEGAL;
}

type::Type* Parser::parserDefType(){
    assert(0);
    // type::ValType val_type{};//=curTok->type;
    // bool hasdef=false;
    // while(!hasdef){
    //     if(curTokIs(tokenType::CONST)){
    //         if(val_type.t&TYPE_CONST){
    //             exit(114);
    //         }
    //         // val_type.t.is_const=true;
    //         val_type.t=val_type.t|TYPE_CONST;
    //     }else if(curTokIs(tokenType::DEFINT)){
    //         val_type.t=val_type.t|TYPE_INT;
    //         val_type.size=INT_SIZE;
    //         hasdef=true;
    //     }else if(curTokIs(tokenType::DEFFLOAT)){
    //         val_type.t=val_type.t|TYPE_FLOAT;
    //         val_type.size=FLOAT_SIZE;
    //         hasdef=true;
    //     }else if(curTokIs(tokenType::VOID)){
    //         if((IS_CONST(val_type.t))) 
    //             exit(12);
    //         hasdef=true;
    //         val_type.t=val_type.t|TYPE_VOID;
    //     }
    //     else {
    //         std::cerr<<"expect  type\n"<<curTok->literal<<endl;
    //         skipIfCurIs(tokenType::INT);
    //     }
    //     nextToken();
    // }
    // if(hasdef==false){
    //     skipIfCurIs(tokenType::INT);
    //     }
    // return val_type;
}
// std::unique_ptr<ast::Statement> Parser::parserStmts(){
//     std::unique_ptr<ast::Statement> ret;
//     if(curTokIs(tokenType::KW_RETURN)){
//         ret=parserRetStmt();
//     }else if(curTokIs(tokenType::KW_LET)||curTokIs(tokenType::KW_VAR)){
//         ret=parserValDefStmt();
//     }else if(curTokIs(tokenType::KW_IF)){
//         ret=parserIfStmt();
//     }else if(curTokIs(tokenType::KW_WHILE)){
//         ret=parserWhileStmt();
//     }else if(curTokIs(tokenType::LBRACE)){
//         ret=parserBlock();
//     }else if(curTokIs(tokenType::KW_CONTINUE)){
//         ret=make_unique<ast::ContinueStmt>(curTok->begin);
//     }else if(curTokIs(tokenType::KW_BREAK)){
//         ret=make_unique<ast::BreakStmt>(curTok->begin);
//     }else if(curTokIs(tokenType::SEMICOLON)){
//         ret=make_unique<ast::EmptyStmt>(curTok->begin);
//         nextToken();
//     }else if(curTokIs(tokenType::KW_STRUCT)){
//         ret=parserStructDecl();
//     }
//     else
//     // if(curTokIs(tokenType::IDENT)||curTokIs(tokenType::INT)||curTokIs(tokenType::FLOAT)){
//     //     ret=parserExprStmt();
//     // }
//     // else{
//     //     exit(14);
//     // }
//         ret=parserExprStmt();
//     return ret;
// }
unique_ptr<ast::CompunitNode> Parser::parserComp(){
    unique_ptr<ast::Statement> p_gval=nullptr;
    while(!curTokIs(tokenType::LEXEOF)){
        while(curTokIs(tokenType::SEMICOLON)){
            nextToken();
        }
        if(curTokIs(tokenType::KW_PACKAGE)){
            consumToken(tokenType::KW_PACKAGE);
            consumToken(tokenType::IDENT);
            if(curTokIs(tokenType::SEMICOLON)){
                nextToken();
            }
        }else if(curTokIs(tokenType::KW_STRUCT)){
            comp->global_defs.push_back(parserStructDecl());
            continue;
        }else if(curTokIs(tokenType::KW_FUNCTION)){
            comp->global_defs.push_back(parserFuncStmt());
	    continue;
	}else if(curTokIs(tokenType::KW_LET)||(curTokIs(tokenType::KW_VAR))){
            comp->global_defs.push_back(parserValDefStmt());            
	    continue;
	}else{
	    if(curTok->literal=="main"){
	        comp->global_defs.push_back(parserSpecialFuncStmt());
                continue;
	    }else{
	        assert(false);
	        parserExpr();
	    }
	  }
        // type::Type* val=parserDefType();
        unique_ptr<ast::DefStmt> p_gval=nullptr;
        if(peekTokIs(tokenType::LPAREM)){
            p_gval=parserFuncStmt();
            comp->global_defs.push_back(std::move(p_gval));
        }else{
            // unique_ptr<ast::ValDeclStmt> p=parserValDeclStmt(val);
            // for(auto &i:p->var_def_list){
            //     if(this->comp->isReDef(i->name)){
            //         std::cerr<<"re def"<<endl;
            //         exit(45);
            //     }else{
            //         comp->global_defs.push_back(std::move(i));
            //     }
            // }
        }

        
    }
    return nullptr;
}
// unique_ptr<ast::ValDeclStmt> Parser::parserValDeclStmt(type::Type* val_type){
//     unique_ptr<ast::ValDeclStmt>val_decl;
//     if(val_type->isMut()==false)
//         val_decl=make_unique<ast::ConstDeclStmt>(curTok->begin,val_type);
//     else
//         val_decl=make_unique<ast::ValDeclStmt>(curTok->begin,val_type);
//     while(!curTokIs(tokenType::SEMICOLON)){
//         val_decl->var_def_list.push_back(parserValDefStmt(val_type));
//         if(curTokIs(tokenType::SEMICOLON)){
//             continue;
//         }else if(curTokIs(tokenType::COMMA)){
//             nextToken();
//         }else skipIfCurIs(tokenType::SEMICOLON);
//     }
//     nextToken();
//     return val_decl;
// }
unique_ptr<ast::Statement> Parser::parserExprStmt(){
    auto estmt=make_unique<ast::ExprStmt>(curTok->begin,parserExpr());
    skipIfCurIs(tokenType::SEMICOLON);
    unique_ptr<ast::Statement> ret;
    if(estmt->expr->getType()==ast::ASSIGN_EXPR){
        auto expr=std::move(estmt->expr);
        ast::AssignExpr* aexp=(ast::AssignExpr*)expr.get();
        ret=make_unique<ast::AssignStmt>(estmt->pos,std::move(aexp->lhs),std::move(aexp->rhs));
        estmt.reset();
    }else{
        ret=std::move(estmt);
    }
    return ret;
}
// void Parser::parserStructDecl(ast::StructDeclStmt *const struct_decl){
// }
unique_ptr<ast::DefStmt> Parser::parserMember(){
  assert(false);
}
unique_ptr<ast::DefStmt> Parser::parserStructDecl(){
    auto pos=curTok->begin;
    consumToken(tokenType::KW_STRUCT);
    string name=std::move(curTok->literal);
    nextToken();
    consumToken(tokenType::LBRACE);
    unique_ptr<ast::StructDeclStmt> struct_decl=make_unique<ast::StructDeclStmt>(name,pos,nullptr);
    // auto parserStructInit=[this](){
    //     auto init=make_unique<ast::FuncDef>(curTok->literal,curTok->begin,nullptr);
    //     consumToken(tokenType::KW_INIT);
    //     consumToken(tokenType::LPAREM);
    //     parserArg(init->func_f_params);
    //     consumToken(RPAREM);
    //     // init->body=parserBlock();
    // 	assert(false);
    // 	return init;
    // };

    auto parserStructItem=[this,&struct_decl/*,&parserStructInit*/](){
        while(!curTokIs(tokenType::RBRACE)){
            type::perm perm=type::perm::DEFAULT;
            if(curTokIs(tokenType::KW_PUBLIC)){
                nextToken();
                perm=type::perm::PUBLIC;
            }else if(curTokIs(tokenType::KW_PROTECTED)){
                nextToken();
                perm=type::perm::PROTECTED;
            }else if(curTokIs(tokenType::KW_PRIVATE)){
                nextToken();
                perm=type::perm::PRIVATE;
            }
            if(curTokIs(tokenType::KW_INIT)){
                struct_decl->init=parserSpecialFuncStmt();
            }else if(curTokIs(tokenType::KW_LET)||curTokIs(tokenType::KW_VAR)){
                struct_decl->members.push_back({parserValDefStmt(),perm});
            }else if(curTokIs(tokenType::KW_FUNCTION)){
                struct_decl->members.push_back({parserFuncStmt(),perm});
            }else{assert(0);}
        }
    };
    parserStructItem();
    consumToken(tokenType::RBRACE);
    return struct_decl;
}
unique_ptr<ast::DefStmt> Parser::parserValDefStmt(){
    auto pos=curTok->begin;
    bool ismut=false;
    if(curTokIs(tokenType::KW_VAR)){
        ismut=true;
    }else if(curTokIs(tokenType::KW_LET)==false){
        // std::cerr<<curTok->literal;
        assert(0);
    }
    nextToken();
    unique_ptr<ast::DefStmt> ret=nullptr;
    if(!curTokIs(tokenType::IDENT)){
        exit(81);
    }
    if(peekTokIs(tokenType::LSQ_BRACE)){
        ret=parserArrDefStmt(ismut);
    }else{
        string name=std::move(curTok->literal);
        nextToken();
        unique_ptr<ast::ValDefStmt> tmp=make_unique<ast::ValDefStmt>(name,pos,nullptr,ismut);
        // type::Type*type=nullptr;
        if(curTokIs(tokenType::COLON)){
            nextToken();
            tmp->type=std::move(nextToken());
        }
        if(curTokIs(tokenType::ASSIGN)){
            nextToken();
            tmp->init_expr=parserExpr();
        }
        if(curTokIs(tokenType::SEMICOLON)){
            nextToken();
        }else if(last_pos_.line==curTok->begin.line){
	  std::cerr<<last_pos_.line<<std::endl;
	  std::cerr<<curTok->begin.line<<std::endl;
	  assert(false);
	}
        ret=std::move(tmp);
    }
    return ret;
}
unique_ptr<ast::ArrDefStmt> Parser::parserArrDefStmt(bool ismut){
    assert(0);
    // unique_ptr<ast::ArrDefStmt> ret;
    // if(ismut==false)
    //     ret=make_unique<ast::ConstArrDefStmt>(curTok->literal,curTok->begin,val_type);
    // else
    //     ret=make_unique<ast::ArrDefStmt>(curTok->literal,curTok->begin,val_type);
    // skipIfCurIs(tokenType::IDENT);
    // while(!curTokIs(tokenType::COMMA)&&!curTokIs(tokenType::SEMICOLON)&&!curTokIs(tokenType::ASSIGN)){
    //     skipIfCurIs(tokenType::LSQ_BRACE);
    //     ret->array_length.push_back(parserExpr());
    //     skipIfCurIs(tokenType::RSQ_BRACE);
    // }
    // if(curTokIs(tokenType::ASSIGN)){
    //     nextToken();
    //     ret->initializers=parserInitlizer();
    // }
    // return ret;
}
unique_ptr<ast::InitializerExpr> Parser::parserInitlizer(){
    unique_ptr<ast::InitializerExpr> ret=make_unique<ast::InitializerExpr>(last_pos_);
    skipIfCurIs(tokenType::LBRACE);
    if(curTokIs(tokenType::RBRACE)){
        skipIfCurIs(tokenType::RBRACE);
        return ret;
    }
    while(!curTokIs(tokenType::RBRACE)){
        if(curTokIs(tokenType::LBRACE)){
            ret->initializers.push_back(std::move( parserInitlizer()));
                //ret.insert(ret.end(), std::make_move_iterator(tmp.begin()), std::make_move_iterator(tmp.end()));
        }else 
            ret->initializers.push_back(parserExpr());
        
        if(curTokIs(tokenType::COMMA)){
            nextToken();
        }else if(curTokIs(tokenType::RBRACE)){
            break;
        }else{
            skipIfCurIs(tokenType::RBRACE);
        }
    }
    skipIfCurIs(tokenType::RBRACE);
    return ret;
}
unique_ptr<Token> Parser::parserFuncTy(){
    if(curTokIs(tokenType::COLON)){
	nextToken();
	return nextToken();
    }
    return nullptr;
}
unique_ptr<ast::FuncDef> Parser::parserFuncStmt(){
    Pos begin=curTok->begin;
    nextToken();
    string name=std::move(curTok->literal);
    consumToken(tokenType::IDENT);
    unique_ptr<Token>type=nullptr;
    if(curTokIs(tokenType::COLON)){
        nextToken();
        type=nextToken();
    }
    unique_ptr<ast::FuncDef> fun=make_unique<ast::FuncDef>(name,begin,std::move(type));
    consumToken(tokenType::LPAREM);
    
    //skip(
    // parserArg(fun->func_f_params);
    while(!curTokIs(tokenType::RPAREM)){
	string arg=curTok->literal;
	consumToken(tokenType::IDENT);
	consumToken(tokenType::COLON);
	fun->func_f_params.push_back({arg, nextToken()});
	if(curTokIs(tokenType::COLON)){
	    consumToken(tokenType::COMMA);
	}else{
	    assert(tokenType::RPAREM);
	}
    }
    consumToken(tokenType::RPAREM);
    // if(curTokIs(tokenType::SEMICOLON)){
    //     nextToken();
    // }else {
    btys.emplace(ScopeType::FUNC);
    fun->type=parserFuncTy();
    assert(curTokIs(tokenType::LBRACE));
    fun->body= unique_ptr<ast::BlockExpr>(static_cast<ast::BlockExpr*>(parserBlockExpr().release()));
    // }
    btys.pop();

    return fun;
}
unique_ptr<ast::FuncDef> Parser::parserSpecialFuncStmt(){
    Pos begin=curTok->begin;
    string name=std::move(curTok->literal);
    // consumToken({KW_INIT});
    static std::array<string const,2> const si{"main","init"};
    auto iter=std::find(si.begin(), si.end(), name);
    if(iter==si.end()){
      assert(false);
    }
    nextToken();
    unique_ptr<Token>type=nullptr;
    if(curTokIs(tokenType::COLON)){
        nextToken();
        type=nextToken();
    }
    unique_ptr<ast::FuncDef> fun=make_unique<ast::FuncDef>(name,begin,std::move(type));
    consumToken(tokenType::LPAREM);
    
    //skip(
    while(!curTokIs(tokenType::RPAREM)){
	string arg=curTok->literal;
	consumToken(tokenType::IDENT);
	consumToken(tokenType::COLON);
	fun->func_f_params.push_back({arg, nextToken()});
	if(curTokIs(tokenType::COLON)){
	    consumToken(tokenType::COMMA);
	}else{
	    assert(tokenType::RPAREM);
	}
    }
    consumToken(tokenType::RPAREM);
    // if(curTokIs(tokenType::SEMICOLON)){
    //     nextToken();
    // }else {
    btys.emplace(ScopeType::FUNC);
    fun->type=parserFuncTy();
    assert(curTokIs(tokenType::LBRACE));
    fun->body= unique_ptr<ast::BlockExpr>(static_cast<ast::BlockExpr*>(parserBlockExpr().release()));
    // }
    btys.pop();

    return fun;
}

unique_ptr<ast::RetStmt>  Parser::parserRetStmt( ){
    std::unique_ptr<ast::RetStmt> ret;
    ret=make_unique<ast::RetStmt>(curTok->begin);
    consumToken(tokenType::KW_RETURN);
    if(!curTokIs(tokenType::SEMICOLON))
        ret->expr=parserExpr();
    consumToken(tokenType::SEMICOLON);
    return ret;
}
void Parser::parserArg(std::vector<unique_ptr<ast::FuncFParam>> &argv){
    while(!curTokIs(tokenType::RPAREM)){
        // type::Type* type=parserDefType();
        // if(!curTokIs(tokenType::IDENT)){
        //     std::cerr<<"无形参"<<endl;
        //     exit(2);
        // }
        string id=curTok->literal;
        consumToken(tokenType::IDENT);
        consumToken(tokenType::COLON);
        auto type=nextToken();
        auto param=make_unique<ast::FuncFParam>(std::move(id),type->begin,std::move(type));
        while(curTokIs(tokenType::LSQ_BRACE)){
            skipIfCurIs(tokenType::LSQ_BRACE);
            if(curTokIs(tokenType::RSQ_BRACE)){
                param->index_num.push_back(nullptr);
            }else
                param->index_num.push_back(parserExpr());
            nextToken();
        }
        argv.push_back(std::move(param));
        if(curTokIs(tokenType::COMMA)){
            nextToken();
        }
        if(curTokIs(tokenType::RPAREM)){
            break;
        }   
    }

}
// unique_ptr<ast::IfStmt> Parser::parserIfStmt(){
//     unique_ptr<ast::IfStmt> ret=make_unique<ast::IfStmt>(curTok->begin);
//     skipIfCurIs(tokenType::KW_IF);
//     skipIfCurIs(tokenType::LPAREM);
//     ret->pred=parserExpr();
//     skipIfCurIs(tokenType::RPAREM);
//     //parserBlockItems(if_state->if_body);
//     ret->then_stmt=parserStmts();
//     if(curTokIs(tokenType::KW_ELSE)){
//         nextToken();
//         //parserBlockItems(if_state->else_body);
//         ret->else_stmt=parserStmts();
//     }
//     return ret;
// }
std::variant<unique_ptr<ast::ExprNode>,unique_ptr<ast::Statement>> Parser::parserStmtExpr(){
  unique_ptr<ast::ExprNode> expr;
  switch (curTok->type) {
  case::KW_VAR:
  case::KW_LET:
  case::KW_CONST:
    return std::variant<unique_ptr<ast::ExprNode>,unique_ptr<ast::Statement>>{parserValDefStmt()};
  case::KW_IF:
    return std::variant<unique_ptr<ast::ExprNode>,unique_ptr<ast::Statement>>{parserIfExpr()};
  case::KW_RETURN:
    return std::variant<unique_ptr<ast::ExprNode>,unique_ptr<ast::Statement>>{parserRetStmt()};
  default:
    expr=parserExpr();
  }
  if(curTok->begin.line>last_pos_.line){
    return expr;
  }
  if(curTokIs(tokenType::SEMICOLON )){
    nextToken();
    auto pos=expr->pos;
    auto ret=make_unique<ast::ExprStmt>(pos,std::move(expr));
    return ret;
  }else{
    assert(false);
  }
}
unique_ptr<ast::ExprNode> Parser::parserBlockExpr(){
  assert(btys.empty()==false);
  // ScopeType bty=btys.top();
  auto ret=make_unique<ast::BlockExpr>(curTok->begin,btys.top());
  consumToken(tokenType::LBRACE);
  while(!curTokIs(tokenType::RBRACE)){
    auto se=parserStmtExpr();
    auto expr_p=std::get_if<unique_ptr<ast::ExprNode>>(&se);
    if(expr_p!=nullptr){
      auto& expr=*expr_p;
      if(curTokIs(tokenType::RBRACE)){
	ret->expr_=std::move(expr);
      }else{
	auto pos=expr->pos;
	ret->stmts_.push_back(make_unique<ast::ExprStmt>(pos,std::move(expr)));
      }
    }else{
      auto stmt=std::get_if<unique_ptr<ast::Statement>>(&se);
      assert(stmt);
      ret->stmts_.push_back(std::move(*stmt));
    }
  }
  nextToken();
  return ret;
}
unique_ptr<ast::ExprNode> Parser::parserIfExpr(){
    unique_ptr<ast::IfExpr> ret=make_unique<ast::IfExpr>(curTok->begin);
    consumToken(tokenType::KW_IF);
    consumToken(tokenType::LPAREM);
    ret->cond_=parserExpr();
    consumToken(tokenType::RPAREM);
    //parserBlockItems(if_state->if_body);
    btys.emplace(ScopeType::IF);
    auto block=parserBlockExpr().release();
    btys.pop();
    ret->then_=unique_ptr<ast::BlockExpr>(static_cast<ast::BlockExpr*>(block));
    if(curTokIs(tokenType::KW_ELSE)){
        nextToken();
        //parserBlockItems(if_state->else_body);
        btys.emplace(ScopeType::ELSE);
	if(curTokIs(tokenType::KW_IF)){
	    ret->else_=parserIfExpr();
	}else
	    ret->else_=parserBlockExpr();
	// auto block=parserBlockExpr().release();

	// ret->else_=unique_ptr<ast::BlockExpr>(static_cast<ast::BlockExpr*>( block));
	btys.pop();
    }
    return ret;
}
unique_ptr<ast::ExprNode> Parser::parserWhileExpr(){
    unique_ptr<ast::WhileExpr> ret=make_unique<ast::WhileExpr>(curTok->begin);
    consumToken(tokenType::KW_IF);
    consumToken(tokenType::LPAREM);
    ret->cond_=parserExpr();
    consumToken(tokenType::RPAREM);
    //parserBlockItems(if_state->if_body);
    btys.emplace(ScopeType::LOOP);
    ret->loop_=unique_ptr<ast::BlockExpr>(static_cast<ast::BlockExpr*>( parserBlockExpr().release()));
    btys.pop();
    return ret;
}

// unique_ptr<ast::WhileStmt> Parser::parserWhileStmt(){
//     unique_ptr<ast::WhileStmt> ret=make_unique<ast::WhileStmt>(curTok->begin);
//     skipIfCurIs(tokenType::KW_WHILE);
//     skipIfCurIs(tokenType::LPAREM);
//     ret->pred=parserExpr();
//     skipIfCurIs(tokenType::RPAREM);
//     ret->loop_stmt=std::move(parserStmts());
//     return ret;
// }
unique_ptr<ast::ExprNode> Parser::parserConst(){
    ast::LitType type;
    if(curTokIs(tokenType::INT_BIN))
        type=ast::LitType::INT_BIN;
    else if(curTokIs(tokenType::INT_OCTAL))
        type=ast::LitType::INT_OCTAL;
    else if(curTokIs(tokenType::INT_HEX))
        type=ast::LitType::INT_HEX;
    else if(curTokIs(tokenType::INT))
        type=ast::LitType::INT;
    else if(curTokIs(tokenType::FLOAT))
        type=ast::LitType::FLOAT;
    else if(curTokIs(tokenType::KW_FALSE)||curTokIs(tokenType::KW_TRUE)){
	type=ast::LitType::BOOL;
    }
    else exit(0);
    // else if(curTokIs(tokenType::CHAR))

    // // ast::valUnion Value;
    // string s;
    // // if(curTokIs(tokenType::MINUS)||curTokIs(tokenType::PLUS)){
    // //     s={curTok->literal+peekTok->literal};
    // //     nextToken();
    // // }else{
    //     s={curTok->literal};
    // // }

    // try{
    //     if(curTokIs(tokenType::INT_BIN))
    //         Value.i=std::stol( s,0,2);
    //     else if(curTokIs(tokenType::INT_OCTAL))
    //         Value.i=std::stol( s,0,8);
    //     else if(curTokIs(tokenType::INT_HEX))
    //         Value.i=std::stol( s,0,16);
    //     else if(curTokIs(tokenType::INT))
    //         Value.i=std::stol( s);
    //     else 
    //         Value.f=std::stof( s);
    // }
    // catch (const std::exception&){
    // //     std::cerr<<"stol error"<<endl;
    // //     exit(15);
    // // }
    // unique_ptr<ast::Literal> Ilt;
    // if(curTokIs(tokenType::FLOAT))
    //     Ilt=make_unique<ast::FloatConst>(curTok->begin,Value);
    // else
    //     Ilt=make_unique<ast::IntConst>(curTok->begin,Value);
    // nextToken();
    // return Ilt;
    auto pos=curTok->begin;
    return make_unique<ast::Literal>(pos,nextToken(),type);
}
unique_ptr<ast::ExprNode> Parser::parserExpr(parserOpPrec prec){
    tokenType l_type=curTok->type;
    selectPreFn(l_type);
    if(prefixFn==nullptr){
        cout<<"cur tok "<<curTok->literal<<curTok->begin.line<<"  "<<curTok->begin.column;
        exit(35);
        // return nullptr;
    }
    auto leftExp=(this->*prefixFn)();
    while(curTokIs(tokenType::LPAREM)||curTokIs(tokenType::RIGHT_ARROW)||curTokIs(tokenType::DOT)){
        leftExp=parserSuffixExpr(std::move(leftExp));
    }
    // if(curTokIs(tokenType::LSQ_BRACE)){
    //     AddLvalIndex((ast::LvalExpr*)leftExp.get());
    // }
    while(!curTokIs(tokenType::SEMICOLON)&&prec<curPrecedence()&&last_pos_.line<=curTok->begin.line){
        selectInFn(curTok->type);
        if (InfixFn==nullptr){
            // return leftExp;
            exit(234);
        }
        leftExp=(this->*InfixFn)(std::move(leftExp));
    }
    return leftExp;
}
void  Parser::AddLvalIndex(ast::LvalExpr* lval ){
    while(curTokIs(tokenType::LSQ_BRACE)){
        skipIfCurIs(tokenType::LSQ_BRACE);
        if(!curTokIs(tokenType::RSQ_BRACE))
            lval->index_num.push_back(std::move(parserExpr()));
        else
            lval->index_num.push_back(nullptr);
        skipIfCurIs(tokenType::RSQ_BRACE);
    }
}
unique_ptr<ast::ExprNode> Parser::parserSuffixExpr(unique_ptr<ast::ExprNode> name){
    auto pos=name->pos;
    unique_ptr<ast::ExprNode> ret;
    if(curTokIs(tokenType::LPAREM)){
      ret=parserCall(std::move(name));
    }else if(curTokIs(tokenType::DOT)){
      auto selector=make_unique<ast::SelectorExpr>(pos,std::move(name));
      consumToken(tokenType::DOT);
      selector->rhs=nextToken();
      ret=std::move(selector);
    }else{
      assert(0);
    }
    return ret;
}
unique_ptr<ast::ExprNode> Parser::parserStr(){
    if(curTokIs(tokenType::QUOTE )==false&&curTokIs(tokenType::D_QUOTE )==false){
      return nullptr;
    }
    auto pos=curTok->begin;
    auto type=curTok->type==tokenType::QUOTE?ast::LitType::RUNE:ast::LitType::STRING;
    auto literal=make_unique<ast::Literal>(pos,std::move(curTok),type);
    return literal;
}
unique_ptr<ast::CallExpr> Parser::parserCall(unique_ptr<ast::ExprNode> name){
    if(name==nullptr){
        exit(114);
    }else if(name->getType()!=(int)ast::ExprType::LVAL_EXPR){
        exit(114);
    }
    unique_ptr<ast::CallExpr> ret=make_unique<ast::CallExpr>(curTok->begin,std::move(name));
    consumToken(tokenType::LPAREM);
    while(!curTokIs(tokenType::RPAREM)){
        ret->func_r_params.push_back(std::move(parserExpr()));
        if(curTokIs(tokenType::RPAREM)){
            continue;
        }else if(curTokIs(tokenType::COMMA))
            nextToken();
        else
            exit(191);
    }
    skipIfCurIs(tokenType::RPAREM);
    return ret;
}
// unique_ptr<ast::CallExpr> Parser::parserCall(){
//     unique_ptr<ast::CallExpr> ret=make_unique<ast::CallExpr>(curTok->tok_pos,curTok->literal);
//     skipIfCurIs(tokenType::IDENT);
//     skipIfCurIs(tokenType::LPAREM);
//     while(!curTokIs(tokenType::RPAREM)){
//         ret->arg.push_back(std::move(parserExpr()));
//         if(curTokIs(tokenType::RPAREM)){
//             continue;
//         }else if(curTokIs(tokenType::COMMA))
//             nextToken();
//         else
//             exit(191);
//     }
//     skipIfCurIs(tokenType::RPAREM);
//     return ret;
// }
unique_ptr<ast::ExprNode> Parser::parserLval(){
    unique_ptr<ast::LvalExpr> ret=make_unique<ast::LvalExpr>(curTok->begin,curTok->literal);
    // skipIfCurIs(tokenType::IDENT);
    consumToken({tokenType::IDENT,tokenType::KW_THIS});
        while(curTokIs(tokenType::LSQ_BRACE)){
            skipIfCurIs(tokenType::LSQ_BRACE);
            if(!curTokIs(tokenType::RSQ_BRACE))
                ret->index_num.push_back(std::move(parserExpr()));
            else
                exit(114);
                // ret->index_num.push_back(nullptr);
            skipIfCurIs(tokenType::RSQ_BRACE);
        }
    return ret;
}
unique_ptr<ast::ExprNode> Parser::parserGroupedExpr(){
    skipIfCurIs(tokenType::LPAREM);
    auto exp=parserExpr(parserOpPrec::LOWEST);
    // if(!curTokIs(tokenType::RPAREM)){
    //     return nullptr;
    // }
    skipIfCurIs(tokenType::RPAREM);
    return exp;
}
parserOpPrec Parser::curPrecedence(){
    parserOpPrec prefix;
    auto prec_it=precedences.find(curTok->type);
    if(prec_it!=precedences.end()){
        prefix=prec_it->second;
    }else prefix=parserOpPrec::LOWEST;
    return prefix;
}
unique_ptr<ast::ExprNode> Parser::parserPrefixExpr(){
    unique_ptr<ast::PrefixExpr> express=make_unique<ast::PrefixExpr>(curTok->begin);
    express->operat=(ast::UnOp)curTok->literal[0];
    nextToken();
    express->rhs=this->parserExpr(OP_PREFIX);
    return express;
}
unique_ptr<ast::ExprNode> Parser::parserInfixExpr(unique_ptr<ast::ExprNode>left){
    parserOpPrec curPrec=this->curPrecedence();
    unique_ptr<ast::InfixExpr> express;
    if(curTok->type<=tokenType::ESPERLUTTE&&curTok->type>=tokenType::PLUS){
        express=make_unique<ast::BinopExpr>(curTok->begin,std::move(left));
    }else if (curTokIs(tokenType::EQUAL)) {
        express=make_unique<ast::EqExpr>(curTok->begin,std::move(left));
    }else if (curTokIs(tokenType::D_OR)) {
        express=make_unique<ast::ORExp>(curTok->begin,std::move(left));
    }else if (curTokIs(tokenType::D_ESPERLUTTE)) {
        express=make_unique<ast::AndExp>(curTok->begin,std::move(left));
    }else{
        express=make_unique<ast::RelopExpr>(curTok->begin,std::move(left));
    }
    express->operat=strToBinop(curTok->literal);
    this->nextToken();
    express->rhs=parserExpr(curPrec);
    return express;    
}
unique_ptr<ast::ExprNode> Parser::parserAssignExpr(unique_ptr<ast::ExprNode> left){
    // parserOpPrec curPrec=this->curPrecedence();
    unique_ptr<ast::AssignExpr> express=make_unique<ast::AssignExpr>(curTok->begin,std::move(left));
    express->operat=strToBinop(curTok->literal);
    this->nextToken();
    //是否可以以最低优先级表示右结合
    express->rhs=parserExpr(parserOpPrec::LOWEST);
    return express;    
};
// unique_ptr<ast::BlockStmt>  Parser::parserBlockItems( ){
//     unique_ptr<ast::BlockStmt>  ret=make_unique<ast::BlockStmt>(curTok->begin);
//     unique_ptr<ast::Statement> tmp;
//     while (!curTokIs(tokenType::RBRACE)) {
//         tmp=parserStmts();
//         if(tmp!=nullptr){
//             ret->block_items.push_back(std::move(tmp));
//         }else{
//             exit(128);
//         }
//     }
//     return ret;
// }
// unique_ptr<ast::BlockStmt> Parser::parserBlock(){
//     consumToken(tokenType::LBRACE);
//     unique_ptr<ast::BlockStmt> ret=parserBlockItems();
//     consumToken(tokenType::RBRACE);
//     return ret;
// }
unique_ptr<Token> Parser::nextToken(){
    last_pos_=curTok->begin;
    auto ret=std::move(curTok);
    curTok=std::move(peekTok);
    peekTok=lex->nextToken();
    return ret;
}

bool inline Parser::curTokIs(tokenType type){
    return this->curTok->type==type;
}
bool inline Parser::peekTokIs(tokenType type){
    return this->peekTok->type==type;
}
void Parser::consumToken(std::vector<tokenType>&&type){
    // this->skipIfCurIs(type);
    if(std::find(type.begin(),type.end(),curTok->type)==type.end()){
        std::cerr<<file_name<<':'<<last_pos_.line<<':'<<last_pos_.column<<':'<<"expect "<<endl;
        exit(1);
    }
    nextToken();
}
void Parser::skipIfCurIs(tokenType type){
    if(!curTokIs(type)){
        static std::map<tokenType const,char const *const> m{
            {tokenType::FLOAT,"float"},
            {tokenType::INT,"int"},
            {tokenType::IDENT,"ident"},
            {tokenType::ASSIGN,"assign"},
            {tokenType::SEMICOLON,";"},
            {tokenType::COMMA,","},
            {tokenType::KW_INT64,"Int64"},
            {tokenType::KW_FLOAT64,"Float64"},
            {tokenType::LBRACE,"{"},
            {tokenType::RBRACE,"}"},
            {tokenType::LPAREM,"("},
            {tokenType::RPAREM,")"},
            {tokenType::LSQ_BRACE,"["},
            {tokenType::RSQ_BRACE,"]"},
        };
        auto i=m.find(type);
        const char*s;
        if(i!=m.end())
            s=i->second;
        else{
            std::cerr<<"no enough"<<endl;
            std::cerr<<file_name<<':'<<last_pos_.line<<':'<<last_pos_.column<<endl;
	    exit(228);
        }
        std::cerr<<file_name<<':'<<last_pos_.line<<':'<<last_pos_.column<<':'<<"expect "<<s<<endl;
        exit(2);
    }
    nextToken();
}

void Parser::selectPreFn(tokenType type){
    this->prefixFn=nullptr;
    switch (type) {
        case tokenType::KW_IF:
          prefixFn=&Parser::parserIfExpr;
        case tokenType::KW_WHILE:
          prefixFn=&Parser::parserWhileExpr;
        case tokenType::IDENT:
        case tokenType::KW_THIS:
            if(peekTokIs(tokenType::LSQ_BRACE))
                prefixFn=&Parser::parserLval;
            else
                prefixFn=&Parser::parserLval;
            break;
        case tokenType::BANG:
        case tokenType::PLUS:
        case tokenType::MINUS:
            prefixFn=&Parser::parserPrefixExpr;
            break;
        case tokenType::INT:
        case tokenType::INT_BIN:
        case tokenType::INT_HEX:
        case tokenType::INT_OCTAL:
        case tokenType::FLOAT:
            prefixFn=&Parser::parserConst;
            break;
        case tokenType::KW_FALSE:
        case tokenType::KW_TRUE:
	    prefixFn=&Parser::parserConst;
	    break;
        case tokenType::QUOTE:
        case tokenType::D_QUOTE:
	  prefixFn=&Parser::parserStr;
	  break;
        case tokenType::LPAREM:
            prefixFn=&Parser::parserGroupedExpr;
            break;
        default:
            this->prefixFn=nullptr;
            break;

    }
    // cout<<curTok->literal<<endl;
}
void Parser::selectInFn(tokenType type){
    switch (type) {
        case tokenType::PLUS:
        case tokenType::MINUS:
        case tokenType::SLASH:
        case tokenType::ASTERISK:
        case tokenType::EQUAL:
        case tokenType::NOTEQUAL:
        case tokenType::LT:
        case tokenType::GT:
        case tokenType::LE:
        case tokenType::GE:  
        case tokenType::OR:
        case tokenType::D_OR:
        case tokenType::ESPERLUTTE:
        case tokenType::D_ESPERLUTTE:
        case tokenType::MOD:
            Parser::InfixFn=&Parser::parserInfixExpr;
            break;
        case tokenType::ASSIGN:
            Parser::InfixFn=&Parser::parserAssignExpr;
            break;
        default:
            this->InfixFn=nullptr;
            break;

    }
}

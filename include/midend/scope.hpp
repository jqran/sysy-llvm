#ifndef SCOPE_HPP
#define SCOPE_HPP

#include "midend/type.hpp"
#include <cassert>
#include <iostream>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// template <typename  T>
enum class DefTy {
    CONST,
    LET,
    FUNC_PARAM,
    VAR,
};
struct IRInfo{
    // std::string id;
    DefTy defty;
    llvm::Value* val;
    type::Type const *ty;
    llvm::Type *llvm_ty;
    bool ismut(){return this->defty==DefTy::VAR;}
};

struct IRScope{
    IRScope* parent_;
    std::vector<std::unique_ptr<IRScope>> children_;
    std::unordered_map<std::string,IRInfo> table_;

    void setParent(IRScope* p){
        this->parent_=p;
    }


public:
    IRScope(IRScope* parent):parent_(parent){}
    IRScope*getParent(){
        return this->parent_;
    }
    void insert(std::pair<std::string,IRInfo>p){
        table_.insert(p);
    }
    IRInfo findValue(std::string &name){
        auto iter=table_.find(name);
        if(iter!=table_.end())
            return iter->second;

        if(parent_!=nullptr)
            return parent_->findValue(name);
        else {
	    std::cerr<<name<<std::endl;
	    assert(0);
            // return nullptr;

	}
    }
    // void enter(){

    // }
    // void exit(){
    //     if(parent_!=nullptr)
    //         this=parent_;
    //     else
    //         exit(200);
    // }
};
struct TableManager{
    std::unique_ptr<IRScope> const global_;
    IRScope* curr_;
    TableManager():global_(std::make_unique<IRScope>(nullptr)),curr_(global_.get()){}
    void enter(){
        std::unique_ptr<IRScope> child=std::make_unique<IRScope>(curr_);
        auto temp=child.get();
        curr_->children_.push_back(std::move(child));
        curr_=temp;
    }
    void exit(){
        curr_=curr_->getParent();
    }
    IRInfo findValue(string id){
        return curr_->findValue(id);
    }
    void insert(std::pair<std::string,IRInfo>p){
        this->curr_->insert(p);
    }
};
#endif

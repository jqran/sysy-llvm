#ifndef SCOPE_HPP
#define SCOPE_HPP

#include "frontend/type.hpp"
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
struct IRInfo{
    // std::string id;
    llvm::Value* val;
    type::Type const *ty;
    llvm::Type *contain_ty;
};
class IRScope{
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
    IRScope():parent_(nullptr){}
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
	    std::cerr<<name<<endl;
	    assert(0);
            // return nullptr;

	}
    }
    void enter(){
        std::unique_ptr<IRScope> child=std::make_unique<IRScope>(this);
        children_.push_back(std::move(child));
    }
    // void exit(){
    //     if(parent_!=nullptr)
    //         this=parent_;
    //     else
    //         exit(200);
    // }
};

#endif

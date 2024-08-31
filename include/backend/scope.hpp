#ifndef SCOPE_HPP
#define SCOPE_HPP

#include <llvm/IR/Value.h>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
namespace gen {


// template <typename  T>
class Scope{
    Scope* parent_;
    std::vector<std::unique_ptr<Scope>> children_;
    std::unordered_map<std::string,llvm::Value*> table_;

    void setParent(Scope* p){
        this->parent_=p;
    }


public:
    Scope(Scope* parent):parent_(parent){}
    Scope*getParent(){
        return this->parent_;
    }
    Scope():parent_(nullptr){}
    void insert(std::pair<std::string,llvm::Value*>p){
        table_.insert(p);
    }
    llvm::Value* findValue(std::string &name){
        auto iter=table_.find(name);
        if(iter!=table_.end())
            return iter->second;

        if(parent_!=nullptr)
            return parent_->findValue(name);
        else 
            return nullptr;
    }
    void enter(){
        std::unique_ptr<Scope> child=std::make_unique<Scope>(this);
        children_.push_back(std::move(child));
    }
    // void exit(){
    //     if(parent_!=nullptr)
    //         this=parent_;
    //     else
    //         exit(200);
    // }
};

}
#endif
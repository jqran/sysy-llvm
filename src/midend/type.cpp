#include "midend/type.hpp"
#include "midend/hir.hpp"
#include <cassert>

namespace type {
    // Info::Info(long l):size(l){
    // }
    // ValType::ValType():t(0x0),size(0x0){}
StructType const *const TypeManager::addStructType(chir::StructDecl * s){
    auto id=s->name_;
    StructType const*   decl_ty=new StructType(id,s);
    s->ty_=decl_ty;
    this->decl_type.insert({id,decl_ty});
    return decl_ty;
}
EnumType const *const TypeManager::addEnumType(chir::EnumDecl * s){
    auto id=s->name_;
    EnumType const*   decl_ty=new EnumType(id,s);
    s->ty_=decl_ty;
    this->decl_type.insert({id,decl_ty});
    return decl_ty;
}
ArrayType::ArrayType(Type const* element,bool isdynamic,ssize_t size):Type(TypeId::ARRAY,0,"Array"+element->name_),element_(element),isdynamic_(isdynamic),size_(size){
}
bool  Type::operator==(Type const& other)const{
    assert(0);
    if(this->isNum()||this->type==TypeId::UNIT||this->type==TypeId::BOOL){
        return type==other.type&&getSize()==other.getSize();
    }else{
        // auto l=(DeclType*)this;
        // auto r=(DeclType*)&other;
        // return l->name_==r->name_;
    }
    }
    ArrayType const * TypeManager::getArrayTy(Type const*ty ){
        if(auto iter=array_type.find(ty);iter!=array_type.end()){
            return iter->second;
        }
        auto ret=new ArrayType(ty);
        array_type.insert({ty,ret});
        return ret;
    }
}
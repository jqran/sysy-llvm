#ifndef TYPE__HPP
#define TYPE__HPP
#include <cassert>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <utility>
#include <vector>
#include "frontend/lex.hpp"
using std::string,std::vector;
namespace type {

enum class perm{
    PUBLIC=1,
    DEFAULT,
    PROTECTED,
    PRIVATE,
};

#define TYPE_CONST (1<<0)
#define TYPE_STATIC (1<<1)
#define TYPE_U_INT (1<<3)
#define TYPE_INT (1<<2)
#define TYPE_FLOAT (1<<4)
// #define TYPE_STRUCT (1<<5)
//void only in func
#define TYPE_VOID (1<<7)

#define IS_CONST(i)  (TYPE_CONST&i)
#define IS_STATIC(i) (TYPE_STATIC&i)
#define IS_U_INT(i) (TYPE_U_INT&i)
#define IS_INT(i) (TYPE_INT&i)
#define IS_FLOAT(i) (TYPE_FLOAT&i)
// #define IS_STRUCT(i) (TYPE_STRUCT&i)
#define IS_VOID(i) (TYPE_VOID&i)


#define ADD_CONST(i)  (i=(TYPE_CONST|i))
#define ADD_STATIC(i) (i=(TYPE_STATIC|i))
#define ADD_U_INT(i) (i=(TYPE_U_INT|i))
#define ADD_INT(i) (i=(TYPE_INT|i))
#define ADD_FLOAT(i) (i=(TYPE_FLOAT|i))
// #define ADD_STRUCT(i) (i=(TYPE_STRUCT|i))
#define ADD_VOID(i) (i=(TYPE_VOID|i))


#define IS_NUM(i)  ((TYPE_FLOAT|TYPE_INT)&i)
typedef int type ;
struct DeclType;

union Info{
    DeclType * pointer;
    long size;
    Info(long );
};
enum class TypeId:int{
    VOID,
    INT,
    UINT,
    FLOAT,
    STRUCT,
    ARRAY,
    UNIT,
};
class Type{
public:
    // bool mut;
    TypeId const type;
private:
    uint size;
public:
    uint inline getSize()const{ return this->size;}
    void inline setSize(uint size){ this->size=size;}
    Type(TypeId id,uint size):type(id),size(size){}
    bool operator==(Type const& other)const;
};
class BuiltinType:public Type{
public:
    bool operator==(Type const& other)const{
        return type==other.type&&getSize()==other.getSize();
    }
    using Type::Type;
};
class DeclType:public Type{
public:
    string name_;
    struct Member{
        string name_;
        Type const *type_;
        perm per_;
    };
    vector<Member> members;
    DeclType(string name):Type(TypeId::STRUCT,0),name_(name){
    }
    // bool operator==(Type&other){
        // if(other.type!=TypeId::STRUCT){
        //     return false;
        // }
    // }
};
bool inline Type::operator==(Type const& other)const{
    if(this->type!=TypeId::STRUCT||other.type!=TypeId::STRUCT){
        return type==other.type&&getSize()==other.getSize();
    }else{
        auto l=(DeclType*)this;
        auto r=(DeclType*)&other;
        return l->name_==r->name_;
    }
}
class TypeManager{
    // std::array<BuiltinType,F16+1> builtin_type;
    std::map<string const, unique_ptr<Type const>> decl_type;
    public:
    BuiltinType const* int_liter =new BuiltinType(TypeId::INT,0);
    BuiltinType const* void_ =new BuiltinType(TypeId::VOID,0);
    BuiltinType const*float_liter= new BuiltinType(TypeId::FLOAT,0);
    Type const *unit_=nullptr;
    ~TypeManager(){
      // delete int_liter;
      // delete float_liter;
      // delete unit_;
    }
    // BuiltinType* getBuiltin(TypeId id)noexcept{
    //     if(id>F16){
    //         return nullptr;
    //     }
    //     return &builtin_type[id];
    // }
    BuiltinType const*getVoid(){
	return void_;
    }
    DeclType *addDeclType(unique_ptr<DeclType> t){
        auto &name=t->name_;
        if(decl_type.count(name))
            return nullptr;
        auto ret=t.get();
        decl_type.insert({name,std::move(t)});
        return ret;
    }
    Type const *getType(std::string s)noexcept{
        auto const iter=decl_type.find(s);
        if(iter!=decl_type.end()){
            return iter->second.get();
        }else{
            // assert(0);
            return nullptr;
        }
    }
    // Type* getType(string &s){

    // }
public:
    TypeManager(){
        decl_type.insert({"Int64",std::move(std::make_unique<BuiltinType>(TypeId::INT,64))});
        decl_type.insert({"Int32",std::move(std::make_unique<BuiltinType>(TypeId::INT,32))});
        decl_type.insert({"Int16",std::move(std::make_unique<BuiltinType>(TypeId::INT,16))});
        decl_type.insert({"Int8",std::move(std::make_unique<BuiltinType>(TypeId::INT,8))});
        decl_type.insert({"Bool",std::move(std::make_unique<BuiltinType>(TypeId::INT,1))});
        decl_type.insert({"UInt64",std::move(std::make_unique<BuiltinType>(TypeId::UINT,64))});
        decl_type.insert({"UInt32",std::move(std::make_unique<BuiltinType>(TypeId::UINT,32))});
        decl_type.insert({"UInt16",std::move(std::make_unique<BuiltinType>(TypeId::UINT,16))});
        decl_type.insert({"UInt8",std::move(std::make_unique<BuiltinType>(TypeId::UINT,8))});
        decl_type.insert({"Float64",std::move(std::make_unique<BuiltinType>(TypeId::FLOAT,64))});
        decl_type.insert({"Float32",std::move(std::make_unique<BuiltinType>(TypeId::FLOAT,32))});
        decl_type.insert({"Float16",std::move(std::make_unique<BuiltinType>(TypeId::FLOAT,16))});
        decl_type.insert({"Unit",std::move(std::make_unique<BuiltinType>(TypeId::UNIT,16))});
	this->unit_=decl_type["Unit"].get();
    }
};
// struct Name  {
// 	string value;
// };

// struct Type  {
// };
// struct Array  {
//     long len  ;
//     ValType elem;
// };
}
// enum ValType{
//     INT_VAL=1,
//     INT_CONST,
//     INT_POINT,
//     INT_POINT_CONST,
//     FLOAT_VAL,
//     FLOAT_CONST,
//     FLOAT_POINT,
//     FLOAT_POINT_CONST,
//     VOID_VAL,
// };
// struct type{
//     bool is_float=0;
//     bool is_const=0;
//     bool is_void=0;
//     bool un_useb=0;
// };
#endif

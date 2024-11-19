#ifndef TYPE__HPP
#define TYPE__HPP
#include <array>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>
#include "frontend/lex.hpp"
using std::string,std::vector;
namespace type {



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
enum TypeId:size_t{
    I64,
    I32,
    I16,
    I8,
    U64,
    U32,
    U16,
    U8,
    F64,
    F32,
    F16,
    STRUCT,
};
class Type{
    // bool mut;
    size_t size;
    TypeId type;
// public:
//     inline bool isMut(){
//         return mut;
//     }
};
class BuiltinType:public Type{
};
class DeclType:public Type{
    string name;
    vector<std::pair<string,Type*>> members;    
};
class TypeManager{
    std::array<BuiltinType,F16+1> builtin_type;
    std::map<string, unique_ptr<DeclType>> decl_type;
    public:
    BuiltinType* getBuiltin(TypeId id)noexcept{
        if(id>F16){
            return nullptr;
        }
        return &builtin_type[id];
    }
    bool addDeclType(string name,unique_ptr<DeclType> t){
        if(decl_type.count(name))
            return false;
        decl_type.insert({name,std::move(t)});
        return true;
    }
    DeclType*getStruct(string&s)noexcept{
        auto const iter=decl_type.find(s);
        if(iter!=decl_type.end()){
            return iter->second.get();
        }else{
            return nullptr;
        }
    }
    // Type* getType(string &s){

    // }
    TypeManager(){

    }
    ~TypeManager(){}
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
#ifndef TYPE__HPP
#define TYPE__HPP
#include <cassert>
#include <cstddef>
#include <map>
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
    std::map<std::string_view const, Type const* const> decl_type;
    BuiltinType const *const i64_;
    BuiltinType const *const i32_;
    BuiltinType const *const i16_;
    BuiltinType const *const i8_;
    BuiltinType const *const u64_;
    BuiltinType const *const u32_;
    BuiltinType const *const u16_;
    BuiltinType const *const u8_;
    BuiltinType const *const bool_;
    BuiltinType const *const f64_;
    BuiltinType const *const f32_;
    BuiltinType const *const f16_;
    BuiltinType const *const unit_;
public:
    BuiltinType const* const int_liter =new BuiltinType(TypeId::INT,0);
    BuiltinType const* const void_;
    BuiltinType const* const float_liter= new BuiltinType(TypeId::FLOAT,0);
    // Type const *unit_=nullptr;
    // Type const *unit_=nullptr;
    ~TypeManager(){
	delete int_liter;
	delete float_liter;
	for(auto [_,p]:decl_type){
	    delete p;
	}
    }
    // BuiltinType* getBuiltin(TypeId id)noexcept{
    //     if(id>F16){
    //         return nullptr;
    //     }
    //     return &builtin_type[id];
    // }
    inline BuiltinType const*getVoid()const{
	return void_;
    }
    inline BuiltinType const*getUnit()const{
	return unit_;
    }
    inline BuiltinType const*getI64()const{
	return i64_;
    }
    inline BuiltinType const*getI32()const{
	return i32_;
    }
    inline BuiltinType const*getI16()const{
	return i16_;
    }
    inline BuiltinType const*getI8()const{
	return i8_;
    }
    inline BuiltinType const*getBool()const{
	return bool_;
    }
    inline BuiltinType const*getU64()const{
	return u64_;
    }
    inline BuiltinType const*getU32()const{
	return u32_;
    }
    inline BuiltinType const*getU16()const{
	return u16_;
    }
    inline BuiltinType const*getU8()const{
	return u8_;
    }    
    inline BuiltinType const*getF64()const{
	return f64_;
    }
    inline BuiltinType const*getF32()const{
	return f32_;
    }
    inline BuiltinType const*getF16()const{
	return f16_;
    }
    DeclType const *const addDeclType(DeclType * const t){
        auto &name=t->name_;
        if(decl_type.count(name))
            return nullptr;
        auto ret=t;
        decl_type.insert({name,t});
        return ret;
    }
    Type const * const getType(std::string s)noexcept{
        auto const iter=decl_type.find(s);
        if(iter!=decl_type.end()){
            return iter->second;
        }else{
            // assert(0);
            return nullptr;
        }
    }
    // Type* getType(string &s){

    // }
public:
    TypeManager():i64_(new BuiltinType(TypeId::INT,64)),
		  i32_(new BuiltinType(TypeId::INT,32)),
		  i16_(new BuiltinType(TypeId::INT,16)),
		  i8_(new BuiltinType(TypeId::INT,8)),
		  bool_(new BuiltinType(TypeId::INT,1)),
		  u64_(new BuiltinType(TypeId::UINT,64)),
		  u32_(new BuiltinType(TypeId::UINT,32)),
		  u16_(new BuiltinType(TypeId::UINT,16)),
		  u8_(new BuiltinType(TypeId::UINT,8)),
		  f64_(new BuiltinType(TypeId::FLOAT,64)),
		  f32_(new BuiltinType(TypeId::FLOAT,32)),
		  f16_(new BuiltinType(TypeId::FLOAT,16)),
		  unit_(new BuiltinType (TypeId::UNIT,16)),
		  void_(new BuiltinType(TypeId::VOID,0))
    {
        decl_type.insert({"Int64",i64_});
        decl_type.insert({"Int32",i32_});
        decl_type.insert({"Int16",i16_});
        decl_type.insert({"Int8",i8_});
        decl_type.insert({"Bool",bool_});
        decl_type.insert({"UInt64",u64_});
        decl_type.insert({"UInt32",u32_});
        decl_type.insert({"UInt16",u16_});
        decl_type.insert({"UInt8",u8_});
        decl_type.insert({"Float64",f64_});
        decl_type.insert({"Float32",f32_});
        decl_type.insert({"Float16",f16_});
        decl_type.insert({"Unit",unit_});
	decl_type.insert({"void",void_});
	// this->unit_=decl_type["Unit"];
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

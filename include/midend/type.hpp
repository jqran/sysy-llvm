#ifndef TYPE__HPP
#define TYPE__HPP
#include <cassert>
#include <climits>
#include <map>
#include <string>
#include <sys/types.h>
#include <utility>

#include <vector>
using std::string,std::vector;
namespace  chir{
    struct StructDecl;
    struct EnumDecl;
    struct Module;
}
namespace type {

enum class perm{
    PUBLIC=1,
    DEFAULT,
    PROTECTED,
    PRIVATE,
};

// #define TYPE_CONST (1<<0)
// #define TYPE_STATIC (1<<1)
// #define TYPE_U_INT (1<<3)
// #define TYPE_INT (1<<2)
// #define TYPE_FLOAT (1<<4)
// // #define TYPE_STRUCT (1<<5)
// //void only in func
// #define TYPE_VOID (1<<7)

// #define IS_CONST(i)  (TYPE_CONST&i)
// #define IS_STATIC(i) (TYPE_STATIC&i)
// #define IS_U_INT(i) (TYPE_U_INT&i)
// #define IS_INT(i) (TYPE_INT&i)
// #define IS_FLOAT(i) (TYPE_FLOAT&i)
// // #define IS_STRUCT(i) (TYPE_STRUCT&i)
// #define IS_VOID(i) (TYPE_VOID&i)


// #define ADD_CONST(i)  (i=(TYPE_CONST|i))
// #define ADD_STATIC(i) (i=(TYPE_STATIC|i))
// #define ADD_U_INT(i) (i=(TYPE_U_INT|i))
// #define ADD_INT(i) (i=(TYPE_INT|i))
// #define ADD_FLOAT(i) (i=(TYPE_FLOAT|i))
// // #define ADD_STRUCT(i) (i=(TYPE_STRUCT|i))
// #define ADD_VOID(i) (i=(TYPE_VOID|i))

// #define IS_NUM(i)  ((TYPE_FLOAT|TYPE_INT)&i)
typedef int type ;
struct StructType;

// union Info{
//     StructType * pointer;
//     long size;
//     Info(long );
// };
enum class TypeId:int{
    BOOL,
    INT,
    UINT,
    FLOAT,
    STRUCT,
    ENUM,
    ARRAY,
    UNIT,
    POINT,
    FUNC,
};
class Type{
public:
    // bool mut;
    TypeId const type;
    string const name_;
private:
    uint size;
public:
    bool isUInt() const{
        return this->type==TypeId::UINT;
    }    
    bool isSInt() const{
        return this->type==TypeId::INT;
    }
    bool isInt() const{
        return isUInt()||isSInt();
    }
    bool isFloat() const{
        return this->type==TypeId::FLOAT;
    }
    bool isNum() const{
        return this->type==TypeId::FLOAT||this->type==TypeId::INT||this->type==TypeId::UINT;
    }
    bool isUnit() const{
        return this->type==TypeId::UNIT;
    }    
    bool isLit() const{
        return this->isNum()&&this->size==0;
    }
    bool isBool()const{
        return this->type==TypeId::BOOL&&this->size==1;
    }
    bool isArray()const{
        return this->type==TypeId::ARRAY;
    }
    bool isFunc() const{
        return this->type==TypeId::FUNC;
    }
    bool isStruct()const{
        return this->type==TypeId::STRUCT;
    }
    bool isPoint()const{
        return this->type==TypeId::POINT;
    }
    uint inline getSize()const{ return this->size;}
    void inline setSize(uint size){ this->size=size;}
    // Type(TypeId id,uint size):type(id),size(size){}
    Type(TypeId id,uint size,std::string name):type(id),size(size),name_(name){}
    bool operator==(Type const& other)const;
};
class BuiltinType:public Type{
public:
    bool operator==(Type const& other)const{
        return type==other.type&&getSize()==other.getSize();
    }
    using Type::Type;
};
struct ArrayType:public Type {
    Type const* const element_; // 数组元素的类型
    bool isdynamic_;    // 标识该数组是否是动态数组
    ssize_t size_;     // 数组的大小（对于静态数组，大小是已知的，对于动态数组，可以是运行时变量）
    ArrayType(Type const* element,bool isdynamic=true,ssize_t size=0);
    // void gen();
};
struct PointType:public Type {
    Type const* const element_; // 数组元素的类型
    PointType(Type const* element):Type(TypeId::POINT,0,element->name_+'*'),element_(element){}
};

class FuncType:public Type{
public:
    Type const* ret_ty_;
    vector<Type const* > types_;
    bool operator==(FuncType const& other)const{
        if(this->ret_ty_==other.ret_ty_){
            if(this->types_.size()==other.types_.size()){
                auto size=types_.size();
                for(auto i=0;i<size;++i){
                    if(types_[i]!=other.types_[i]){
                        return false;
                    }
                }
            }
        }
        return true;
    }
    FuncType(Type const* ret,vector<Type const* > types):Type(TypeId::FUNC,8,""),ret_ty_(ret),types_(std::move(types)){}

};
class StructType:public Type{
public:
    chir::StructDecl *const struct_;
    // struct Member{
    //     string name_;
    //     Type const *type_;
    //     perm per_;
    // };
    // vector<Member> members;
    StructType(string name,chir::StructDecl* const struct_):Type(TypeId::STRUCT,0,name),struct_(struct_){
    }
    // bool operator==(Type&other){
        // if(other.type!=TypeId::STRUCT){
        //     return false;
        // }
    // }
};

class EnumType:public Type{
public:
    chir::EnumDecl *const enum_;
    // struct Member{
    //     string name_;
    //     Type const *type_;
    //     perm per_;
    // };
    // vector<Member> members;
    EnumType(string name,chir::EnumDecl* const enum_):Type(TypeId::ENUM,0,name),enum_(enum_){
    }
    // bool operator==(Type&other){
        // if(other.type!=TypeId::STRUCT){
        //     return false;
        // }
    // }
};
    

class TypeManager{
    // std::array<BuiltinType,F16+1> builtin_type;
public:
    chir::Module* module_=nullptr;
private:
    std::map<std::string const, Type const* const> decl_type;
    std::map<Type const* const, PointType const* const> point_type;
    std::map<Type const* const, ArrayType const* const> array_type;
    std::vector<FuncType const*> func_types_;
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
    std::map<std::pair<::type::Type const*,::type::Type const*>,int>distmap;
public:
    BuiltinType const* const int_liter =new BuiltinType(TypeId::INT,0,"");
    BuiltinType const* const float_liter= new BuiltinType(TypeId::FLOAT,0,"");
    int findDist(Type const* l,Type const*r){
        if(l==r){
            return 0;
        }else{
            auto dist=distmap.find({l,r});
            if(dist!=distmap.end()){
                return dist->second;
            }else{
                return INT_MIN;
            }
        }
    }
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
    
    ArrayType const *getArrayTy(Type const*ty );
    FuncType const* getFuncTy(Type const *ret_ty,std::vector<Type const*> params)const{
        for(auto func_ty:func_types_){
            bool eq=true;
            if(func_ty->ret_ty_!=ret_ty){
                continue;
            }

            if(params.size()==func_ty->types_.size()){
                auto size=params.size();
                for(auto i=0;i<size;++i){
                    if(func_ty->types_[i]!=params[i]){
                        eq=false;
                    }
                    break;
                }
                if(eq)
                    return func_ty;
            }
        }
        return new FuncType(ret_ty,std::move(params));
    }
    StructType const *const addStructType(chir::StructDecl* const s);
    EnumType const *const addEnumType(chir::EnumDecl* const s);
    // StructType const *const addStructType(StructType * const t){
    //     auto &name=t->name_;
    //     if(decl_type.count(name))
    //         return nullptr;
    //     auto ret=t;
    //     decl_type.insert({name,t});
    //     return ret;
    // }
    std::map<Type const* const, ArrayType const* const> const& getAllArrayType(){
        return this->array_type;
    }
    Type const * const getType(std::string s)const noexcept{
        auto const iter=decl_type.find(s);
        if(iter!=decl_type.end()){
            return iter->second;
        }else{
            // assert(0);
            return nullptr;
        }
    }
    PointType const * const getPointType(std::string s){
        auto ty=getType(s);
        return point_type.at(ty);
    }
    PointType const * const getPointType(Type const *ty){
        auto iter= point_type.find(ty);
        if(iter==point_type.end()){
            // point_type.insert(PointType);
            PointType  const* const p=new PointType(ty);
            point_type.insert({ty,p});
            return p;
        }else{
            return iter->second;
        }
    }
    // Type* getType(string &s){

    // }
    inline Type const*getUnit()const{
        return unit_;
        }
        inline Type const*getI64()const{
        return i64_;
        }
        inline Type const*getI32()const{
        return i32_;
        }
        inline Type const*getI16()const{
        return i16_;
        }
        inline Type const*getI8()const{
        return i8_;
        }
        inline Type const*getBool()const{
        return bool_;
        }
        inline Type const*getU64()const{
        return u64_;
        }
        inline Type const*getU32()const{
        return u32_;
        }
        inline Type const*getU16()const{
        return u16_;
        }
        inline Type const*getU8()const{
        return u8_;
        }    
        inline Type const*getF64()const{
        return f64_;
        }
        inline Type const*getF32()const{
        return f32_;
        }
        inline Type const*getF16()const{
        return f16_;
        }

public:

    TypeManager():i64_(new BuiltinType(TypeId::INT,64,"Int64")),
		  i32_(new BuiltinType(TypeId::INT,32,"Int32")),
		  i16_(new BuiltinType(TypeId::INT,16,"Int16")),
		  i8_(new BuiltinType(TypeId::INT,8,"Int8")),
		  bool_(new BuiltinType(TypeId::BOOL,1,"Bool")),
		  u64_(new BuiltinType(TypeId::UINT,64,"UInt64")),
		  u32_(new BuiltinType(TypeId::UINT,32,"UInt32")),
		  u16_(new BuiltinType(TypeId::UINT,16,"UInt16")),
		  u8_(new BuiltinType(TypeId::UINT,8,"UInt8")),
		  f64_(new BuiltinType(TypeId::FLOAT,64,"Float64")),
		  f32_(new BuiltinType(TypeId::FLOAT,32,"Float32")),
		  f16_(new BuiltinType(TypeId::FLOAT,16,"Float16")),
		  unit_(new BuiltinType (TypeId::UNIT,16,"Unit"))

		//   void_(new BuiltinType(TypeId::VOID,0))
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
        distmap={
            {{this->getI64(),this->int_liter},0x0},
            {{this->int_liter,this->getI64()},0x0},
            {{this->getU64(),this->int_liter},0x1},
            {{this->int_liter,this->getU64()},0x1},
            {{this->getI32(),this->int_liter},0x10},
            {{this->int_liter,this->getI32()},0x10},
            {{this->getU32(),this->int_liter},0x11},
            {{this->int_liter,this->getU32()},0x11},
            {{this->getI16(),this->int_liter},0x100},
            {{this->int_liter,this->getI16()},0x100},
            {{this->getU16(),this->int_liter},0x110},
            {{this->int_liter,this->getU16()},0x110},
            {{this->getI8(),this->int_liter},0x1000},
            {{this->int_liter,this->getI8()},0x1000},
            {{this->getU8(),this->int_liter},0x1100},
            {{this->int_liter,this->getU8()},0x1100},
            
            {{this->getF64(),this->float_liter},0x0},
            {{this->float_liter,this->getF64()},0x0},
            {{this->getF32(),this->float_liter},0x10},
            {{this->float_liter,this->getF32()},0x10},
            {{this->getF16(),this->float_liter},0x100},
            {{this->float_liter,this->getF16()},0x100},
        };
	// decl_type.insert({"void",void_});
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

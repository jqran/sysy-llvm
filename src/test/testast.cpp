#include "frontend/parser.hpp"
#include "midend/checker.hpp"
#include <iostream>
// #include <llvm/IR/Module.h>
#include <memory>
using namespace std;
int main(int argc , char**argv){
    // if(argc<2){
    //     std::cerr<<("expect argv")<<endl;
    //     exit('a');
    // }
    Parser *p=new Parser("/home/qran/code/sy/testcase/src/1.cj");
    p->parserComp();
    p->comp->print();
    Checker c;
    p->comp->accept(c);
    // std::unique_ptr<LLVMContext> context=std::make_unique<LLVMContext>();
    // std::unique_ptr<Module> module=std::make_unique<Module>("hello",*context);
    // std::unique_ptr<IRBuilder<>> irbuilder=std::make_unique<IRBuilder<>>(*context);
    // Builder b(context.get(),irbuilder.get(),module.get(),std::move(c));
    delete (p);
    return 0;
}

#include "frontend/parser.hpp"
#include "midend/builder.hpp"
#include <iostream>
#include <llvm/IR/Module.h>

#include "llvm/IR/LLVMContext.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ManagedStatic.h"

#include <llvm/Support/raw_ostream.h>
#include <memory>
using namespace std;
using namespace llvm;
int main(int argc , char**argv){
    // if(argc<2){
    //     std::cerr<<("expect argv")<<endl;
    //     exit('a');
    // }
    Parser *p=new Parser("/home/qran/code/sy/testcase/src/2.cj");
    p->parserComp();
    p->comp->print();
    Checker c;
    p->comp->accept(c);
    std::unique_ptr<LLVMContext> context=std::make_unique<LLVMContext>();
    std::unique_ptr<Module> module=std::make_unique<Module>("hello",*context);
    std::unique_ptr<IRBuilder<>> irbuilder=std::make_unique<IRBuilder<>>(*context);
    Builder b(context.get(),irbuilder.get(),module.get(),std::move(c));
    b.genIR(c.moveModule());
    delete (p);





    module->print(llvm::errs(),nullptr);
    int ret;
    //module->dump();
    // 此处要初始化jit环境
    :: llvm::InitializeNativeTarget();
    ::llvm::InitializeNativeTargetAsmPrinter();
    LLVMLinkInMCJIT();
    {
        llvm::EngineBuilder builder(std::move(module));
        std::string error;
        std::unique_ptr<llvm::ExecutionEngine> ee(builder.setErrorStr(&error).setEngineKind(llvm::EngineKind::JIT)
                                                    .setOptLevel(llvm::CodeGenOptLevel::None).create());
      // 获取函数地址
        void *address = (void *)ee->getFunctionAddress("main");
        ret=((int(*)())address)();
        std::cout<<"return "<<ret<< std::endl;
    }
    llvm::llvm_shutdown();
    return 0;







    return 0;
}

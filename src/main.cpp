#include "backend/codegen.hpp"
#include "frontend/parser.hpp"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ManagedStatic.h"
#include <cstdlib>
#include <llvm/Support/CodeGen.h>
#include <memory>
using namespace llvm;

int main(int argc ,char**argv){
    if(argc<2){
        std::cerr<<("expect argv")<<endl;
        exit(EXIT_FAILURE);
    }
    Parser p(argv[1]);
    p.parserComp();

    std::unique_ptr<LLVMContext> context=std::make_unique<LLVMContext>();
    auto module=std::make_unique<Module>("hello",*context);
    auto builder=std::make_unique<IRBuilder<>>(*context);
    // module->getOrInsertGlobal("var",builder->getInt32Ty());
    // module->print(errs(),nullptr);
    gen::CodeGen irgen{module.get(),builder.get(),context.get()};

    p.comp->accept(irgen);
    module->print(errs(),nullptr);
    int ret;
    //module->dump();
    // 此处要初始化jit环境
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
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
}
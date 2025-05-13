#include "ANTLRInputStream.h"
#include "CangjieLexer.h"
#include "CangjieParser.h"
#include "CangjieVisitor.h"
#include "CommonTokenStream.h"
#include "ParserRuleContext.h"
#include "midend/builder.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <llvm/IR/Module.h>
#include "llvm/IRReader/IRReader.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ManagedStatic.h"

#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <optional>
#include <ostream>
#include "midend/AntlrChecker.hpp"
#include "midend/hir.hpp"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"

#include "llvm/Linker/Linker.h"



#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Error.h>
#include <llvm/Target/TargetOptions.h>

using namespace std;
using namespace llvm;
void emitObjectFile(std::unique_ptr<Module> &module, const std::string &filename) {
    // 初始化所有目标（必须）
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmPrinters();
    InitializeAllAsmParsers();

    // 获取当前平台的 target triple
    std::string targetTriple = sys::getDefaultTargetTriple();
    module->setTargetTriple(targetTriple);

    std::string error;
    const Target *target = TargetRegistry::lookupTarget(targetTriple, error);
    if (!target) {
        errs() << "Error: " << error << "\n";
        return;
    }

    TargetOptions opt;
    auto RM = std::optional<Reloc::Model>();
    TargetMachine *targetMachine = target->createTargetMachine(targetTriple, "generic", "", opt, RM);

    module->setDataLayout(targetMachine->createDataLayout());

    // 输出文件
    std::error_code EC;
    raw_fd_ostream dest(filename, EC, sys::fs::OF_None);
    if (EC) {
        errs() << "Could not open file: " << EC.message() << "\n";
        return;
    }

    legacy::PassManager pass;
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, CodeGenFileType::ObjectFile)) {
        errs() << "TargetMachine can't emit object file\n";
        return;
    }

    pass.run(*module);
    dest.flush();
    outs() << "Wrote object file to: " << filename << "\n";
}


void pass(llvm::Module&mod){
    PassBuilder pb;
    LoopAnalysisManager lam;
    FunctionAnalysisManager fam;
    CGSCCAnalysisManager cgam;
    ModuleAnalysisManager mam;
    
    pb.registerModuleAnalyses(mam);
    pb.registerCGSCCAnalyses(cgam);
    pb.registerFunctionAnalyses(fam);
    pb.registerLoopAnalyses(lam);
    pb.crossRegisterProxies(lam, fam, cgam, mam);
    // 使用 Clang -O2 默认优化 Pipeline
    ModulePassManager mpm = pb.buildPerModuleDefaultPipeline(OptimizationLevel::O1);

    mpm.run(mod, mam);
}
int main(int argc, const char* argv[]) {
    std::ifstream stream;
    stream.open(argv[1]);
    
    antlr4::ANTLRInputStream input(stream);
    CangjieLexer lexer(&input);
    // lexer.reset();
    antlr4::CommonTokenStream tokens(&lexer);
  	tokens.fill();
	// for (auto token : tokens.getTokens()) {
	// 	std::cout << token->toString() << std::endl;
	// }
  //   lexer.reset();

    CangjieParser parser(&tokens);    
    parser.setBuildParseTree(true);
    CangjieParser::TranslationUnitContext* tree = parser.translationUnit();
    // cout<<tree->toStringTree(1)<<endl;
    CangjieChecker * visitor= new CangjieChecker;
    visitor->visitTranslationUnit(tree);

  
    auto m=visitor->getmodule();

    std::unique_ptr<LLVMContext> context=std::make_unique<LLVMContext>();
    std::unique_ptr<Module> module=std::make_unique<Module>(argv[1],*context);
    std::unique_ptr<IRBuilder<>> irbuilder=std::make_unique<IRBuilder<>>(*context);
    // SMDiagnostic err;
    // std::unique_ptr<Module> runtimeModule = llvm::parseIRFile("/home/qran/code/sysy-llvm/lib/lib.ll", err, *context);
    // if (llvm::Linker::linkModules(*module, std::move(runtimeModule))) {
    //     errs() << "Link failed\n";
    //     return 1;
    // }
    Builder b(context.get(),irbuilder.get(),module.get(),std::move(visitor->getTypeMan()));
    b.genIR(m.get());





    // pass(*module);
    // module->print(llvm::errs(),nullptr);
    std::error_code EC;
    llvm::raw_fd_ostream file("output.ll", EC);
    module->print(file, nullptr);
    file.close();
    // module->print(out,nullptr);
    
    int ret;
    //module->dump();
    // 此处要初始化jit环境
    :: llvm::InitializeNativeTarget();
    ::llvm::InitializeNativeTargetAsmPrinter();
  
    // // 在初始化 JIT 之前
    // std::string error;
    // auto targetTriple = llvm::sys::getDefaultTargetTriple();
    // const llvm::Target *target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    // if (!target) {
    //     llvm::errs() << "Failed to lookup target: " << error << "\n";
    //     return 1;
    // }

    // auto CPU = "generic";
    // auto Features = "";
    // llvm::TargetOptions opt;
    // auto RM = std::optional<llvm::Reloc::Model>();
    // auto targetMachine = target->createTargetMachine(targetTriple, CPU, Features, opt, RM);

    // // 设置 module 的 triple 和 layout
    // module->setTargetTriple(targetTriple);
    // module->setDataLayout(targetMachine->createDataLayout());

    
    LLVMLinkInMCJIT();

    {
        llvm::EngineBuilder builder(std::move(module));
        std::string error;
        auto  ee(builder.setErrorStr(&error).setEngineKind(llvm::EngineKind::JIT)
                                                    .setOptLevel(llvm::CodeGenOptLevel::None).create());
        if (!ee) {
            std::cerr << "Failed to create ExecutionEngine: " << error << std::endl;
            return 1;
        }
      // 获取函数地址
        auto address = ee->getFunctionAddress("main");
        if (address == 0) {
            std::cerr << "Failed to get address of 'main'" << std::endl;
            return 1;
        }
        ret=((int64_t(*)())address)();
        std::cout<<"return "<<ret<< std::endl;
    }
    llvm::llvm_shutdown();

    return 0;
}
// int main(int argc , char**argv){
//     // if(argc<2){
//     //     std::cerr<<("expect argv")<<endl;
//     //     exit('a');
//     // }
//     Parser *p=new Parser("/home/qran/code/sy/testcase/src/2.cj");
//     p->parserComp();
//     p->comp->print();
//     Checker c;
//     p->comp->accept(c);

//     delete (p);





//     module->print(llvm::errs(),nullptr);
//     int ret;
//     //module->dump();
//     // 此处要初始化jit环境
//     :: llvm::InitializeNativeTarget();
//     ::llvm::InitializeNativeTargetAsmPrinter();
//     LLVMLinkInMCJIT();
//     {
//         llvm::EngineBuilder builder(std::move(module));
//         std::string error;
//         std::unique_ptr<llvm::ExecutionEngine> ee(builder.setErrorStr(&error).setEngineKind(llvm::EngineKind::JIT)
//                                                     .setOptLevel(llvm::CodeGenOptLevel::None).create());
//       // 获取函数地址
//         void *address = (void *)ee->getFunctionAddress("main");
//         ret=((int(*)())address)();
//         std::cout<<"return "<<ret<< std::endl;
//     }
//     llvm::llvm_shutdown();
//     return 0;







//     return 0;
// }

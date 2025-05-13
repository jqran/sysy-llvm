#include "ANTLRInputStream.h"
#include "CangjieLexer.h"
#include "CangjieParser.h"
#include "CommonTokenStream.h"
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

#include <getopt.h> // POSIX getopt
#include <cstring>

int main(int argc, char* argv[]) {
    std::string input_file;
    std::string output_file = "output.ll";
    int opt_level = 0;
    bool run = false;

    int opt;
    while ((opt = getopt(argc, argv, "o:O:rh")) != -1) {
        switch (opt) {
            case 'o':
                output_file = optarg;
                break;
            case 'O':
                opt_level = std::atoi(optarg);
                if (opt_level < 0 || opt_level > 3) {
                    std::cerr << "Invalid optimization level: " << optarg << "\n";
                    return 1;
                }
                break;
            case 'r':
                run = true;
                break;
            case 'h':
                std::cout << "Usage: " << argv[0] << " [options] <input file>\n"
                          << "Options:\n"
                          << "  -o <file>     Output LLVM IR file (default: output.ll)\n"
                          << "  -O[0-3]       Optimization level (default: 0)\n"
                          << "  -r            Run the compiled code via JIT\n"
                          << "  -h            Show this help message\n";
                return 0;
            default:
                std::cerr << "Use -h for help.\n";
                return 1;
        }
    }

    if (optind >= argc) {
        std::cerr << "Input file is required.\n";
        return 1;
    }

    input_file = argv[optind];

    std::ifstream stream(input_file);
    if (!stream) {
        std::cerr << "Cannot open input file: " << input_file << "\n";
        return 1;
    }

    // ====== 以下部分几乎不变（解析、生成 IR） ======

    antlr4::ANTLRInputStream input(stream);
    CangjieLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    tokens.fill();

    CangjieParser parser(&tokens);    
    parser.setBuildParseTree(true);
    auto* tree = parser.translationUnit();

    auto* visitor = new CangjieChecker;
    visitor->visitTranslationUnit(tree);

    auto m = visitor->getmodule();
    auto context = std::make_unique<LLVMContext>();
    auto module = std::make_unique<Module>(input_file, *context);
    auto irbuilder = std::make_unique<IRBuilder<>>(*context);

    Builder b(context.get(), irbuilder.get(), module.get(), std::move(visitor->getTypeMan()));
    b.genIR(m.get());

    // 根据优化等级选择是否优化
    if (opt_level > 0) {
        llvm::PassBuilder pb;
        llvm::LoopAnalysisManager lam;
        llvm::FunctionAnalysisManager fam;
        llvm::CGSCCAnalysisManager cgam;
        llvm::ModuleAnalysisManager mam;
        pb.registerModuleAnalyses(mam);
        pb.registerCGSCCAnalyses(cgam);
        pb.registerFunctionAnalyses(fam);
        pb.registerLoopAnalyses(lam);
        pb.crossRegisterProxies(lam, fam, cgam, mam);

        llvm::ModulePassManager mpm;
        switch (opt_level) {
            case 1: mpm = pb.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O1); break;
            case 2: mpm = pb.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2); break;
            case 3: mpm = pb.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3); break;
        }

        mpm.run(*module, mam);
    }

    // 输出 IR 到文件
    std::error_code EC;
    llvm::raw_fd_ostream file(output_file, EC);
    if (EC) {
        std::cerr << "Error writing to file: " << EC.message() << "\n";
        return 1;
    }
    module->print(file, nullptr);
    file.close();

    // 如果 -r 参数开启，则执行 IR
    if (run) {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        LLVMLinkInMCJIT();

        llvm::EngineBuilder builder(std::move(module));
        std::string error;
        auto ee = builder.setErrorStr(&error).setEngineKind(llvm::EngineKind::JIT)
                         .setOptLevel(llvm::CodeGenOptLevel::None).create();
        if (!ee) {
            std::cerr << "ExecutionEngine error: " << error << "\n";
            return 1;
        }

        auto address = ee->getFunctionAddress("main");
        if (address == 0) {
            std::cerr << "Failed to get address of 'main'\n";
            return 1;
        }
        int ret = ((int64_t(*)())address)();
        std::cout << "return " << ret << std::endl;
    }

    llvm::llvm_shutdown();
    return 0;
}

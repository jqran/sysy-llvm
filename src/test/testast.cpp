#include "ANTLRInputStream.h"
#include "CangjieLexer.h"
#include "CangjieParser.h"
#include "CangjieVisitor.h"
#include "CommonTokenStream.h"
#include "ParserRuleContext.h"
#include "midend/builder.hpp"
#include <cstdint>
#include <iostream>
#include <llvm/IR/Module.h>

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
static string const string_ir=R"(; ModuleID = './lib.c'
source_filename = "./lib.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.__va_list_tag = type { i32, i32, ptr, ptr }

@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@.str.1 = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@.str.2 = private unnamed_addr constant [3 x i8] c"%a\00", align 1
@.str.3 = private unnamed_addr constant [4 x i8] c"%d:\00", align 1
@.str.4 = private unnamed_addr constant [4 x i8] c" %d\00", align 1
@.str.6 = private unnamed_addr constant [4 x i8] c" %a\00", align 1
@stdout = external local_unnamed_addr global ptr, align 8

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @getint() local_unnamed_addr #0 {
  %1 = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %1) #5
  %2 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef nonnull @.str, ptr noundef nonnull %1)
  %3 = load i32, ptr %1, align 4, !tbaa !5
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %1) #5
  ret i32 %3
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nofree nounwind
declare noundef i32 @__isoc99_scanf(ptr nocapture noundef readonly, ...) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nofree nounwind uwtable
define dso_local range(i32 -128, 128) i32 @getch() local_unnamed_addr #0 {
  %1 = alloca i8, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %1) #5
  %2 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef nonnull @.str.1, ptr noundef nonnull %1)
  %3 = load i8, ptr %1, align 1, !tbaa !9
  %4 = sext i8 %3 to i32
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %1) #5
  ret i32 %4
}

; Function Attrs: nofree nounwind uwtable
define dso_local float @getfloat() local_unnamed_addr #0 {
  %1 = alloca float, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %1) #5
  %2 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef nonnull @.str.2, ptr noundef nonnull %1)
  %3 = load float, ptr %1, align 4, !tbaa !10
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %1) #5
  ret float %3
}

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @getarray(ptr noundef %0) local_unnamed_addr #0 {
  %2 = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %2) #5
  %3 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef nonnull @.str, ptr noundef nonnull %2)
  %4 = load i32, ptr %2, align 4, !tbaa !5
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %8, label %6

6:                                                ; preds = %8, %1
  %7 = phi i32 [ %4, %1 ], [ %13, %8 ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %2) #5
  ret i32 %7

8:                                                ; preds = %1, %8
  %9 = phi i64 [ %12, %8 ], [ 0, %1 ]
  %10 = getelementptr inbounds i32, ptr %0, i64 %9
  %11 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef nonnull @.str, ptr noundef %10)
  %12 = add nuw nsw i64 %9, 1
  %13 = load i32, ptr %2, align 4, !tbaa !5
  %14 = sext i32 %13 to i64
  %15 = icmp slt i64 %12, %14
  br i1 %15, label %8, label %6, !llvm.loop !12
}

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @getfarray(ptr noundef %0) local_unnamed_addr #0 {
  %2 = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %2) #5
  %3 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef nonnull @.str, ptr noundef nonnull %2)
  %4 = load i32, ptr %2, align 4, !tbaa !5
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %8, label %6

6:                                                ; preds = %8, %1
  %7 = phi i32 [ %4, %1 ], [ %13, %8 ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %2) #5
  ret i32 %7

8:                                                ; preds = %1, %8
  %9 = phi i64 [ %12, %8 ], [ 0, %1 ]
  %10 = getelementptr inbounds float, ptr %0, i64 %9
  %11 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef nonnull @.str.2, ptr noundef %10)
  %12 = add nuw nsw i64 %9, 1
  %13 = load i32, ptr %2, align 4, !tbaa !5
  %14 = sext i32 %13 to i64
  %15 = icmp slt i64 %12, %14
  br i1 %15, label %8, label %6, !llvm.loop !14
}

; Function Attrs: nofree nounwind uwtable
define dso_local void @putint(i32 noundef %0) local_unnamed_addr #0 {
  %2 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str, i32 noundef %0)
  ret void
}

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #2

; Function Attrs: nofree nounwind uwtable
define dso_local void @putch(i32 noundef %0) local_unnamed_addr #0 {
  %2 = tail call i32 @putchar(i32 %0)
  ret void
}

; Function Attrs: nofree nounwind uwtable
define dso_local void @putarray(i32 noundef %0, ptr nocapture noundef readonly %1) local_unnamed_addr #0 {
  %3 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.3, i32 noundef %0)
  %4 = icmp sgt i32 %0, 0
  br i1 %4, label %5, label %7

5:                                                ; preds = %2
  %6 = zext nneg i32 %0 to i64
  br label %9

7:                                                ; preds = %9, %2
  %8 = tail call i32 @putchar(i32 10)
  ret void

9:                                                ; preds = %5, %9
  %10 = phi i64 [ 0, %5 ], [ %14, %9 ]
  %11 = getelementptr inbounds i32, ptr %1, i64 %10
  %12 = load i32, ptr %11, align 4, !tbaa !5
  %13 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.4, i32 noundef %12)
  %14 = add nuw nsw i64 %10, 1
  %15 = icmp eq i64 %14, %6
  br i1 %15, label %7, label %9, !llvm.loop !15
}

; Function Attrs: nofree nounwind uwtable
define dso_local void @putfloat(float noundef %0) local_unnamed_addr #0 {
  %2 = fpext float %0 to double
  %3 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.2, double noundef %2)
  ret void
}

; Function Attrs: nofree nounwind uwtable
define dso_local void @putfarray(i32 noundef %0, ptr nocapture noundef readonly %1) local_unnamed_addr #0 {
  %3 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.3, i32 noundef %0)
  %4 = icmp sgt i32 %0, 0
  br i1 %4, label %5, label %7

5:                                                ; preds = %2
  %6 = zext nneg i32 %0 to i64
  br label %9

7:                                                ; preds = %9, %2
  %8 = tail call i32 @putchar(i32 10)
  ret void

9:                                                ; preds = %5, %9
  %10 = phi i64 [ 0, %5 ], [ %15, %9 ]
  %11 = getelementptr inbounds float, ptr %1, i64 %10
  %12 = load float, ptr %11, align 4, !tbaa !10
  %13 = fpext float %12 to double
  %14 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.6, double noundef %13)
  %15 = add nuw nsw i64 %10, 1
  %16 = icmp eq i64 %15, %6
  br i1 %16, label %7, label %9, !llvm.loop !16
}

; Function Attrs: nofree nounwind uwtable
define dso_local void @putf(ptr nocapture noundef readonly %0, ...) local_unnamed_addr #0 {
  %2 = alloca [1 x %struct.__va_list_tag], align 16
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %2) #5
  call void @llvm.va_start.p0(ptr nonnull %2)
  %3 = load ptr, ptr @stdout, align 8, !tbaa !17
  %4 = call i32 @vfprintf(ptr noundef %3, ptr noundef %0, ptr noundef nonnull %2) #5
  call void @llvm.va_end.p0(ptr nonnull %2)
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %2) #5
  ret void
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.va_start.p0(ptr) #3

; Function Attrs: nofree nounwind
declare noundef i32 @vfprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ptr noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.va_end.p0(ptr) #3

; Function Attrs: nofree nounwind
declare noundef i32 @putchar(i32 noundef) local_unnamed_addr #4

attributes #0 = { nofree nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nocallback nofree nosync nounwind willreturn }
attributes #4 = { nofree nounwind }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Debian clang version 19.1.7 (1+b1)\"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!7, !7, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"float", !7, i64 0}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.mustprogress"}
!14 = distinct !{!14, !13}
!15 = distinct !{!15, !13}
!16 = distinct !{!16, !13}
!17 = !{!18, !18, i64 0}
!18 = !{!"any pointer", !7, i64 0}
)";
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
	for (auto token : tokens.getTokens()) {
		std::cout << token->toString() << std::endl;
	}
    lexer.reset();

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
    Builder b(context.get(),irbuilder.get(),module.get(),std::move(visitor->getTypeMan()));
    b.genIR(m.get());





    // pass(*module);
    module->print(llvm::errs(),nullptr);
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

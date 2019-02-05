#include <iostream>
#include <fstream>
#include <exception>

#include <clang/Frontend/CompilerInstance.h>
#include <clang/FrontendTool/Utils.h>
//#include <clang/Driver/DriverDiagnostic.h>
#include <clang/Frontend/TextDiagnosticBuffer.h>
#include <clang/Frontend/FrontendDiagnostic.h>
//#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <llvm/Support/TargetSelect.h>
//#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
//#include <clang/Driver/Options.h>
//#include <llvm/Option/OptTable.h>
#include <clang/CodeGen/ModuleBuilder.h>
#include <clang/Parse/ParseAST.h>

#include <llvm/IR/LLVMContext.h>

#include <llvm/Support/Signals.h>
#include <llvm/Support/Process.h>
#include <llvm/Support/Timer.h>
//static llvm::cl::OptionCategory foundationOptinCategory("foundationTool options");
//static std::unique_ptr<llvm::opt::OptTable> Options(clang::driver::createDriverOptTable());
//static llvm::cl::opt<bool>
//ASTDump("ast-dump", llvm::cl::desc(Options->getOptionHelpText(clang::driver::options::OPT_ast_dump)),
//        llvm::cl::cat(foundationOptinCategory));
//static llvm::cl::opt<bool>
//ASTList("ast-list", llvm::cl::desc(Options->getOptionHelpText(clang::driver::options::OPT_ast_list)),
//        llvm::cl::cat(foundationOptinCategory));
//static llvm::cl::opt<bool>
//ASTPrint("ast-print",
//         llvm::cl::desc(Options->getOptionHelpText(clang::driver::options::OPT_ast_print)),
//         llvm::cl::cat(foundationOptinCategory));
//static llvm::cl::opt<std::string> ASTDumpFilter(
//                                          "ast-dump-filter",
//                                          llvm::cl::desc(Options->getOptionHelpText(clang::driver::options::OPT_ast_dump_filter)),
//                                          llvm::cl::cat(foundationOptinCategory));
//static llvm::cl::opt<bool>
//Analyze("analyze", llvm::cl::desc(Options->getOptionHelpText(clang::driver::options::OPT_analyze)),
//        llvm::cl::cat(foundationOptinCategory));


#include <consumer.h>


// Emitting constructors for global objects involves looking
// at the source file name. This makes sure that we don't crash
// if the source file is a memory buffer.
const char TestProgram[] =
"class EmitCXXGlobalInitFunc    "
"{                              "
"public:                        "
"   EmitCXXGlobalInitFunc() {}  "
"};                             "
"EmitCXXGlobalInitFunc test;    ";


static void LLVMErrorHandler(void *UserData, const std::string &Message,
                             bool GenCrashDiag) {
    clang::DiagnosticsEngine &Diags = *static_cast<clang::DiagnosticsEngine*>(UserData);
    
    Diags.Report(clang::diag::err_fe_error_backend) << Message;
    
    // Run the interrupt handlers to make sure any special cleanups get done, in
    // particular that we remove files registered with RemoveFileOnSignal.
    llvm::sys::RunInterruptHandlers();
    
    // We cannot recover from llvm errors.  When reporting a fatal error, exit
    // with status 70 to generate crash diagnostics.  For BSD systems this is
    // defined as an internal software error.  Otherwise, exit with status 1.
    exit(GenCrashDiag ? 70 : 1);
}

/*
 * Get File Name from a Path with or without extension
 */
std::string getFileName(std::string filePath, bool withoutExtension = true, char seperator = '/')
{
    // Get last dot position
    std::size_t dotPos = filePath.rfind('.');
    std::size_t sepPos = filePath.rfind(seperator);
    
    if(sepPos != std::string::npos)
    {
        return filePath.substr(sepPos + 1, ((dotPos != std::string::npos && withoutExtension) ?  dotPos : filePath.size() ) - sepPos - 1);
    }
    return "";
}

std::string removeExt(std::string filePath)
{
    // Get last dot position
    std::size_t dotPos = filePath.rfind('.');
    
    if(dotPos != std::string::npos)
    {
        return filePath.substr(0, dotPos);
    }
    return filePath;
}



int main(int argc, char *argv[])
{
    // parse command line
    try
    {
    }
    catch (std::exception &e)
    {
        
    }
    catch (...)
    {
    }
    
    std::cout<<"Hello, testTool starts!\n";
    
    std::string inputFile;
    std::string outputDir;
    
    for (int i = 0; i < argc; ++i)
    {
        std::cout << argv[i] << "\n";
        if (i==1)
        {
            // dir
            outputDir = std::string(argv[i]);
//            inputFile = getFileName(inputFile, true);
        }
        else if (i==2)
        {
            // dir
            inputFile = std::string(argv[i]);
            inputFile = removeExt(inputFile);
            
            std::string outputFile = outputDir + "/" + inputFile + "_autogen.cpp";
            std::cout << "testTool "<<  i << ": write " << outputFile << "\n";
            std::ofstream myfile;
            myfile.open(outputFile);
            myfile << "static void "+ getFileName(inputFile, true) + "_autogen() {}";
            myfile.close();
        }
        else
        {
            std::cout << "testTool arg["<<  i << "] = " << argv[i] << "\n";
        }
    }
    
    std::unique_ptr<clang::CompilerInstance> Clang(new clang::CompilerInstance());
    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(new clang::DiagnosticIDs());
    
    llvm::SmallVector<const char *, 256> _argv;
    llvm::SpecificBumpPtrAllocator<char> ArgAllocator;
    std::error_code EC = llvm::sys::Process::GetArgumentVector(
                                                               _argv, llvm::makeArrayRef(argv, argc), ArgAllocator);
    if (EC) {
        llvm::errs() << "error: couldn't get arguments: " << EC.message() << '\n';
        return 1;
    }
    
    // Register the support for object-file-wrapped Clang modules.
//    auto PCHOps = Clang->getPCHContainerOperations();
//    PCHOps->registerWriter(llvm::make_unique<ObjectFilePCHContainerWriter>());
//    PCHOps->registerReader(llvm::make_unique<ObjectFilePCHContainerReader>());
    
    // Initialize targets first, so that --version shows registered targets.
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmPrinters();
    llvm::InitializeAllAsmParsers();
    
    llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts = new clang::DiagnosticOptions();
    clang::TextDiagnosticBuffer *DiagsBuffer = new clang::TextDiagnosticBuffer;
    clang::DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagsBuffer);
    bool Success = clang::CompilerInvocation::CreateFromArgs(
                                                      Clang->getInvocation(), _argv.begin()+3, _argv.end(), Diags);
    
    // Infer the builtin include path if unspecified.
    if (Clang->getHeaderSearchOpts().UseBuiltinIncludes &&
       Clang->getHeaderSearchOpts().ResourceDir.empty())
    {
    }
//    Clang->getHeaderSearchOpts().ResourceDir =
//    clang::CompilerInvocation::GetResourcesPath(_argv.begin(), MainAddr);
    
    // Create the actual diagnostics engine.
    Clang->createDiagnostics();
    if (!Clang->hasDiagnostics())
        return 1;
    
    Clang->getLangOpts().CPlusPlus = 1;
    Clang->getLangOpts().CPlusPlus11 = 1;
    
    
//    // Set an error handler, so that any LLVM backend diagnostics go through our
//    // error handler.
//    llvm::install_fatal_error_handler(LLVMErrorHandler,
//                                      static_cast<void*>(&Clang->getDiagnostics()));
//
//    DiagsBuffer->FlushDiagnostics(Clang->getDiagnostics());
//    if (!Success)
//        return 1;
    
    Clang->getTargetOpts().Triple = llvm::Triple::normalize(
                                                              llvm::sys::getProcessTriple());
    Clang->setTarget(clang::TargetInfo::CreateTargetInfo(
                                                           Clang->getDiagnostics(),
                                                           std::make_shared<clang::TargetOptions>(                                                                                         Clang->getTargetOpts())));
    
    // create FileManager
    Clang->createFileManager();
    clang::FileManager& fileMgr = Clang->getFileManager();
    
    // create SourceManager
    Clang->createSourceManager( fileMgr );
    clang::SourceManager& SourceMgr = Clang->getSourceManager();
    
    const clang::FileEntry * file = fileMgr.getFile( std::string(*(_argv.begin()+2)) );
    SourceMgr.setMainFileID(  SourceMgr.createFileID( file, clang::SourceLocation(), clang::SrcMgr::C_User) );
    
    
    // get a reference to HeaderSearchOptions and add the include paths
    clang::HeaderSearchOptions& searchOpts = Clang->getHeaderSearchOpts();
    searchOpts.UseLibcxx = true;
    
    
    // Execute the frontend actions.
    Clang->createPreprocessor(clang::TU_Prefix);
    
    Clang->getDiagnosticClient().BeginSourceFile(Clang->getLangOpts(), &Clang->getPreprocessor() );
    
    Clang->createASTContext();
    
//    for ( auto path : searchOpts )
//    {
//        searchOpts.AddPath( path, clang::frontend::Quoted, false, false );
//    }
    
    
    // Set an error handler, so that any LLVM backend diagnostics go through our
    // error handler.
    llvm::install_fatal_error_handler(LLVMErrorHandler,
                                      static_cast<void*>(&Clang->getDiagnostics()));
    
//    DiagsBuffer->FlushDiagnostics(Clang->getDiagnostics());
//    if (!Success)
//        return 1;
    
    // create UTBuilder consumer
    MyConsumer consumer( &Clang->getASTContext(), std::string(*(_argv.begin()+2)) );
    
//    clang::tooling::runToolOnCode(new Action, argv[1]);
    
    // Parse the AST and execute all the visitors
    clang::ParseAST(Clang->getPreprocessor(), &consumer, Clang->getASTContext(), true);
    
    Clang->getDiagnosticClient().EndSourceFile();
    
    
    Success = clang::ExecuteCompilerInvocation(Clang.get());
    
    // If any timers were active but haven't been destroyed yet, print their
    // results now.  This happens in -disable-free mode.
    llvm::TimerGroup::printAll(llvm::errs());
    
    // Our error handler depends on the Diagnostics object, which we're
    // potentially about to delete. Uninstall the handler now so that any
    // later errors use the default handling behavior instead.
    llvm::remove_fatal_error_handler();
    
    
    // When running with -disable-free, don't do any destruction or shutdown.
    if (Clang->getFrontendOpts().DisableFree) {
        BuryPointer(std::move(Clang));
        return !Success;
    }
    
    if (!Success)
    {
//        Clang->getDiagnostics().printCommands(llvm::errs());
//        std::cout << llvm::errs().error()
        llvm::outs().flush();
        return 1;
    }
    
    llvm::LLVMContext Context;
    
    Clang->setASTConsumer(std::unique_ptr<clang::ASTConsumer>(
                                                         CreateLLVMCodeGen(
                                                                           Clang->getDiagnostics(),
                                                                           "EmitCXXGlobalInitFuncTest",
                                                                           Clang->getHeaderSearchOpts(),
                                                                           Clang->getPreprocessorOpts(),
                                                                           Clang->getCodeGenOpts(),
                                                                           Context)));
    
    Clang->createSema(clang::TU_Prefix, nullptr);
    
    clang::SourceManager &sm = Clang->getSourceManager();
    sm.setMainFileID(sm.createFileID(
                                     llvm::MemoryBuffer::getMemBuffer(TestProgram), clang::SrcMgr::C_User));
    
    clang::ParseAST(Clang->getSema(), false, false);

    
    
    // If any timers were active but haven't been destroyed yet, print their
    // results now.  This happens in -disable-free mode.
    llvm::TimerGroup::printAll(llvm::errs());
    
    // Our error handler depends on the Diagnostics object, which we're
    // potentially about to delete. Uninstall the handler now so that any
    // later errors use the default handling behavior instead.
    llvm::remove_fatal_error_handler();
    
    
    std::cout<<"Hello, testTool ends!\n";
    
    return !Success;
}

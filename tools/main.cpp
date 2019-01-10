#include <iostream>
#include <fstream>
#include <exception>


#include <llvm/Support/TargetSelect.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Driver/Options.h>
#include <llvm/Option/OptTable.h>

static llvm::cl::OptionCategory ClangCheckCategory("foundationTool options");
static std::unique_ptr<llvm::opt::OptTable> Options(clang::driver::createDriverOptTable());
static llvm::cl::opt<bool>
ASTDump("ast-dump", llvm::cl::desc(Options->getOptionHelpText(clang::driver::options::OPT_ast_dump)),
        llvm::cl::cat(ClangCheckCategory));
static llvm::cl::opt<bool>
ASTList("ast-list", llvm::cl::desc(Options->getOptionHelpText(clang::driver::options::OPT_ast_list)),
        llvm::cl::cat(ClangCheckCategory));
static llvm::cl::opt<bool>
ASTPrint("ast-print",
         llvm::cl::desc(Options->getOptionHelpText(clang::driver::options::OPT_ast_print)),
         llvm::cl::cat(ClangCheckCategory));
static llvm::cl::opt<std::string> ASTDumpFilter(
                                          "ast-dump-filter",
                                          llvm::cl::desc(Options->getOptionHelpText(clang::driver::options::OPT_ast_dump_filter)),
                                          llvm::cl::cat(ClangCheckCategory));
static llvm::cl::opt<bool>
Analyze("analyze", llvm::cl::desc(Options->getOptionHelpText(clang::driver::options::OPT_analyze)),
        llvm::cl::cat(ClangCheckCategory));

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
        else if (i>=2)
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
    }
    
    // Initialize targets for clang module support.
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmPrinters();
    llvm::InitializeAllAsmParsers();
    
    clang::tooling::CommonOptionsParser OptionsParser(argc, (const char**)argv, ClangCheckCategory);
    clang::tooling::ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
    
    std::cout<<"Hello, testTool ends!\n";
    
    return 0;
}

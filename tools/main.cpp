#include <iostream>
#include <fstream>
#include <exception>


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
    
    
    std::cout<<"Hello, testTool ends!\n";
    
    return 0;
}

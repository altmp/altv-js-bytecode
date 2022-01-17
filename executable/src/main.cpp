#include "cli.h"
#include "logger.h"
#include "alt-config/alt-config.h"

#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
    CLI::Parser parser(argc, argv);
    if(parser.IsEmpty())
    {
        Logger::Instance().LogError("Failed to parse commandline arguments");
        return 1;
    }

    if(parser.HasArgument("help"))
    {
        Logger::Instance().Log("Available arguments:");
        Logger::Instance().Log("  --help: Show this help");
        Logger::Instance().Log("  --input: Specify the resource directory");
        Logger::Instance().Log("  --output: Specify the output directory");
        return 0;
    }

    fs::path resourceDir = parser.HasArgument("input") ? parser.GetArgument("input") : "";
    if(resourceDir.empty() || !fs::exists(resourceDir) || !fs::is_directory(resourceDir))
    {
        Logger::Instance().LogError("Invalid input directory specified");
        return 1;
    }
    std::string outputDir = parser.HasArgument("output") ? parser.GetArgument("output") : "";
    if(outputDir.empty() || !fs::exists(outputDir) || !fs::is_directory(outputDir))
    {
        Logger::Instance().LogError("Invalid output directory specified");
        return 1;
    }

    return 0;
}

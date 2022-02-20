#include "cli.h"
#include "logger.h"
#include "package.h"

#include "alt-config/alt-config.h"
#include "v8.h"
#include "libplatform/libplatform.h"

#include <filesystem>

namespace fs = std::filesystem;

static alt::config::Node ReadResourceCfg(Package& package, const fs::path& path)
{
    // Check if the input directory has a resource.cfg
    if(!fs::exists(path))
    {
        Logger::Instance().LogError("Input directory does not contain a resource.cfg");
        return alt::config::Node();
    }

    // Read the resource.cfg
    std::string resourceCfgSource;
    resourceCfgSource.resize(fs::file_size(path));
    if(!package.ReadFile(path.string(), resourceCfgSource.data(), resourceCfgSource.size()))
    {
        Logger::Instance().LogError("Failed to read resource.cfg");
        return alt::config::Node();
    }

    // Parse the resource.cfg
    try
    {
        alt::config::Parser configParser(resourceCfgSource.c_str(), resourceCfgSource.size());
        alt::config::Node result = configParser.Parse();
        return result;
    }
    catch(alt::config::Error& e)
    {
        Logger::Instance().LogError("Failed to parse resource.cfg: " + std::string(e.what()));
        return alt::config::Node();
    }
}

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
    fs::path outputDir = parser.HasArgument("output") ? parser.GetArgument("output") : "";
    if(outputDir.empty() || !fs::exists(outputDir) || !fs::is_directory(outputDir))
    {
        Logger::Instance().LogError("Invalid output directory specified");
        return 1;
    }

    // Set up the file package, to read and write files
    Package package(outputDir, resourceDir);

    // Read the resource.cfg
    alt::config::Node resourceCfg = ReadResourceCfg(package, resourceDir / "resource.cfg");
    if(resourceCfg.IsNone()) return 1;

    // Get the resource client-main file
    alt::config::Node clientMain = resourceCfg["client-main"];
    if(!clientMain.IsScalar())
    {
        Logger::Instance().LogError("Failed to find suitable client-main in resource.cfg");
        return 1;
    }
    std::string clientMainFile = clientMain.ToString();
    fs::path clientMainPath = resourceDir / clientMainFile;
    if(!fs::exists(clientMainPath))
    {
        Logger::Instance().LogError("Failed to find client-main file: " + clientMainFile);
        return 1;
    }

    // Set up v8
    v8::V8::SetFlagsFromString("--harmony-import-assertions --short-builtin-calls --no-lazy --no-flush-bytecode");
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    v8::Isolate::CreateParams params;
    params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate = v8::Isolate::New(params);

    // Set up the compiler
    BytecodeCompiler::Compiler compiler(isolate, &package, &Logger::Instance());
    compiler.SetIgnoredModules({ "alt", "alt-client", "natives", "alt-worker", "alt-shared" });

    // Compile the main file
    if(!compiler.CompileModule(clientMainPath.string())) return 1;

    return 0;
}

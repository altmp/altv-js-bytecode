#include "libplatform/libplatform.h"
#include "alt-config/alt-config.h"
#include "toml/TomlConfig.h"

#include "package.h"
#include "logger.h"
#include "cli.h"
#include "v8.h"

namespace fs = std::filesystem;

static void EvaluateArgs(CLI::Parser& parser) {
    Logger logger = Logger::Instance();

    if(parser.IsEmpty())
    {
        logger.LogError("Failed to parse commandline arguments");
        exit(1);
    }

    if(parser.HasArgument("help"))
    {
        logger.Log("Available arguments:");
        logger.Log("  --help: Show this help");
        logger.Log("  --input: Specify the resource directory");
        logger.Log("  --output: Specify the output directory");
        logger.Log("  --debug: Enables debug info");
        exit(0);
    }

    logger.ToggleDebugLogs(parser.HasArgument("debug"));
}

static fs::path GetResourceDir(CLI::Parser& parser) {
    fs::path resourceDir = parser.GetArgument("input");

    if(resourceDir.empty() || !fs::exists(resourceDir) || !fs::is_directory(resourceDir))
    {
        Logger::Instance().LogError("Invalid input directory specified");
        exit(1);
    }

    return resourceDir;
}

static fs::path GetOutputDir(CLI::Parser& parser) {
    fs::path outputDir = parser.GetArgument("output");

    if(outputDir.empty() || !fs::exists(outputDir) || !fs::is_directory(outputDir))
    {
        Logger::Instance().LogError("Invalid output directory specified");
        exit(1);
    }

    return outputDir;
}

static Config::Value::ValuePtr GetResourceConfig(Package& package, const fs::path& path)
{
    // Check if the input directory has a resource.toml
    if(!fs::exists(path.string()))
    {
        Logger::Instance().LogError("Input directory does not contain a resource.toml");
        exit(1);
    }

    // Read the resource.toml
    std::string resourceCfgSource;
    resourceCfgSource.resize(package.GetFileSize(path.string()));

    if(!package.ReadFile(path.string(), resourceCfgSource.data(), resourceCfgSource.size()))
    {
        Logger::Instance().LogError("Failed to read resource.toml");
        exit(1);
    }

    // Parse the resource.toml
    std::string error = "Failed to parse resource.toml";
    return TomlConfig::Parse(resourceCfgSource, error);
}

static fs::path GetClientMain(Package& package, const fs::path& resourceDir) {
    // Read the resource.cfg
    Config::Value::ValuePtr config = GetResourceConfig(package, resourceDir / "resource.toml");
    std::string clientMainFile = config->Get("client-main")->AsString();

    // Get the resource client-main file
    if(clientMainFile.empty())
    {
        Logger::Instance().LogError("Failed to find client-main in resource.toml");
        exit(1);
    }

    // no need to check is the clientMainPath exists as its checked in the compiler
    return resourceDir / clientMainFile;
}

int main(int argc, char* argv[])
{
    CLI::Parser parser(argc, argv);

    EvaluateArgs(parser);
    fs::path resourceDir = GetResourceDir(parser);
    fs::path outputDir = GetOutputDir(parser);

    // Set up v8
    v8::V8::SetFlagsFromString("--harmony-import-assertions --short-builtin-calls --no-lazy --no-flush-bytecode --no-enable-lazy-source-positions");
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    // V8 isolate stuff
    v8::Isolate::CreateParams params;
    params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();

    v8::Isolate* isolate = v8::Isolate::New(params);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);

    // Set up the file package, to read and write files
    Package package(outputDir, resourceDir);
    fs::path clientMain = GetClientMain(package, resourceDir);

    // Set up the compiler
    BytecodeCompiler::Compiler compiler(isolate, &package, &Logger::Instance());
    compiler.SetIgnoredModules({ "alt", "alt-client", "natives", "alt-worker", "alt-shared" });

    // Compile the main file
    return compiler.CompileModule(clientMain.string()) ? 0 : 1;
}

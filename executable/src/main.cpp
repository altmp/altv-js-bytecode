#include "libplatform/libplatform.h"
#include "alt-config/alt-config.h"
#include "toml/TomlConfig.h"

#include "package.h"
#include "logger.h"
#include "cli.h"
#include "v8.h"

namespace fs = std::filesystem;

static void EvaluateArgs(CLI::Parser& parser)
{
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
        logger.Log("  --input: Specify the input file");
        logger.Log("  --output: Specify the output directory");
        logger.Log("  --debug: Enables debug info");
        exit(0);
    }

    logger.ToggleDebugLogs(parser.HasArgument("debug"));
}

static fs::path GetInputFile(CLI::Parser& parser)
{
    fs::path inputFile = parser.GetArgument("input");

    if(inputFile.empty() || !fs::exists(inputFile) || !fs::is_regular_file(inputFile))
    {
        Logger::Instance().LogError("Invalid input file specified");
        exit(1);
    }

    return inputFile;
}

static fs::path GetOutputDir(CLI::Parser& parser)
{
    fs::path outputDir = parser.GetArgument("output");

    if(outputDir.empty() || (fs::exists(outputDir) && !fs::is_directory(outputDir)))
    {
        Logger::Instance().LogError("Invalid output directory specified");
        exit(1);
    }

    if(!fs::exists(outputDir)) fs::create_directory(outputDir);
    return outputDir;
}

int main(int argc, char* argv[])
{
    CLI::Parser parser(argc, argv);

    EvaluateArgs(parser);
    fs::path inputFile = GetInputFile(parser);
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
    Package package(outputDir);

    // Set up the compiler
    BytecodeCompiler::Compiler compiler(isolate, &package, &Logger::Instance());
    compiler.SetIgnoredModules({ "alt", "alt-server", "alt-shared" });

    // Compile the main file
    return compiler.CompileModule(inputFile.string()) ? 0 : 1;
}

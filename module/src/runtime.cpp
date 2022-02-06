#include "runtime.h"
#include "Log.h"
#include "compiler.h"
#include "package.h"
#include "logger.h"

JSBytecodeRuntime::JSBytecodeRuntime()
{
    v8::V8::SetFlagsFromString("--harmony-import-assertions --short-builtin-calls --no-lazy --no-flush-bytecode");
    platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();

    isolate = v8::Isolate::New(create_params);
}

void JSBytecodeRuntime::ProcessClientFile(alt::IResource* resource, alt::IPackage* package)
{
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);

    // Set up compiler
    alt::IPackage* resourcePackage = resource->GetPackage();
    Package compilerPackage(package, resourcePackage, resource);
    Logger compilerLogger;
    BytecodeCompiler::Compiler compiler(isolate, &compilerPackage, &compilerLogger);

    static std::vector<std::string> ignoredModules = { "alt", "alt-client", "natives", "alt-worker" };
    compiler.SetIgnoredModules(ignoredModules);

    // Compile client main file
    bool result = compiler.CompileModule(resource->GetClientMain());
    if(!result) return;

    // Compile the extra files
    alt::config::Node config = resource->GetConfig();
    alt::config::Node& node = config["extra-compile-files"];
    if(node && node.IsList())
    {
        alt::config::Node::List& list = node.ToList();
        std::vector<std::string> extraFilePatterns;
        extraFilePatterns.reserve(list.size());
        for(alt::config::Node& item : list)
        {
            if(item.IsScalar()) extraFilePatterns.push_back(item.ToString());
        }

        std::set<std::string> files = resource->GetMatchedFiles(extraFilePatterns);
        for(const std::string& file : files)
        {
            bool result = compiler.CompileModule(file, false);
            if(!result) return;
        }
    }

    // Write all other files normally
    const std::vector<std::string>& clientFiles = resource->GetClientFiles();
    const std::vector<std::string>& compiledFiles = compiler.GetCompiledFiles();
    for(const std::string& clientFile : clientFiles)
    {
        // Check if the file is compiled, then we don't want to overwrite it
        if(std::find(compiledFiles.begin(), compiledFiles.end(), clientFile) != compiledFiles.end()) continue;

        // Open the file from the resource package and read the content
        alt::IPackage::File* file = resourcePackage->OpenFile(clientFile);
        size_t fileSize = resourcePackage->GetFileSize(file);
        std::string buffer;
        buffer.resize(fileSize);
        resourcePackage->ReadFile(file, buffer.data(), buffer.size());
        resourcePackage->CloseFile(file);

        // Write the file content into the client package
        alt::IPackage::File* clientPkgFile = package->OpenFile(clientFile);
        package->WriteFile(clientPkgFile, buffer.data(), buffer.size());
        package->CloseFile(clientPkgFile);
    }
}

bool JSBytecodeRuntime::GetProcessClientType(std::string& clientType)
{
    clientType = "jsb";
    return true;
}

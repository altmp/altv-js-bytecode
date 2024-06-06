#include "runtime.h"
#include "Log.h"
#include "compiler.h"
#include "CScriptRuntimeInfo.h"
#include "package.h"
#include "logger.h"

void JSBytecodeRuntime::ProcessClientFile(alt::IResource* resource, alt::IPackage* package)
{
    v8::Isolate* isolate = CScriptRuntimeInfo::Instance().GetIsolate();
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);

    // Set up compiler
    alt::IPackage* resourcePackage = resource->GetPackage();
    Package compilerPackage(package, resourcePackage, resource);
    Logger compilerLogger;
    BytecodeCompiler::Compiler compiler(isolate, &compilerPackage, &compilerLogger);

    Config::Value::ValuePtr config = resource->GetConfig();
    // Get ignored files
    std::vector<std::string> ignoredModules = { "alt", "alt-client", "natives", "alt-worker", "alt-shared" };
    Config::Value::ValuePtr ignoredFiles = config->Get("ignored-files");
    Config::Value::Bool verboseLogging = config->Get("verbose")->AsBool(false);

    if(ignoredFiles->IsList())
    {
        Config::Value::List list = ignoredFiles->As<Config::Value::List>();
        ignoredModules.reserve(ignoredModules.size() + list.size());
        for(auto& item : list)
        {
            if(item->IsString()) ignoredModules.push_back(item->As<std::string>());
        }
    }
    compiler.SetIgnoredModules(ignoredModules);

    // Compile client main file
    bool result = compiler.CompileModule(resource->GetClientMain(), true, verboseLogging);
    if(!result) return;

    // Compile the extra files
    Config::Value::ValuePtr extraCompileFiles = config->Get("extra-compile-files");
    if(extraCompileFiles->IsList())
    {
        Config::Value::List list = extraCompileFiles->As<Config::Value::List>();
        std::vector<std::string> extraFilePatterns;
        extraFilePatterns.reserve(list.size());
        for(auto& item : list)
        {
            if(item->IsString()) extraFilePatterns.push_back(item->As<std::string>());
        }

        std::set<std::string> files = resource->GetMatchedFiles(extraFilePatterns);
        for(const std::string& file : files)
        {
            bool result = compiler.CompileModule(file, false, verboseLogging);
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

    compilerLogger.Log("Converted " + std::to_string(compiledFiles.size()) + " script files to bytecode");
}

bool JSBytecodeRuntime::GetProcessClientType(std::string& clientType)
{
    clientType = "jsb";
    return true;
}

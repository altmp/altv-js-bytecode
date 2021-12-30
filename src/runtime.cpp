#include "runtime.h"
#include "Log.h"
#include "compiler.h"

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

    // Keep track of all the files we compiled to bytecode
    std::vector<std::string> compiledFiles;

    // Read client main file
    alt::IPackage* resourcePackage = resource->GetPackage();
    std::string fileName = resource->GetClientMain();
    alt::IPackage::File* file = resourcePackage->OpenFile(fileName);
    size_t fileSize = resourcePackage->GetFileSize(file);
    std::string buffer;
    buffer.resize(fileSize);
    resourcePackage->ReadFile(file, buffer.data(), buffer.size());
    resourcePackage->CloseFile(file);
    Compiler::CompileModuleToBytecode(isolate, resource, package, fileName, buffer, compiledFiles);

    // Read the extra files
    std::vector<std::string> extraFilePatterns = resource->GetConfigStringList("extra-compile-files");
    std::set<std::string> files = resource->GetMatchedFiles(extraFilePatterns);
    for(const std::string& file : files)
    {
        alt::IPackage::PathInfo pathInfo = alt::ICore::Instance().Resolve(resource, file, "");
        if(!pathInfo.pkg) continue;
        alt::IPackage::File* pkgFile = pathInfo.pkg->OpenFile(file);
        if(!pkgFile) continue;

        size_t fileSize = pathInfo.pkg->GetFileSize(pkgFile);
        std::string buffer;
        buffer.resize(fileSize);
        pathInfo.pkg->ReadFile(pkgFile, buffer.data(), buffer.size());
        pathInfo.pkg->CloseFile(pkgFile);

        Compiler::CompileModuleToBytecode(isolate, resource, resourcePackage, file, buffer, compiledFiles);
    }

    // Write all other files normally
    const std::vector<std::string>& clientFiles = resource->GetClientFiles();
    for(const std::string& clientFile : clientFiles)
    {
        if(std::find(compiledFiles.begin(), compiledFiles.end(), clientFile) != compiledFiles.end()) continue;
        alt::IPackage::File* file = resourcePackage->OpenFile(clientFile);
        size_t fileSize = resourcePackage->GetFileSize(file);
        std::string buffer;
        buffer.resize(fileSize);
        resourcePackage->ReadFile(file, buffer.data(), buffer.size());
        resourcePackage->CloseFile(file);

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

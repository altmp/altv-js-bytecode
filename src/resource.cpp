#include "resource.h"
#include "runtime.h"
#include "Bytecode.h"

#include <sstream>
#include <fstream>
#include <filesystem>

bool JSBytecodeResource::WriteClientFile(alt::IPackage* package, const std::string& fileName, void* buffer, uint64_t size)
{
    std::filesystem::path path(fileName);
    if(path.extension() != ".js") return false;

    JSBytecodeRuntime& runtime = JSBytecodeRuntime::Instance();
    v8::ScriptOrigin origin(runtime.GetIsolate(),
                            v8::String::NewFromUtf8(runtime.GetIsolate(), fileName.c_str()).ToLocalChecked(),
                            0,
                            0,
                            false,
                            -1,
                            v8::Local<v8::Value>(),
                            false,
                            false,
                            true,
                            v8::Local<v8::PrimitiveArray>());
    alt::String sourceCode(reinterpret_cast<char*>(buffer), size);
    v8::ScriptCompiler::Source source(v8::String::NewFromUtf8(isolate, sourceCode.CStr()).ToLocalChecked(), origin);
    v8::MaybeLocal<v8::Module> maybeModule = v8::ScriptCompiler::CompileModule(isolate, &source);
    if(maybeModule.IsEmpty())
    {
        alt::ICore::Instance().LogError("[V8 Bytecode] Failed to compile module: " + fileName);
        return false;
    }
    v8::Local<v8::Module> module = maybeModule.ToLocalChecked();
    v8::ScriptCompiler::CachedData* cache = v8::ScriptCompiler::CreateCodeCache(module->GetUnboundModuleScript());
    if(cache == nullptr)
    {
        alt::ICore::Instance().LogError("[V8 Bytecode] Failed to create code cache: " + fileName);
        return false;
    }

    Bytecode bytecode(cache->length, cache->data);
    // Bytecode struct copies the data, so we delete the cached data here
    delete cache;
    if(!bytecode.IsValid())
    {
        alt::ICore::Instance().LogError("[V8 Bytecode] Failed to create bytecode: " + fileName);
        return false;
    }
    alt::IPackage::File* file = package->OpenFile(fileName);
    if(!file)
    {
        alt::ICore::Instance().LogError("[V8 Bytecode] Failed to open file: " + fileName);
        return false;
    }

    // Copy the bytecode data to a new buffer
    auto result = bytecode.ToBuffer();
    package->WriteFile(file, result.first, result.second);
    delete result.first;

    return true;
}

bool JSBytecodeResource::Stop()
{
    context.Reset();
    return true;
}

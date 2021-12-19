#include "resource.h"
#include "runtime.h"
#include "Bytecode.h"

#include <sstream>
#include <fstream>
#include <filesystem>

bool JSBytecodeResource::WriteClientFile(alt::IPackage* package, const std::string& fileName, void* buffer, uint64_t size)
{
    alt::ICore::Instance().LogWarning(__FUNCTION__);
    std::filesystem::path path(fileName);
    if(path.extension() != ".js") return false;
    alt::ICore::Instance().LogWarning(fileName);

    JSBytecodeRuntime& runtime = JSBytecodeRuntime::Instance();
    v8::Isolate* isolate = runtime.GetIsolate();
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);

    v8::ScriptOrigin origin(isolate,
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

    alt::IPackage::File* file = package->OpenFile(fileName);
    if(!file)
    {
        alt::ICore::Instance().LogError("[V8 Bytecode] Failed to open file: " + fileName);
        return false;
    }

    package->WriteFile(file, (void*)cache->data, cache->length);
    package->CloseFile(file);

    delete cache;

    return true;
}

bool JSBytecodeResource::Start()
{
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    context.Reset(isolate, v8::Context::New(isolate));
    return true;
}

bool JSBytecodeResource::Stop()
{
    context.Reset();
    return true;
}

#pragma once

#include "v8.h"
#include <libplatform/libplatform.h>

class CScriptRuntimeInfo
{
private:
    v8::Isolate* isolate;
    std::unique_ptr<v8::Platform> platform;

public:
    v8::Isolate* GetIsolate() { return isolate; }

    void Instanciate()
    {
        v8::V8::SetFlagsFromString("--harmony-import-assertions --short-builtin-calls --no-lazy --no-flush-bytecode --no-enable-lazy-source-positions");
        platform = v8::platform::NewDefaultPlatform();
        v8::V8::InitializePlatform(platform.get());

        v8::V8::Initialize();

        auto createParams = v8::Isolate::CreateParams{};
        createParams.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();

        isolate = v8::Isolate::New(createParams);
    }

    static CScriptRuntimeInfo& Instance()
    {
        static CScriptRuntimeInfo runtimeInfo;
        return runtimeInfo;
    }
};

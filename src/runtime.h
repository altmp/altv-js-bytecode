#pragma once

#include "cpp-sdk/SDK.h"
#include "v8.h"
#include "libplatform/libplatform.h"

class JSBytecodeRuntime : public alt::IScriptRuntime
{
    v8::Isolate* isolate;
    v8::Isolate::CreateParams create_params;
    std::unique_ptr<v8::Platform> platform;

public:
    JSBytecodeRuntime();

    bool GetProcessClientType(std::string& clientType) override;
    void ProcessClientFile(alt::IResource* resource, alt::IPackage* clientPackage) override;

    v8::Isolate* GetIsolate()
    {
        return isolate;
    }

    alt::IResource::Impl* CreateImpl(alt::IResource* resource) override
    {
        return nullptr;
    }

    void DestroyImpl(alt::IResource::Impl* impl) override {}

    static JSBytecodeRuntime& Instance()
    {
        static JSBytecodeRuntime runtime;
        return runtime;
    }
};

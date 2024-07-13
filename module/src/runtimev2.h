#pragma once

#include "SDK.h"
#include "v8.h"

class JSBytecodeRuntimeV2 : public alt::IScriptRuntime
{
public:
    bool GetProcessClientType(std::string& clientType) override;
    void ProcessClientFile(alt::IResource* resource, alt::IPackage* clientPackage) override;

    alt::IResource::Impl* CreateImpl(alt::IResource* resource) override
    {
        return nullptr;
    }

    void DestroyImpl(alt::IResource::Impl* impl) override {}

    static JSBytecodeRuntimeV2& Instance()
    {
        static JSBytecodeRuntimeV2 runtime;
        return runtime;
    }
};

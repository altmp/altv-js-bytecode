#pragma once

#include "SDK.h"
#include "v8.h"

class JSBytecodeRuntime : public alt::IScriptRuntime
{
public:
    bool GetProcessClientType(std::string& clientType) override;
    void ProcessClientFile(alt::IResource* resource, alt::IPackage* clientPackage) override;

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

#pragma once

#include "cpp-sdk/SDK.h"
#include "v8.h"
#include "resource.h"
#include "libplatform/libplatform.h"

class JSBytecodeRuntime : public alt::IScriptRuntime
{
    v8::Isolate* isolate;
    v8::Isolate::CreateParams create_params;
    std::unique_ptr<v8::Platform> platform;

public:
    JSBytecodeRuntime();

    v8::Isolate* GetIsolate() { return isolate; }

    alt::IResource::Impl *CreateImpl(alt::IResource *resource) override
	{
		return new JSBytecodeResource(resource, isolate);
	}

	void DestroyImpl(alt::IResource::Impl *impl) override 
    { 
        auto res = static_cast<JSBytecodeResource*>(impl);
        delete res;
    }

    static JSBytecodeRuntime& Instance()
    {
        static JSBytecodeRuntime runtime;
        return runtime;
    }
};

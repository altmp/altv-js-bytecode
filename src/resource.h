#pragma once

#include "cpp-sdk/SDK.h"
#include "v8.h"

class JSBytecodeRuntime;

class JSBytecodeResource : public alt::IResource::Impl
{
    v8::Isolate* isolate;
    alt::IResource* resource;

public:
    JSBytecodeResource(alt::IResource* resource, v8::Isolate* isolate) : resource(resource), isolate(isolate) {}

    bool MakeClient(alt::IResource::CreationInfo *info, alt::Array<alt::String> files) override;
};

#include "cpp-sdk/SDK.h"
#include "cpp-sdk/version/version.h"
#include "runtime.h"

EXPORT bool altMain(alt::ICore* core)
{
    alt::ICore::SetInstance(core);

    auto& runtime = JSBytecodeRuntime::Instance();
    core->RegisterScriptRuntime("jsb", &runtime);

    return true;
}

EXPORT const char* GetSDKHash()
{
    return ALT_SDK_VERSION;
}

#include "cpp-sdk/SDK.h"
#include "runtime.h"

EXPORT bool altMain(alt::ICore* core)
{
    alt::ICore::SetInstance(core);

    auto& runtime = JSBytecodeRuntime::Instance();
    core->RegisterScriptRuntime("jsb", &runtime);

    return true;
}

EXPORT uint32_t GetSDKVersion()
{
	return alt::ICore::SDK_VERSION;
}

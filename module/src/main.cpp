#include "CScriptRuntimeInfo.h"
#include "SDK.h"
#include "version/version.h"
#include "Log.h"
#include "runtime.h"
#include "runtimev2.h"

static void CommandHandler(const std::vector<std::string>& args)
{
    if(args.size() == 0)
    {
        Log::Colored << "~y~Usage: ~w~jsb-module [options]" << Log::Endl;
        Log::Colored << "  Use: ~ly~\"jsb-module --help\" ~w~for more info" << Log::Endl;
    }
    else if(args[0] == "--version")
    {
        Log::Colored << "~ly~cpp-sdk: #" << ALT_SDK_VERSION << Log::Endl;
        Log::Colored << "~ly~"
                     << "Copyright | 2022 altMP team." << Log::Endl;

        Log::Colored << "~ly~v8: " << v8::V8::GetVersion() << Log::Endl;
        Log::Colored << "~ly~"
                     << "Copyright | 2014 The V8 project authors." << Log::Endl;
    }
    else if(args[0] == "--help")
    {
        Log::Colored << "~y~Usage: ~w~jsb-module [options]" << Log::Endl;
        Log::Colored << "~y~Options:" << Log::Endl;
        Log::Colored << "  ~ly~--help    ~w~- this message." << Log::Endl;
        Log::Colored << "  ~ly~--version ~w~- version info." << Log::Endl;
    }
}

EXPORT bool altMain(alt::ICore* core)
{
    alt::ICore::SetInstance(core);

    CScriptRuntimeInfo::Instance().Instanciate();

    core->RegisterScriptRuntime("jsb", &JSBytecodeRuntime::Instance());
    core->RegisterScriptRuntime("jsv2b", &JSBytecodeRuntimeV2::Instance());

    core->SubscribeCommand("jsb-module", &CommandHandler);

    return true;
}

EXPORT const char* GetSDKHash()
{
    return ALT_SDK_VERSION;
}

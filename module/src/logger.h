#pragma once

#include "compiler.h"
#include "cpp-sdk/SDK.h"

class Logger : public BytecodeCompiler::ILogger
{
    static constexpr const char* prefix = "[V8 Bytecode] ";

public:
    std::string GetHighlightColor() override
    {
        return "~lg~";
    }

    void Log(const std::string& message) override
    {
        alt::ICore::Instance().LogColored(GetHighlightColor() + prefix + "~w~" + message);
    }
    void LogError(const std::string& message) override
    {
        alt::ICore::Instance().LogError(GetHighlightColor() + prefix + "~w~" + message);
    }
};

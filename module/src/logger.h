#pragma once

#include "compiler.h"
#include "SDK.h"

class Logger : public BytecodeCompiler::ILogger
{
    static constexpr const char* prefix = "[V8 Bytecode]";

public:
    std::string GetHighlightColor() override
    {
        return "~lg~";
    }

    void Log(const std::string& message) override
    {
        alt::ICore::Instance().LogColored(prefix, message);
    }
    void LogError(const std::string& message) override
    {
        alt::ICore::Instance().LogError(std::string(prefix), message);
    }
};

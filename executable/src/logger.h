#pragma once

#include "compiler.h"
#include <iostream>

class Logger : public BytecodeCompiler::ILogger
{
    static constexpr const char* highlightColor = "\x1b[32m";
    static constexpr const char* errorColor = "\x1b[31m";
    static constexpr const char* resetColor = "\x1b[0m";
    static constexpr const char* debugColor = "\x1b[36m";

    static constexpr const char* prefix = "Bytecode compiler  ";

    bool debug = false;

public:
    void ToggleDebugLogs(bool state)
    {
        debug = state;
    }

    std::string GetHighlightColor() override
    {
        return highlightColor;
    }

    void Log(const std::string& message) override
    {
        std::cout << highlightColor << prefix << resetColor << message << resetColor << std::endl;
    }
    void LogError(const std::string& message) override
    {
        std::cerr << errorColor << prefix << resetColor << message << resetColor << std::endl;
    }
    void LogDebug(const std::string& message) override
    {
        if(!debug) return;
        std::cout << debugColor << "[DEBUG] " << highlightColor << prefix << resetColor << message << resetColor << std::endl;
    }

    static Logger& Instance()
    {
        static Logger logger;
        return logger;
    }
};

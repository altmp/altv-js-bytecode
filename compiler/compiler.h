#pragma once

#include "v8.h"

#include <vector>
#include <string>

namespace BytecodeCompiler
{
    class IPackage
    {
    public:
        virtual bool WriteFile(const std::string& file, void* data, size_t size) = 0;
        virtual bool ReadFile(const std::string& file, void* data, size_t size) = 0;
        virtual bool FileExists(const std::string& file) = 0;
        virtual size_t GetFileSize(const std::string& file) = 0;

        // Resolves a JS file path from the given file and base path
        // For example, this should append the `.js` extension if it doesn't exist
        virtual std::string ResolveFile(const std::string& file, const std::string& basePath) = 0;
    };

    class ILogger
    {
    public:
        virtual std::string GetHighlightColor()
        {
            return "";
        }

        virtual void Log(const std::string& message) = 0;
        virtual void LogError(const std::string& message) = 0;
        virtual void LogDebug(const std::string& message) {}
    };

    class Compiler
    {
        v8::Isolate* isolate;
        IPackage* package;
        ILogger* logger;
        std::vector<std::string> compiledFiles;
        std::vector<std::string> ignoredModules;

        std::vector<char> magicBytes = { 'A', 'L', 'T', 'B', 'C' };

    public:
        Compiler() = delete;
        Compiler(v8::Isolate* _isolate, IPackage* _package, ILogger* _logger) : isolate(_isolate), package(_package), logger(_logger) {}

        const std::vector<std::string>& GetCompiledFiles() const
        {
            return compiledFiles;
        }
        void SetIgnoredModules(const std::vector<std::string>& modules)
        {
            ignoredModules = modules;
        }
        const std::vector<std::string>& GetIgnoredModules() const
        {
            return ignoredModules;
        }
        void SetMagicBytes(const std::vector<char>& bytes)
        {
            magicBytes = bytes;
        }
        const std::vector<char>& GetMagicBytes() const
        {
            return magicBytes;
        }

        bool CompileModule(const std::string& fileName, bool compileDependencies = true, bool verbose = false);

        bool IsBytecodeFile(void* buffer, size_t size);

    private:
        std::vector<uint8_t> CreateBytecodeBuffer(const uint8_t* buffer, int length, int sourceLength);

        static void FixBytecode(const uint8_t* buffer, int sourceLength);
    };
}  // namespace BytecodeCompiler

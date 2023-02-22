#pragma once

#include "compiler.h"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class Package : public BytecodeCompiler::IPackage
{
    fs::path outPath;

public:
    Package(fs::path _outPath) : outPath(_outPath) {}

    bool WriteFile(const std::string& fileName, void* data, size_t size) override
    {
        fs::path p(fileName);
        fs::path filePath = p.replace_extension(".jsb").filename();

        std::ofstream file(outPath / filePath, std::ios::out | std::ios::binary);
        if(!file.good()) return false;

        file.write((char*)data, size);
        file.close();
        return true;
    }
    bool ReadFile(const std::string& fileName, void* data, size_t size) override
    {
        std::ifstream file(fileName, std::ios::in | std::ios::binary);
        if(!file.good()) return false;
        file.read((char*)data, size);
        file.close();
        return true;
    }
    bool FileExists(const std::string& fileName) override
    {
        return fs::exists(fileName);
    }
    bool FileExists(const fs::path& path)
    {
        return FileExists(path.string());
    }
    size_t GetFileSize(const std::string& fileName) override
    {
        return fs::file_size(fileName);
    }
    std::string ResolveFile(const std::string& file, const std::string& basePath) override
    {
        fs::path path = (fs::path(basePath).remove_filename() / file).lexically_normal();
        if(!fs::exists(path)) return std::string();
        return path.string();
    }
};

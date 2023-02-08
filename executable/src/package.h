#pragma once

#include "compiler.h"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class Package : public BytecodeCompiler::IPackage
{
    fs::path outPath;
    fs::path inPath;

public:
    Package(fs::path _outPath, fs::path _inPath) : outPath(_outPath), inPath(_inPath) {}

    bool WriteFile(const std::string& fileName, void* data, size_t size) override
    {
        fs::path p (fileName);
        fs::path filePath = p.replace_extension(".bin").filename();

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
        fs::path path = fs::path(basePath) / file;
        if(!fs::exists(path)) return std::string();
        fs::path rootPath = path.root_path().string();
        fs::path fileName = path.filename().string();
        if(fileName.empty())
        {
            if(FileExists(rootPath / "index.js")) fileName = "index.js";
            else if(FileExists(rootPath / "index.mjs"))
                fileName = "index.mjs";
            else
                return std::string();
        }
        else
        {
            if(FileExists(rootPath / fs::path(fileName.string() + ".js"))) fileName += ".js";
            else if(FileExists(rootPath / fs::path(fileName.string() + ".mjs")))
                fileName += ".mjs";
            else if(FileExists(rootPath / fs::path(fileName.string() + "/index.js")))
                fileName += "/index.js";
            else if(FileExists(rootPath / fs::path(fileName.string() + "/index.mjs")))
                fileName += "/index.mjs";
            else if(!FileExists(fileName))
                return std::string();
        }
        return (rootPath / fileName).string();
    }
};

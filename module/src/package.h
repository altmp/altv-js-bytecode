#pragma once

#include "compiler.h"
#include "cpp-sdk/SDK.h"

class Package : public BytecodeCompiler::IPackage
{
    alt::IPackage* writePackage;
    alt::IPackage* readPackage;
    alt::IResource* resource;

    inline alt::IPackage::File* OpenFile(alt::IPackage* package, const std::string& file, bool skipExistsCheck = false)
    {
        if(!skipExistsCheck && !package->FileExists(file)) return nullptr;
        return package->OpenFile(file);
    }

public:
    Package(alt::IPackage* _writePackage, alt::IPackage* _readPackage, alt::IResource* _resource) : writePackage(_writePackage), readPackage(_readPackage), resource(_resource) {}

    bool WriteFile(const std::string& fileName, void* data, size_t size) override
    {
        alt::IPackage::File* file = OpenFile(writePackage, fileName, true);
        if(!file) return false;
        writePackage->WriteFile(file, data, size);
        writePackage->CloseFile(file);
        return true;
    }
    bool ReadFile(const std::string& fileName, void* data, size_t size) override
    {
        alt::IPackage::File* file = OpenFile(readPackage, fileName);
        if(!file) return false;
        readPackage->ReadFile(file, data, size);
        readPackage->CloseFile(file);
        return true;
    }
    bool FileExists(const std::string& fileName) override
    {
        return readPackage->FileExists(fileName);
    }
    size_t GetFileSize(const std::string& fileName) override
    {
        alt::IPackage::File* file = OpenFile(readPackage, fileName);
        size_t size = readPackage->GetFileSize(file);
        readPackage->CloseFile(file);
        return size;
    }

    std::string ResolveFile(const std::string& file, const std::string& basePath) override
    {
        alt::IPackage::PathInfo path = alt::ICore::Instance().Resolve(resource, file, basePath);
        if(!path.pkg) return std::string();
        std::string fileName = path.fileName;
        if(fileName.size() == 0)
        {
            if(path.pkg->FileExists("index.js")) fileName = "index.js";
            else if(path.pkg->FileExists("index.mjs"))
                fileName = "index.mjs";
            else
                return std::string();
        }
        else
        {
            if(path.pkg->FileExists(fileName + ".js")) fileName += ".js";
            else if(path.pkg->FileExists(fileName + ".mjs"))
                fileName += ".mjs";
            else if(path.pkg->FileExists(fileName + "/index.js"))
                fileName += "/index.js";
            else if(path.pkg->FileExists(fileName + "/index.mjs"))
                fileName += "/index.mjs";
            else if(!path.pkg->FileExists(fileName))
                return std::string();
        }
        return path.prefix + fileName;
    }
};

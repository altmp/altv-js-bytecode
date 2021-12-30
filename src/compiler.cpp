#include "compiler.h"
#include "Log.h"

// Replicates the V8 'SerializedCodeData::SourceHash' function
inline uint32_t CreateV8SourceHash(const std::string& src)
{
    uint32_t size = (uint32_t)src.size();
    // We always use modules, so this flag is always used
    static constexpr uint32_t moduleFlagMask = (1 << 31);
    return size | moduleFlagMask;
}

// Copies a uint32 value to the buffer, considering the endianness of the system
inline void CopyValueToBuffer(const uint8_t* buffer, size_t offset, uint32_t val)
{
    bool isLittleEndian = true;
    {
        int n = 1;
        isLittleEndian = *(char*)&n == 1;
    }

    uintptr_t dst = (uintptr_t)buffer + offset;
    if(isLittleEndian) memcpy((void*)(dst), &val, sizeof(val));
    else
    {
        // Code inspired by V8
        uint8_t* src = reinterpret_cast<uint8_t*>(&val);
        uint8_t* dstPtr = reinterpret_cast<uint8_t*>(dst);
        for(size_t i = 0; i < sizeof(val); i++)
        {
            dstPtr[i] = src[sizeof(val) - i - 1];
        }
    }
}

// Gets the file name from JS import specifier
inline std::string GetFileName(alt::IResource* resource, const std::string& name, const std::string& referrer)
{
    alt::IPackage::PathInfo path = alt::ICore::Instance().Resolve(resource, name, referrer);
    if(!path.pkg) return std::string();
    std::string fileName = path.fileName.ToString();
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
    return path.prefix.ToString() + fileName;
}

void Compiler::CompileModuleToBytecode(
  v8::Isolate* isolate, alt::IResource* resource, alt::IPackage* package, const std::string& fileName, const std::string& sourceCode, std::vector<std::string>& compiledFiles)
{
    v8::ScriptOrigin origin(
      isolate, v8::String::NewFromUtf8(isolate, fileName.c_str()).ToLocalChecked(), 0, 0, false, -1, v8::Local<v8::Value>(), false, false, true, v8::Local<v8::PrimitiveArray>());
    v8::ScriptCompiler::Source source(v8::String::NewFromUtf8(isolate, sourceCode.c_str()).ToLocalChecked(), origin);
    v8::MaybeLocal<v8::Module> maybeModule = v8::ScriptCompiler::CompileModule(isolate, &source);
    if(maybeModule.IsEmpty())
    {
        alt::ICore::Instance().LogError("[V8 Bytecode] Failed to compile module: " + fileName);
        return;
    }
    v8::Local<v8::Module> module = maybeModule.ToLocalChecked();
    v8::ScriptCompiler::CachedData* cache = v8::ScriptCompiler::CreateCodeCache(module->GetUnboundModuleScript());
    if(cache == nullptr)
    {
        alt::ICore::Instance().LogError("[V8 Bytecode] Failed to create code cache: " + fileName);
        return;
    }

    alt::IPackage::File* file = package->OpenFile(fileName);
    if(!file)
    {
        alt::ICore::Instance().LogError("[V8 Bytecode] Failed to open file: " + fileName);
        return;
    }

    // * Debug to show current flags hash, useful to overwrite the old flags hash if
    // * the flags in client js changed
    // Log::Info << "Flags hash: " << *(uint32_t*)(cache->data + flagsHashOffset) << Log::Endl;

    std::pair<uint8_t*, size_t> bytecodeResult = CreateBytecodeBuffer(cache->data, cache->length);
    uint8_t* buf = bytecodeResult.first;
    size_t bufSize = bytecodeResult.second;

    package->WriteFile(file, (void*)buf, bufSize);
    package->CloseFile(file);
    delete buf;
    // Make sure the byte buffer is deleted with the cached data from V8
    cache->buffer_policy = v8::ScriptCompiler::CachedData::BufferPolicy::BufferOwned;
    delete cache;

    if(alt::ICore::Instance().IsDebug()) Log::Colored << "~g~[V8 Bytecode] ~w~Converted file to bytecode: ~lg~" << fileName << Log::Endl;
    compiledFiles.push_back(fileName);

    // Convert file dependencies too
    v8::Local<v8::Context> ctx = v8::Context::New(isolate);
    v8::Local<v8::FixedArray> dependencies = module->GetModuleRequests();
    int length = dependencies->Length();
    for(int i = 0; i < length; i++)
    {
        v8::Local<v8::Data> dep = dependencies->Get(ctx, i);
        v8::Local<v8::ModuleRequest> request = dep.As<v8::ModuleRequest>();
        if(request->GetImportAssertions()->Length() > 0) continue;

        v8::Local<v8::String> depStr = request->GetSpecifier();
        std::string depPath = *v8::String::Utf8Value(isolate, depStr);
        if(depPath == "alt" || depPath == "alt-client" || depPath == "natives") continue;

        alt::IPackage::PathInfo pathInfo = alt::ICore::Instance().Resolve(resource, depPath, fileName);
        if(!pathInfo.pkg) continue;

        alt::IPackage::File* file = pathInfo.pkg->OpenFile(pathInfo.fileName);
        size_t fileSize = pathInfo.pkg->GetFileSize(file);
        std::string buffer;
        buffer.resize(fileSize);
        pathInfo.pkg->ReadFile(file, buffer.data(), buffer.size());
        pathInfo.pkg->CloseFile(file);

        CompileModuleToBytecode(isolate, resource, package, GetFileName(resource, (pathInfo.prefix + pathInfo.fileName).ToString(), ""), buffer, compiledFiles);
    }
}

void Compiler::FixBytecode(const uint8_t* buffer)
{
    // Copy hash of empty source file into bytecode source hash section
    // Needed because V8 compares the bytecode code hash to provided source hash
    CopyValueToBuffer(buffer, srcHashOffset, srcHash);

    // Overwrite flags hash with the hash used in client js
    // !!! Make sure to update the hash if flags in client js change !!!
    CopyValueToBuffer(buffer, flagsHashOffset, flagsHash);
}

std::pair<uint8_t*, size_t> Compiler::CreateBytecodeBuffer(const uint8_t* buffer, int length)
{
    // Make necessary changes to the bytecode
    FixBytecode(buffer);

    // Create our own custom bytecode buffer by appending our magic bytes
    // at the front, and then the bytecode itself at the end
    size_t bufSize = sizeof(magicBytes) + length;
    uint8_t* buf = new uint8_t[bufSize];
    memcpy(buf, magicBytes, sizeof(magicBytes));
    memcpy(buf + sizeof(magicBytes), buffer, length);

    return std::make_pair(buf, bufSize);
}

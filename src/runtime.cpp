#include "runtime.h"

#include <sstream>
#include <fstream>
#include <filesystem>

JSBytecodeRuntime::JSBytecodeRuntime()
{
    v8::V8::SetFlagsFromString("--harmony-import-assertions --short-builtin-calls --turbo-fast-api-calls --no-lazy --no-flush-bytecode");
    platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();

    isolate = v8::Isolate::New(create_params);
}

// Replicates the V8 'SerializedCodeData::SourceHash' function
inline uint32_t CreateV8SourceHash(const std::string& src)
{
    uint32_t size = (uint32_t)src.size();
    // We always use modules, so this flag is always used
    static constexpr uint32_t moduleFlagMask = (1 << 31);
    return size | moduleFlagMask;
}

// Code inspired from V8
inline void CopyBigEndianToLittleEndian(uintptr_t dstPtr, uint32_t val)
{
    uint8_t* src = reinterpret_cast<uint8_t*>(&val);
    uint8_t* dst = reinterpret_cast<uint8_t*>(dstPtr);
    for(size_t i = 0; i < sizeof(val); i++)
    {
        dst[i] = src[sizeof(val) - i - 1];
    }
}

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
        CopyBigEndianToLittleEndian(dst, val);
}

void CompileFilesToBytecode(v8::Isolate* isolate, alt::IResource* resource, alt::IPackage* package, const std::string& fileName, const std::string& sourceCode)
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

    // Copy hash of empty source file into bytecode source hash section
    // Needed because V8 compares the bytecode code hash to provided source hash
    static uint32_t srcHash = CreateV8SourceHash("");
    static constexpr int srcHashOffset = 8;
    CopyValueToBuffer(cache->data, srcHashOffset, srcHash);

    // Overwrite flags hash with the hash used in client js
    // !!! Make sure to update the hash if flags in client js change !!!
    static uint32_t flagsHash = 1064582566;
    static constexpr int flagsHashOffset = 12;
    CopyValueToBuffer(cache->data, flagsHashOffset, flagsHash);

    static const char magic[] = { 'A', 'L', 'T', 'B', 'C' };
    size_t bufSize = sizeof(magic) + cache->length;
    uint8_t* buf = new uint8_t[bufSize];
    memcpy(buf, magic, sizeof(magic));
    memcpy(buf + sizeof(magic), cache->data, cache->length);

    package->WriteFile(file, (void*)buf, bufSize);
    package->CloseFile(file);

    delete cache;
    delete buf;

    alt::ICore::Instance().LogColored("~g~[V8 Bytecode] ~w~Converted file to bytecode: ~lg~" + fileName);

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
        if(depPath == "alt" || depPath == "alt-client") continue;

        alt::IPackage::PathInfo pathInfo = alt::ICore::Instance().Resolve(resource, depPath, fileName);
        if(!pathInfo.pkg) continue;

        alt::IPackage::File* file = pathInfo.pkg->OpenFile(pathInfo.fileName);
        size_t fileSize = pathInfo.pkg->GetFileSize(file);
        std::string buffer;
        buffer.resize(fileSize);
        pathInfo.pkg->ReadFile(file, buffer.data(), buffer.size());
        pathInfo.pkg->CloseFile(file);

        CompileFilesToBytecode(isolate, resource, pathInfo.pkg, (pathInfo.prefix + pathInfo.fileName).ToString(), buffer);
    }
}

void JSBytecodeRuntime::WriteClientFile(alt::IResource* resource, alt::IPackage* package, const std::string& fileName, void* buffer, uint64_t size)
{
    if(fileName != resource->GetClientMain()) return;
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);

    alt::String sourceCode(reinterpret_cast<char*>(buffer), size);
    CompileFilesToBytecode(isolate, resource, package, fileName, sourceCode.ToString());

    // Read the extra files
    std::vector<std::string> extraFilePatterns = resource->GetConfigStringList("extra-compile-files");
    std::unordered_set<std::string> files = resource->GetMatchedFiles(extraFilePatterns);
    for(const std::string& file : files)
    {
        alt::IPackage::PathInfo pathInfo = alt::ICore::Instance().Resolve(resource, file, "");
        if(!pathInfo.pkg) continue;
        alt::IPackage::File* pkgFile = pathInfo.pkg->OpenFile(file);
        if(!pkgFile) continue;

        size_t fileSize = package->GetFileSize(pkgFile);
        std::string buffer;
        buffer.resize(fileSize);
        pathInfo.pkg->ReadFile(pkgFile, buffer.data(), buffer.size());
        pathInfo.pkg->CloseFile(pkgFile);

        CompileFilesToBytecode(isolate, resource, pathInfo.pkg, file, buffer);
    }
}

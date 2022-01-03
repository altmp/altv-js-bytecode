#include "compiler.h"
#include "Log.h"

using namespace BytecodeCompiler;

constexpr const char magicBytes[] = { 'A', 'L', 'T', 'B', 'C' };

// Hash for empty module ("")
constexpr uint32_t srcHash = 2147483648;
constexpr int srcHashOffset = 8;

constexpr uint32_t flagsHash = 1064582566;
constexpr int flagsHashOffset = 12;

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

static void FixBytecode(const uint8_t* buffer)
{
    // Copy hash of empty source file into bytecode source hash section
    // Needed because V8 compares the bytecode code hash to provided source hash
    CopyValueToBuffer(buffer, srcHashOffset, srcHash);

    // Overwrite flags hash with the hash used in client js
    // !!! Make sure to update the hash if flags in client js change !!!
    CopyValueToBuffer(buffer, flagsHashOffset, flagsHash);
}

static std::pair<uint8_t*, size_t> CreateBytecodeBuffer(const uint8_t* buffer, int length)
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

void Compiler::CompileModule(const std::string& fileName, bool compileDependencies)
{
    if(!package->FileExists(fileName))
    {
        logger->LogError("File not found: " + logger->GetHighlightColor() + fileName);
        return;
    }
    size_t size = package->GetFileSize(fileName);
    std::string sourceCode;
    sourceCode.resize(size);
    if(!package->ReadFile(fileName, sourceCode.data(), sourceCode.size()))
    {
        logger->LogError("Failed to read file: " + logger->GetHighlightColor() + fileName);
        return;
    }

    v8::ScriptOrigin origin(
      isolate, v8::String::NewFromUtf8(isolate, fileName.c_str()).ToLocalChecked(), 0, 0, false, -1, v8::Local<v8::Value>(), false, false, true, v8::Local<v8::PrimitiveArray>());
    v8::ScriptCompiler::Source source(v8::String::NewFromUtf8(isolate, sourceCode.c_str()).ToLocalChecked(), origin);
    v8::MaybeLocal<v8::Module> maybeModule = v8::ScriptCompiler::CompileModule(isolate, &source);
    if(maybeModule.IsEmpty())
    {
        logger->LogError("Failed to compile module: " + logger->GetHighlightColor() + fileName);
        return;
    }
    v8::Local<v8::Module> module = maybeModule.ToLocalChecked();
    v8::ScriptCompiler::CachedData* cache = v8::ScriptCompiler::CreateCodeCache(module->GetUnboundModuleScript());
    if(cache == nullptr)
    {
        alt::ICore::Instance().LogError("Failed to create code cache: " + logger->GetHighlightColor() + fileName);
        return;
    }
    // * Debug to show current flags hash, useful to overwrite the old flags hash if
    // * the flags in client js changed
    // Log::Info << "Flags hash: " << *(uint32_t*)(cache->data + flagsHashOffset) << Log::Endl;

    std::pair<uint8_t*, size_t> bytecodeResult = CreateBytecodeBuffer(cache->data, cache->length);
    uint8_t* buf = bytecodeResult.first;
    size_t bufSize = bytecodeResult.second;

    package->WriteFile(fileName, (void*)buf, bufSize);
    delete buf;
    // Make sure the byte buffer is deleted with the cached data from V8
    cache->buffer_policy = v8::ScriptCompiler::CachedData::BufferPolicy::BufferOwned;
    delete cache;

    logger->Log("Converted file to bytecode: " + logger->GetHighlightColor() + fileName);
    compiledFiles.push_back(fileName);

    // Convert file dependencies too
    if(compileDependencies)
    {
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

            std::string fullFileName = package->ResolveFile(depPath, fileName);
            size_t fileSize = package->GetFileSize(fullFileName);
            std::string buffer;
            buffer.resize(fileSize);
            if(!package->ReadFile(fullFileName, buffer.data(), buffer.size()))
            {
                logger->LogError("Failed to read dependency file: " + logger->GetHighlightColor() + fullFileName);
                continue;
            }

            CompileModule(fullFileName, true);
        }
    }
}

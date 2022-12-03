#include "compiler.h"
#include "helpers.h"

#include <algorithm>

using namespace BytecodeCompiler;

bool Compiler::CompileModule(const std::string& fileName, bool compileDependencies)
{
    // Read the file
    if(!package->FileExists(fileName))
    {
        logger->LogError("File not found: " + fileName);
        return false;
    }
    size_t size = package->GetFileSize(fileName);
    std::string sourceCode;
    sourceCode.resize(size);
    if(!package->ReadFile(fileName, sourceCode.data(), sourceCode.size()))
    {
        logger->LogError("Failed to read file: " + fileName);
        return false;
    }

    v8::TryCatch tryCatch(isolate);
    v8::Local<v8::Context> ctx = v8::Context::New(isolate);
    v8::Context::Scope ctxScope(ctx);

    // Compile the file to a JavaScript module
    v8::ScriptOrigin origin(
      isolate, v8::String::NewFromUtf8(isolate, fileName.c_str()).ToLocalChecked(), 0, 0, false, -1, v8::Local<v8::Value>(), false, false, true, v8::Local<v8::PrimitiveArray>());
    v8::ScriptCompiler::Source source(v8::String::NewFromUtf8(isolate, sourceCode.c_str()).ToLocalChecked(), origin);
    v8::MaybeLocal<v8::Module> maybeModule = v8::ScriptCompiler::CompileModule(isolate, &source);
    if(maybeModule.IsEmpty() || tryCatch.HasCaught())
    {
        logger->LogError("Failed to compile module: " + fileName);
        Helpers::CheckTryCatch(fileName, logger, tryCatch, ctx);
        return false;
    }

    // Retrieve the bytecode from the module
    v8::Local<v8::Module> module = maybeModule.ToLocalChecked();
    v8::ScriptCompiler::CachedData* cache = v8::ScriptCompiler::CreateCodeCache(module->GetUnboundModuleScript());
    if(cache == nullptr || tryCatch.HasCaught())
    {
        logger->LogError("Failed to create bytecode: " + fileName);
        Helpers::CheckTryCatch(fileName, logger, tryCatch, ctx);
        return false;
    }

    // Write the bytecode to file
    std::vector<uint8_t> bytecodeResult = CreateBytecodeBuffer(cache->data, cache->length, sourceCode.size());
    bool writeResult = package->WriteFile(fileName, (void*)bytecodeResult.data(), bytecodeResult.size());
    if(!writeResult)
    {
        logger->LogError("Failed to write to file: " + fileName);
        return false;
    }

    // Make sure the byte buffer is deleted with the cached data from V8
    cache->buffer_policy = v8::ScriptCompiler::CachedData::BufferPolicy::BufferOwned;
    delete cache;

    logger->Log("Converted file to bytecode: " + logger->GetHighlightColor() + fileName);
    compiledFiles.push_back(fileName);

    // Compile all dependencies
    if(compileDependencies)
    {
        v8::Local<v8::Context> ctx = v8::Context::New(isolate);
        v8::Local<v8::FixedArray> dependencies = module->GetModuleRequests();
        int length = dependencies->Length();
        for(int i = 0; i < length; i++)
        {
            v8::Local<v8::Data> dep = dependencies->Get(ctx, i);
            v8::Local<v8::ModuleRequest> request = dep.As<v8::ModuleRequest>();
            // Ignore all imports with import assertions, as those are not loaded as
            // normal JS files
            if(request->GetImportAssertions()->Length() > 0) continue;

            v8::Local<v8::String> depStr = request->GetSpecifier();
            std::string depPath = *v8::String::Utf8Value(isolate, depStr);
            // Ignore the built-in modules
            if(std::find(ignoredModules.begin(), ignoredModules.end(), depPath) != ignoredModules.end()) continue;

            // Compile the dependency file
            std::string fullFileName = package->ResolveFile(depPath, fileName);

            // Check if the file has already been compiled
            if(std::find(compiledFiles.begin(), compiledFiles.end(), fullFileName) != compiledFiles.end()) continue;

            // Dont compile if the module is ignored
            if(std::find(ignoredModules.begin(), ignoredModules.end(), fullFileName) != ignoredModules.end()) continue;

            if(!CompileModule(fullFileName, true)) return false;
        }
    }

    return true;
}

bool Compiler::IsBytecodeFile(void* buffer, size_t size)
{
    if(size < magicBytes.size()) return false;
    if(memcmp(buffer, magicBytes.data(), magicBytes.size()) != 0) return false;
    return true;
}

std::vector<uint8_t> Compiler::CreateBytecodeBuffer(const uint8_t* buffer, int length, int sourceLength)
{
    // Make necessary changes to the bytecode
    FixBytecode(buffer, sourceLength);

    // Create our own custom bytecode buffer by appending our magic bytes
    // at the front, and then the bytecode itself at the end
    std::vector<uint8_t> buf;
    size_t bufSize = magicBytes.size() + sizeof(int) + length;
    buf.resize(bufSize);

    memcpy(buf.data(), magicBytes.data(), magicBytes.size());
    memcpy(buf.data() + magicBytes.size(), &sourceLength, sizeof(int));
    memcpy(buf.data() + magicBytes.size() + sizeof(int), buffer, length);

    return buf;
}

static constexpr int srcHashOffset = 8;

static constexpr uint32_t flagsHash = 3901848073;
static constexpr int flagsHashOffset = 12;

void Compiler::FixBytecode(const uint8_t* buffer, int sourceLength)
{
    // Copy hash of source into bytecode source hash section
    // Needed because V8 compares the bytecode code hash to provided source hash
    Helpers::CopyValueToBuffer(buffer, srcHashOffset, Helpers::CreateV8SourceHash(sourceLength));

    // Overwrite flags hash with the hash used in client js
    // !!! Make sure to update the hash if flags in client js change !!!
    Helpers::CopyValueToBuffer(buffer, flagsHashOffset, flagsHash);
}

#pragma once
#include "v8.h"

namespace BytecodeCompiler 
{
    struct Bytecode
    {
        uint8_t* bytecode;
        int size;
        uint32_t version;

        Bytecode(v8::ScriptCompiler::CachedData* data) : size(data->length), version(GetVersion()) 
        {
            bytecode = new uint8_t[data->length];
            memcpy(bytecode, data->data, data->length);
        }
        ~Bytecode() { delete bytecode; }
    };

    extern uint32_t GetVersion();
    extern Bytecode CompileSourceIntoBytecode(v8::Isolate* isolate, v8::ScriptOrigin& origin, const char* sourceCode);
}

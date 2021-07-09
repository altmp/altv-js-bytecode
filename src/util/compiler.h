#pragma once

#include "v8.h"

namespace BytecodeCompiler 
{
    v8::ScriptCompiler::CachedData* CompileSourceIntoBytecode(v8::Isolate* isolate, const char* name, const char* sourceCode);
};

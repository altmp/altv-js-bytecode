#pragma once

#include "bytecode.h"

namespace BytecodeCompiler 
{
    Bytecode CompileSourceIntoBytecode(v8::Isolate* isolate, const char* name, const char* sourceCode);
};

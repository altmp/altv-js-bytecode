#include "compiler.h"
#include <iostream>

BytecodeCompiler::Bytecode BytecodeCompiler::CompileSourceIntoBytecode(v8::Isolate* isolate, const char* name, const char* sourceCode)
{
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope scope(isolate);

    v8::ScriptOrigin scriptOrigin(
		isolate,
		v8::String::NewFromUtf8(isolate, name).ToLocalChecked(),
		0,
		0, 
        false,
		-1,
		v8::Local<v8::Value>(),
		false,
		false,
		true,
		v8::Local<v8::PrimitiveArray>()
	);
    v8::ScriptCompiler::Source compilerSource(v8::String::NewFromUtf8(isolate, sourceCode).ToLocalChecked(), scriptOrigin);
    auto script = 
        v8::ScriptCompiler::CompileModule(isolate, &compilerSource, v8::ScriptCompiler::CompileOptions::kNoCompileOptions).
        ToLocalChecked()->
        GetUnboundModuleScript();
    auto data = v8::ScriptCompiler::CreateCodeCache(script);
    return BytecodeCompiler::Bytecode{ data };
}

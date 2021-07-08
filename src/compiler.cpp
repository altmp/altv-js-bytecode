#include "compiler.h"

extern BytecodeCompiler::Bytecode BytecodeCompiler::CompileSourceIntoBytecode(v8::Isolate* isolate, const char* name, const char* sourceCode)
{
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope scope(isolate);

    v8::ScriptOrigin origin(
		isolate,
		v8::String::NewFromUtf8(isolate, name).ToLocalChecked(),
		0,
		0, false,
		-1,
		v8::Local<v8::Value>(),
		false,
		false,
		true,
		v8::Local<v8::PrimitiveArray>()
	);
    v8::ScriptCompiler::Source source(v8::String::NewFromUtf8(isolate, sourceCode).ToLocalChecked(), origin);
    auto maybeModule = v8::ScriptCompiler::CompileModule(isolate, &source);
    auto module = maybeModule.ToLocalChecked();
    auto unboundScript = module->GetUnboundModuleScript();
    auto data = v8::ScriptCompiler::CreateCodeCache(unboundScript);
    return BytecodeCompiler::Bytecode{ data };
}

extern uint32_t BytecodeCompiler::GetVersion() 
{
    return v8::ScriptCompiler::CachedDataVersionTag();
}

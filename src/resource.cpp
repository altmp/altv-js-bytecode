#include "resource.h"
#include "util/compiler.h"
#include <sstream>

bool JSBytecodeResource::MakeClient(alt::IResource::CreationInfo *info, alt::Array<alt::String> files) 
{
    if(info->type != "jsb" && info->type != "js") return true;

    v8::Locker locker(isolate);
	v8::Isolate::Scope isolateScope(isolate);
	v8::HandleScope handleScope(isolate);

    auto file = info->pkg->OpenFile(info->main);

	alt::String source(info->pkg->GetFileSize(file));
	info->pkg->ReadFile(file, source.GetData(), source.GetSize());
    auto bytecode = BytecodeCompiler::CompileSourceIntoBytecode(isolate, info->main.CStr(), source.CStr());
    std::cout << "Bytecode: " << (uintptr_t)bytecode->data << std::endl;
    info->pkg->WriteFile(file, (void*)bytecode->data, bytecode->length);
    info->pkg->CloseFile(file);

    std::stringstream stream;
    stream << __FUNCTION__ << " " << "size: " << bytecode->length;
    alt::ICore::Instance().LogWarning(stream.str());

    return true;
}

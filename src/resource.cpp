#include "resource.h"
#include "util/compiler.h"
#include <sstream>
#include <fstream>

bool JSBytecodeResource::MakeClient(alt::IResource::CreationInfo *info, alt::Array<alt::String> files) 
{
    if(info->type != "jsb" && info->type != "js") return true;

    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);

    alt::String path = resource->GetPath() + "/" + info->main;
    std::ifstream iFile;
    iFile.open(path.CStr());
    std::string source((std::istreambuf_iterator<char>(iFile)), std::istreambuf_iterator<char>());

    auto bytecode = BytecodeCompiler::CompileSourceIntoBytecode(isolate, info->main.CStr(), source.c_str());
    auto file = info->pkg->OpenFile(info->main);
    info->pkg->WriteFile(file, (void*)bytecode->data, bytecode->length);
    info->pkg->CloseFile(file);

    std::cout << __FUNCTION__ << " " << "size: " << bytecode->length << std::endl;
    std::cout << __FUNCTION__ << " " << "version: " << v8::ScriptCompiler::CachedDataVersionTag() << std::endl;

    return true;
}

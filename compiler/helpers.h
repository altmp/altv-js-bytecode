#pragma once

#include <stdint.h>
#include "v8.h"
#include "compiler.h"

namespace Helpers
{
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
    inline void CheckTryCatch(const std::string& fileName, BytecodeCompiler::ILogger* logger, v8::TryCatch& tryCatch, v8::Local<v8::Context> ctx)
    {
        if(tryCatch.HasCaught())
        {
            v8::Local<v8::Message> message = tryCatch.Message();
            if(!message.IsEmpty())
            {
                v8::Isolate* isolate = ctx->GetIsolate();
                v8::MaybeLocal<v8::String> maybeString = message->Get();
                v8::MaybeLocal<v8::String> maybeSourceLine = message->GetSourceLine(ctx);
                v8::Maybe<int> maybeLine = message->GetLineNumber(ctx);

                if(!maybeLine.IsNothing()) logger->LogError("Exception at " + fileName + ":" + std::to_string(maybeLine.ToChecked()));
                if(!maybeString.IsEmpty()) logger->LogError(*v8::String::Utf8Value(isolate, maybeString.ToLocalChecked()));
                if(!maybeSourceLine.IsEmpty()) logger->LogError(*v8::String::Utf8Value(isolate, maybeSourceLine.ToLocalChecked()));
            }
        }
    }
}  // namespace Helpers

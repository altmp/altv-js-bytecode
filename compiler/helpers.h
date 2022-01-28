#pragma once

#include <stdint.h>

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
}  // namespace Helpers

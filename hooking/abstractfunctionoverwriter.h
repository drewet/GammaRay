/*
  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2010-2011 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Andreas Holzammer <andreas.holzammer@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ABSTRACTFUNCTIONOVERWRITER_H
#define ABSTRACTFUNCTIONOVERWRITER_H

#include <QList>
#include <QString>

#if defined(_M_X64) || defined(__amd64) || defined(__x86_64)
#define ARCH_64
#elif defined(_M_IX86) || defined(__i386__)
#define ARCH_X86
#endif

namespace GammaRay {

class AbstractFunctionOverwriter
{
public:
    virtual ~AbstractFunctionOverwriter();

    /**
     * Overwrites a given Qt Core 4 function with a replacement function.
     * @param orignalFunc orignal Qt Core 4 function
     * @param replacementFunc replacement function
     */
    bool overwriteFunction(const QString &orignalFunc, void * const replacementFunc);

protected:
    /**
     * Unprotects a memory region so that the memory can get written
     * @param mem start address of the memory that should be unprotected
     * @param size size of the memory region
     */
    virtual bool unprotectMemory(void *mem, int size) = 0;
    /**
     * Restore the protection of the memory region that has been
     * unproteced before with unprotectMemory
     * @param mem start address of the memory that should be protected
     * @param size size of the memory region
     */
    virtual bool reprotectMemory(void *mem, int size) = 0;

    /**
     * Writes a short jump at a given target to jump to a function.
     * NOTE: A short jump takes 5 bytes, so make sure these 5 bytes
     * can be written.
     * @param target position where the jump should be written to
     * @param size size of the memory region
     */
    virtual bool writeShortJump(void *target, void * const func);
    /**
     * Writes a long jump at a given target to jump to a function.
     * NOTE: A short jump takes 10 bytes for x86 and 14 bytes for x64,
     * so make sure these 5 bytes can be written.
     * @param target position where the jump should be written to
     * @param size size of the memory region
     */
    virtual bool writeLongJump(void *target, void * const func);

    /**
     * Get memory close to a target address.
     * @param addr position that should the memory be close to
     * @param size size of the memory region
     */
    virtual void* getMemoryNearAddress(void * const addr, int size);

    /**
     * Creates a tranpoline function close to another function.
     * @param func function that the tranpoline should be close to
     * @param replacement function that the trampoline should point to
     */
    virtual void* createTrampoline(void * const func, void * const replacement);

    /**
     * Get address range in which the program resists.
     * @param min min address
     * @param max max address
     */
    virtual bool getAddressRange(intptr_t &min, intptr_t &max) = 0;

    /**
     * Check if the memory region is not already taken.
     * @param mem start address
     * @param size size of the memory region
     */
    virtual bool isMemoryFree(void * const mem, int size) = 0;

    /**
     * Allocate memory at the griven position
     * @param mem start address
     * @param size size of the memory region
     */
    virtual void* reserveMemory(void *mem, int size) = 0;

    /**
     * Commit memory at the griven position
     * @param mem start address
     * @param size size of the memory region
     */
    virtual bool commitMemory(void *mem, int size) = 0;

    /**
     * Lookup function address of a given Qt Core 4 function.
     * @param function function name
     */
    virtual void* qtCoreFunctionLookup(const QString &function) = 0;

private:
    struct MemorySegment {
        void *mem;
        int size;
        int free;
    };

    QList<MemorySegment> memoryPool;
};

}

#endif // ABSTRACTFUNCTIONOVERWRITER_H
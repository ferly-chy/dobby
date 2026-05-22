#pragma once

#include "MemoryAllocator/AssemblyCodeBuilder.h"

std::unique_ptr<CodeBufferBase> GenerateNormalTrampolineBuffer(addr_t from, addr_t to);
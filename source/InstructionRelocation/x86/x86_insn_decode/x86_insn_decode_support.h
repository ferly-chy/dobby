#ifndef X86_INSN_DECODE_SUPPORT_H
#define X86_INSN_DECODE_SUPPORT_H

#include <string.h>

#include "base/check_logging.h"

#define X86_INSN_DEBUG_LOG(...) ((void)0)
#define X86_INSN_ERROR_LOG(...) ((void)0)
#define X86_INSN_UNIMPLEMENTED() ASSERT(0)

#endif

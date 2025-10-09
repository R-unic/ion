#include "ion/diagnostics.h"
#include "../include/ion/symbols/symbol.h"

type_ptr_t Symbol::get_type()
{
    COMPILER_ASSERT(type.has_value(), "No type for symbol found");
    return std::move(*type);
}
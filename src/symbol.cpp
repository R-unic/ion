#include "ion/diagnostics.h"
#include "ion/symbol.h"

type_ptr_t Symbol::get_type()
{
    COMPILER_ASSERT(type.has_value(), "No type for symbol '" + name + "' found");
    return std::move(*type);
}
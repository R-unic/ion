#include "ion/diagnostics.h"
#include "ion/ast/node.h"

symbol_ptr_t SyntaxNode::get_symbol()
{
    COMPILER_ASSERT(symbol.has_value(), "No symbol for AST node found");
    return std::move(*symbol);
}
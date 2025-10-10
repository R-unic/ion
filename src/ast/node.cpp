#include "ion/diagnostics.h"
#include "ion/ast/node.h"

Token SyntaxNode::get_last_token() const
{
    return get_first_token();
}

FileSpan SyntaxNode::get_span() const
{
    const auto first = get_first_token();
    const auto last = get_last_token();
    if (first.span == last.span)
        return first.span;

    return create_span(first.span.start, last.span.end);
}

symbol_ptr_t SyntaxNode::get_symbol()
{
    COMPILER_ASSERT(symbol.has_value(), "No symbol for AST node found");
    return std::move(*symbol);
}
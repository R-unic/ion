#pragma once
#include <string>

#include "ast/expressions.h"
#include "ast/abstract/syntax_node.h"

class ast_viewer final : public virtual expression_visitor<std::string>, public virtual statement_visitor<std::string>
{
public:
    std::string visit(expression expression)
    {
        return expression.accept(*this);
    }
    
    std::string visit(literal literal) override
    {
        return std::format("literal {{ token: {} }}", literal.token);
    }
};

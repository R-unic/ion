#pragma once
#include <algorithm>
#include <optional>

#include "ion/token.h"
#include "ion/ast/node.h"

class InstanceConstructor final : public NamedDeclaration
{
public:
    // TODO: allow no type when `clone` clause is used
    Token instance_keyword;
    std::optional<Token> long_arrow, clone_keyword;
    ColonTypeClause* colon_type;
    std::optional<BracedStatementList*> declarators;
    std::optional<expression_ptr_t> clone_target, parent;

    explicit InstanceConstructor(
        Token instance_keyword,
        Token name,
        ColonTypeClause* colon_type,
        std::optional<Token> clone_keyword,
        std::optional<expression_ptr_t> clone_target,
        std::optional<BracedStatementList*> declarators,
        std::optional<Token> long_arrow,
        std::optional<expression_ptr_t> parent)
        : NamedDeclaration(std::move(name)),
          instance_keyword(std::move(instance_keyword)),
          long_arrow(std::move(long_arrow)),
          clone_keyword(std::move(clone_keyword)),
          colon_type(colon_type),
          declarators(declarators),
          clone_target(std::move(clone_target)),
          parent(std::move(parent))
    {
    }

    static statement_ptr_t create(
        Token instance_keyword,
        Token name,
        ColonTypeClause* colon_type,
        std::optional<Token> clone_keyword,
        std::optional<expression_ptr_t> clone_target,
        std::optional<BracedStatementList*> declarators,
        std::optional<Token> long_arrow,
        std::optional<expression_ptr_t> parent)
    {
        return std::make_unique<InstanceConstructor>(std::move(instance_keyword), std::move(name), colon_type,
                                                     std::move(clone_keyword), std::move(clone_target), declarators,
                                                     std::move(long_arrow), std::move(parent));
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_instance_constructor(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return instance_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return parent.has_value()
                   ? parent.value()->get_last_token()
                   : declarators.has_value()
                         ? declarators.value()->get_last_token()
                         : clone_target.has_value()
                               ? clone_target.value()->get_last_token()
                               : colon_type->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto clone_clause_text = clone_target.has_value() ? " clone " + clone_target.value()->get_text() : "";
        const auto declarators_text = declarators.has_value() ? declarators.value()->get_text() + ' ' : "";
        const auto parent_text = parent.has_value() ? "-> " + parent.value()->get_text() : "";

        return "instance " + name.get_text() + colon_type->get_text() + clone_clause_text + ' ' + declarators_text + parent_text;
    }
};
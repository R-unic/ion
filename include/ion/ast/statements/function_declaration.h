#pragma once
#include <algorithm>
#include <optional>

#include "../containers/function_body.h"
#include "../containers/parameter_list_clause.h"
#include "ion/token.h"
#include "ion/ast/node.h"

class FunctionDeclaration final : public NamedDeclaration
{
public:
    std::vector<statement_ptr_t> decorator_list;
    std::optional<Token> async_keyword;
    Token fn_keyword;
    std::optional<TypeListClause*> type_parameters;
    std::optional<ParameterListClause*> parameters;
    std::optional<ColonTypeClause*> return_type;
    FunctionBody* body;

    explicit FunctionDeclaration(
        std::vector<statement_ptr_t> decorator_list,
        std::optional<Token> async_keyword,
        Token fn_keyword,
        Token name,
        const std::optional<TypeListClause*> type_parameters,
        const std::optional<ParameterListClause*> parameters,
        const std::optional<ColonTypeClause*> return_type,
        FunctionBody* body)
        : NamedDeclaration(std::move(name)),
          decorator_list(std::move(decorator_list)),
          async_keyword(std::move(async_keyword)),
          fn_keyword(std::move(fn_keyword)),
          type_parameters(type_parameters),
          parameters(parameters),
          return_type(return_type),
          body(body)
    {
    }

    ~FunctionDeclaration() override
    {
        if (type_parameters.has_value())
            delete *type_parameters;
        if (parameters.has_value())
            delete *parameters;
        if (return_type.has_value())
            delete *return_type;

        delete body;
    }

    static statement_ptr_t create(
        std::vector<statement_ptr_t> decorator_list,
        std::optional<Token> async_keyword,
        Token fn_keyword,
        Token name,
        const std::optional<TypeListClause*> type_parameters,
        const std::optional<ParameterListClause*> parameters,
        const std::optional<ColonTypeClause*> return_type,
        FunctionBody* body)
    {
        return std::make_unique<FunctionDeclaration>(std::move(decorator_list), std::move(async_keyword), std::move(fn_keyword),
                                                     std::move(name), type_parameters, parameters, return_type, body);
    }

    void accept(StatementVisitor<void>& visitor) override
    {
        return visitor.visit_function_declaration(*this);
    }

    [[nodiscard]] Token get_first_token() const override
    {
        return async_keyword.has_value() ? *async_keyword : fn_keyword;
    }

    [[nodiscard]] Token get_last_token() const override
    {
        return body->get_last_token();
    }

    [[nodiscard]] std::string get_text() const override
    {
        const auto type_parameters_text = type_parameters.has_value() ? type_parameters.value()->get_text() : "";
        const auto parameters_text = parameters.has_value() ? '(' + join_by(parameters.value()->list, ", ") + ')' : "";
        const auto return_type_text = return_type.has_value() ? ": " + return_type.value()->get_text() : "";

        return "fn " + name.get_text() + type_parameters_text + parameters_text + return_type_text + ' ' + body->get_text();
    }
};
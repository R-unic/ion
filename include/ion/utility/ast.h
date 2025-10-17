#pragma once
#include <vector>
#include <string>

#include "basic.h"
#include "ion/diagnostics.h"
#include "ion/ast/node.h"

static void check_for_ambiguous_equals(const expression_ptr_t& condition)
{
    if (condition->is_assignment())
        warn_ambiguous_equals(condition);
}

static void check_for_unreachable_code(const std::vector<statement_ptr_t>& statements)
{
    auto return_used = false;
    for (const auto& statement : statements)
        if (statement->is_return())
            return_used = true;
        else if (return_used)
            warn_unreachable_code(statement);
}

inline void assert_nameof_target(const expression_ptr_t& expression)
{
    if (!expression->is_name_of_target())
        report_invalid_nameof(expression);
}

inline void assert_assignment_target(const expression_ptr_t& expression)
{
    if (!expression->is_assignment_target())
        report_invalid_assignment(expression);
}

inline std::string join_by(const std::vector<expression_ptr_t>& expressions, const std::string& separator)
{
    std::vector<std::string> texts = {};
    texts.reserve(texts.size());
    for (const auto& expression : expressions)
        texts.push_back(expression->get_text());

    return join_strings_by(texts, separator);
}

inline std::string join_by(const std::vector<statement_ptr_t>& statements, const std::string& separator)
{
    std::vector<std::string> texts = {};
    texts.reserve(texts.size());
    for (const auto& statement : statements)
        texts.push_back(statement->get_text());

    return join_strings_by(texts, separator);
}

inline std::string join_by(const std::vector<type_ref_ptr_t>& type_refs, const std::string& separator)
{
    std::vector<std::string> texts = {};
    texts.reserve(texts.size());
    for (const auto& type_ref : type_refs)
        texts.push_back(type_ref->get_text());

    return join_strings_by(texts, separator);
}

inline std::string join_tokens_by(const std::vector<Token>& tokens, const std::string& separator)
{
    std::vector<std::string> texts = {};
    texts.reserve(texts.size());
    for (const auto& token : tokens)
        texts.push_back(token.get_text());

    return join_strings_by(texts, separator);
}
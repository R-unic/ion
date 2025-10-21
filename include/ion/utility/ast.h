#pragma once
#include <vector>
#include <string>

#include "basic.h"
#include "ion/diagnostics.h"
#include "ion/ast/node.h"

#define DEFINE_JOIN_BY_METHOD(element_name, string_eval, element_type) \
    inline std::string join_by(const std::vector<element_type>& element_name##s, const std::string& separator) \
    { \
        std::vector<std::string> texts = {}; \
        texts.reserve(element_name##s.size()); \
        for (const auto& element_name : element_name##s) \
            texts.push_back(string_eval()); \
        return join_by(texts, separator); \
    }

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

DEFINE_JOIN_BY_METHOD(type, type->to_string, type_ptr_t);
DEFINE_JOIN_BY_METHOD(expression, expression->get_text, expression_ptr_t);
DEFINE_JOIN_BY_METHOD(statement, statement->get_text, statement_ptr_t);
DEFINE_JOIN_BY_METHOD(type_ref, type_ref->get_text, type_ref_ptr_t);
DEFINE_JOIN_BY_METHOD(token, token.get_text, Token);

inline std::string generics_to_string(const std::vector<type_ptr_t>& type_parameters)
{
    return type_parameters.empty() ? "" : '<' + join_by(type_parameters, ", ") + '>';
}
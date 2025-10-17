#pragma once

#define COMPILER_ASSERT(condition, error_message) \
    if (!(condition)) report_compiler_error(error_message)

#define EMPTY_DIAGNOSTIC(name) \
struct name \
{ \
};

#define BASIC_DIAGNOSTIC(name, member) \
struct name \
{ \
    std::string member; \
};

#define GENERATE_ERROR_NODE_OVERLOADS_H(fn) \
[[noreturn]] void fn(const expression_ptr_t& expression); \
[[noreturn]] void fn(const statement_ptr_t& statement); \
[[noreturn]] void fn(const Token& token);

#define GENERATE_NODE_OVERLOADS_H(fn) \
void fn(const expression_ptr_t& expression); \
void fn(const statement_ptr_t& statement); \
void fn(const Token& token);

#define GENERATE_ERROR_NODE_OVERLOADS_WITH_TEXT(fn) \
[[noreturn]] void fn(const expression_ptr_t& expression) \
{ \
fn(expression->get_span(), expression->get_text()); \
} \
[[noreturn]] void fn(const statement_ptr_t& statement) \
{ \
fn(statement->get_span(), statement->get_text()); \
} \
[[noreturn]] void fn(const Token& token) \
{ \
fn(token.span, token.get_text()); \
}

#define GENERATE_ERROR_NODE_OVERLOADS(fn) \
[[noreturn]] void fn(const expression_ptr_t& expression) \
{ \
fn(expression->get_span()); \
} \
[[noreturn]] void fn(const statement_ptr_t& statement) \
{ \
fn(statement->get_span()); \
} \
[[noreturn]] void fn(const Token& token) \
{ \
fn(token.span); \
}

#define GENERATE_ERROR_NODE_OVERLOADS_WITH_TEXT(fn) \
[[noreturn]] void fn(const expression_ptr_t& expression) \
{ \
fn(expression->get_span(), expression->get_text()); \
} \
[[noreturn]] void fn(const statement_ptr_t& statement) \
{ \
fn(statement->get_span(), statement->get_text()); \
} \
[[noreturn]] void fn(const Token& token) \
{ \
fn(token.span, token.get_text()); \
}

#define GENERATE_NODE_OVERLOADS(fn) \
void fn(const expression_ptr_t& expression) \
{ \
fn(expression->get_span()); \
} \
void fn(const statement_ptr_t& statement) \
{ \
fn(statement->get_span()); \
} \
void fn(const Token& token) \
{ \
fn(token.span); \
}

#define GENERATE_NODE_OVERLOADS_WITH_TEXT(fn) \
void fn(const expression_ptr_t& expression) \
{ \
fn(expression->get_span(), expression->get_text()); \
} \
void fn(const statement_ptr_t& statement) \
{ \
fn(statement->get_span(), statement->get_text()); \
} \
void fn(const Token& token) \
{ \
fn(token.span, token.get_text()); \
}
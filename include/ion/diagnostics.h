#pragma once
#include <cstdint>
#include <variant>
#include <string>

#include "diagnostics_macros.h"
#include "ast/node.h"

enum class DiagnosticSeverity : uint8_t
{
    Error,
    Warning,
    Info,
    Debug
};


struct UnexpectedCharacter
{
    char character;
};

struct ExpectedDifferentSyntax
{
    std::string expected;
    std::string got;
    bool quote_expected;
};

EMPTY_DIAGNOSTIC(UnexpectedEOF);

EMPTY_DIAGNOSTIC(UnreachableCode);

EMPTY_DIAGNOSTIC(AmbiguousEquals);

BASIC_DIAGNOSTIC(MalformedNumber, malformed);

BASIC_DIAGNOSTIC(UnterminatedString, body);

BASIC_DIAGNOSTIC(UnexpectedSyntax, lexeme);

BASIC_DIAGNOSTIC(InvalidAssignment, lexeme);

BASIC_DIAGNOSTIC(InvalidExport, lexeme);

BASIC_DIAGNOSTIC(InvalidNameOf, lexeme);

EMPTY_DIAGNOSTIC(InvalidDecoratorTarget);

EMPTY_DIAGNOSTIC(InvalidBreak);

EMPTY_DIAGNOSTIC(InvalidContinue);

EMPTY_DIAGNOSTIC(InvalidReturn);

EMPTY_DIAGNOSTIC(InvalidAwait);

BASIC_DIAGNOSTIC(DuplicateVariable, name);

BASIC_DIAGNOSTIC(DuplicateField, field_type);

BASIC_DIAGNOSTIC(VariableNotFound, name);

EMPTY_DIAGNOSTIC(VariableReadInOwnInitializer);

EMPTY_DIAGNOSTIC(NoVariableTypeOrInitializer);

using diagnostic_data_t = std::variant<
    UnexpectedCharacter,
    MalformedNumber,
    UnterminatedString,
    UnexpectedSyntax,
    UnexpectedEOF,
    ExpectedDifferentSyntax,
    InvalidAssignment,
    InvalidExport,
    InvalidNameOf,
    InvalidDecoratorTarget,
    DuplicateVariable,
    VariableNotFound,
    VariableReadInOwnInitializer,
    InvalidBreak,
    InvalidContinue,
    InvalidReturn,
    InvalidAwait,
    DuplicateField,
    NoVariableTypeOrInitializer,
    UnreachableCode,
    AmbiguousEquals
>;

struct Diagnostic
{
    uint8_t code {};
    DiagnosticSeverity severity = DiagnosticSeverity::Error;
    FileSpan span;
    diagnostic_data_t data;
};

[[noreturn]] void report_compiler_error(const std::string&);
[[noreturn]] void report_unexpected_character(const FileSpan&, char);
[[noreturn]] void report_malformed_number(const FileSpan&, const std::string&);
[[noreturn]] void report_unterminated_string(const FileSpan&, const std::string&);
GENERATE_ERROR_NODE_OVERLOADS_H(report_unexpected_syntax);
[[noreturn]] void report_unexpected_syntax(const FileSpan&, const std::string&);
[[noreturn]] void report_unexpected_eof(const FileSpan&);
[[noreturn]] void report_expected_different_syntax(const FileSpan&, const std::string&, const std::string&, bool = true);
GENERATE_ERROR_NODE_OVERLOADS_H(report_invalid_assignment);
[[noreturn]] void report_invalid_assignment(const FileSpan&, const std::string&);
GENERATE_ERROR_NODE_OVERLOADS_H(report_invalid_export);
[[noreturn]] void report_invalid_export(const FileSpan&, const std::string&);
GENERATE_ERROR_NODE_OVERLOADS_H(report_invalid_nameof);
[[noreturn]] void report_invalid_nameof(const FileSpan&, const std::string&);
GENERATE_ERROR_NODE_OVERLOADS_H(report_invalid_decorator_target);
[[noreturn]] void report_invalid_decorator_target(const FileSpan&);
GENERATE_ERROR_NODE_OVERLOADS_H(report_duplicate_variable);
[[noreturn]] void report_duplicate_variable(const FileSpan&, const std::string&);
GENERATE_ERROR_NODE_OVERLOADS_H(report_variable_not_found);
[[noreturn]] void report_variable_not_found(const FileSpan&, const std::string&);
GENERATE_ERROR_NODE_OVERLOADS_H(report_variable_read_in_own_initializer);
[[noreturn]] void report_variable_read_in_own_initializer(const FileSpan&);
GENERATE_ERROR_NODE_OVERLOADS_H(report_invalid_break);
[[noreturn]] void report_invalid_break(const FileSpan&);
GENERATE_ERROR_NODE_OVERLOADS_H(report_invalid_continue);
[[noreturn]] void report_invalid_continue(const FileSpan&);
GENERATE_ERROR_NODE_OVERLOADS_H(report_invalid_return);
[[noreturn]] void report_invalid_return(const FileSpan&);
GENERATE_ERROR_NODE_OVERLOADS_H(report_invalid_await);
[[noreturn]] void report_invalid_await(const FileSpan&);
[[noreturn]] void report_duplicate_member(const FileSpan&, const std::string&);
GENERATE_ERROR_NODE_OVERLOADS_H(report_no_variable_type_or_initializer);
[[noreturn]] void report_no_variable_type_or_initializer(const FileSpan&);

GENERATE_NODE_OVERLOADS_H(warn_unreachable_code);
void warn_unreachable_code(const FileSpan&);
GENERATE_NODE_OVERLOADS_H(warn_ambiguous_equals);
void warn_ambiguous_equals(const FileSpan&);

std::string format_diagnostic(const Diagnostic&);
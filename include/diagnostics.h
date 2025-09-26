#pragma once
#include <cstdint>
#include <variant>
#include <string>

#include "token.h"
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

struct MalformedNumber
{
    std::string malformed;
};

struct UnterminatedString
{
    std::string body;
};

struct UnexpectedSyntax
{
    std::string lexeme;
};

struct UnexpectedEOF
{
};

struct ExpectedDifferentSyntax
{
    std::string expected;
    std::string got;
    bool quote_expected;
};

struct InvalidAssignment
{
    std::string lexeme;
};

struct InvalidExport
{
    std::string lexeme;
};

using diagnostic_data_t = std::variant<
    UnexpectedCharacter, 
    MalformedNumber, 
    UnterminatedString, 
    UnexpectedSyntax,
    UnexpectedEOF,
    ExpectedDifferentSyntax,
    InvalidAssignment,
    InvalidExport
>;

struct Diagnostic
{
    uint8_t code;
    DiagnosticSeverity severity;
    FileSpan span;
    diagnostic_data_t  data;
};

[[noreturn]] void report_compiler_error(const std::string&);
[[noreturn]] void report_unexpected_character(const FileSpan&, char);
[[noreturn]] void report_malformed_number(const FileSpan&, const std::string&);
[[noreturn]] void report_unterminated_string(const FileSpan&, const std::string&);
[[noreturn]] void report_unexpected_syntax(const expression_ptr_t&);
[[noreturn]] void report_unexpected_syntax(const statement_ptr_t&);
[[noreturn]] void report_unexpected_syntax(const Token&);
[[noreturn]] void report_unexpected_syntax(const FileSpan&, const std::string&);
[[noreturn]] void report_unexpected_eof(const FileSpan&);
[[noreturn]] void report_expected_different_syntax(const FileSpan&, const std::string&, const std::string&, bool = true);
[[noreturn]] void report_invalid_assignment(const expression_ptr_t&);
[[noreturn]] void report_invalid_assignment(const statement_ptr_t&);
[[noreturn]] void report_invalid_assignment(const Token&);
[[noreturn]] void report_invalid_assignment(const FileSpan&, const std::string&);
[[noreturn]] void report_invalid_export(const expression_ptr_t&);
[[noreturn]] void report_invalid_export(const statement_ptr_t&);
[[noreturn]] void report_invalid_export(const Token&);
[[noreturn]] void report_invalid_export(const FileSpan&, const std::string&);
std::string format_diagnostic(const Diagnostic&);

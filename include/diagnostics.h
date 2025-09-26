#pragma once
#include <cstdint>
#include <variant>
#include <string>

#include "token.h"

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

using diagnostic_data_t = std::variant<
    UnexpectedCharacter, 
    MalformedNumber, 
    UnterminatedString, 
    UnexpectedSyntax,
    UnexpectedEOF,
    ExpectedDifferentSyntax
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
[[noreturn]] void report_unexpected_syntax(const FileSpan&, const std::string&);
[[noreturn]] void report_unexpected_syntax(const Token&);
[[noreturn]] void report_unexpected_eof(const FileSpan&);
[[noreturn]] void report_expected_different_syntax(const FileSpan&, const std::string&, const std::string&, bool = true);
std::string format_diagnostic(const Diagnostic&);

#include <iostream>
#include <iomanip>
#include <complex>
#include <format>
#include <regex>

#include "ion/diagnostics.h"
#include "ion/source_file.h"

[[noreturn]] static void error(const std::string& message, const uint8_t code)
{
    std::cout << message << '\n';
    exit(code);
}

static std::string format_severity(const DiagnosticSeverity severity)
{
    switch (severity)
    {
        case DiagnosticSeverity::Error:
            return "error";
        case DiagnosticSeverity::Warning:
            return "warn";
        case DiagnosticSeverity::Info:
            return "info";
        case DiagnosticSeverity::Debug:
            return "debug";
    }
    
    return "???";
}

[[noreturn]] static void report(const Diagnostic& diagnostic)
{
    error(format_diagnostic(diagnostic), diagnostic.code);
}

[[noreturn]] static void report_error(const uint8_t code, const FileSpan& span, const diagnostic_data_t& data)
{
    report(Diagnostic {
        .code = code,
        .severity = DiagnosticSeverity::Error,
        .span = span,
        .data = data
    });
}

[[noreturn]] void report_compiler_error(const std::string& message)
{
    error("Compiler error: " + message, -1);
}

[[noreturn]] void report_unexpected_character(const FileSpan& span, const char character)
{
    report_error(0001, span, UnexpectedCharacter { character });
}

[[noreturn]] void report_malformed_number(const FileSpan& span, const std::string& malformed)
{
    report_error(0002, span, MalformedNumber { malformed });
}

[[noreturn]] void report_unterminated_string(const FileSpan& span, const std::string& body)
{
    report_error(0002, span, UnterminatedString { body });
}

[[noreturn]] void report_unexpected_syntax(const expression_ptr_t& node)
{
    report_unexpected_syntax(node->get_span(), node->get_text());
}

[[noreturn]] void report_unexpected_syntax(const statement_ptr_t& node)
{
    report_unexpected_syntax(node->get_span(), node->get_text());
}

[[noreturn]] void report_unexpected_syntax(const Token& token)
{
    report_unexpected_syntax(token.span, token.get_text());
}

[[noreturn]] void report_unexpected_syntax(const FileSpan& span, const std::string& lexeme)
{
    report_error(0003, span, UnexpectedSyntax { lexeme });
}

[[noreturn]] void report_unexpected_eof(const FileSpan& span)
{
    report_error(0004, span, UnexpectedEOF {});
}

[[noreturn]] void report_expected_different_syntax(const FileSpan& span, const std::string& expected, const std::string& got, const bool quote_expected)
{
    report_error(0005, span, ExpectedDifferentSyntax { .expected = expected, .got = got, .quote_expected = quote_expected });
}

[[noreturn]] void report_invalid_assignment(const expression_ptr_t& node)
{
    report_invalid_assignment(node->get_span(), node->get_text());
}

[[noreturn]] void report_invalid_assignment(const statement_ptr_t& node)
{
    report_invalid_assignment(node->get_span(), node->get_text());
}

[[noreturn]] void report_invalid_assignment(const Token& token)
{
    report_invalid_assignment(token.span, token.get_text());
}

[[noreturn]] void report_invalid_assignment(const FileSpan& span, const std::string& got)
{
    report_error(0006, span, InvalidAssignment { .lexeme = got });
}

[[noreturn]] void report_invalid_export(const expression_ptr_t& node)
{
    report_invalid_export(node->get_span(), node->get_text());
}

[[noreturn]] void report_invalid_export(const statement_ptr_t& node)
{
    report_invalid_export(node->get_span(), node->get_text());
}

[[noreturn]] void report_invalid_export(const Token& token)
{
    report_invalid_export(token.span, token.get_text());
}

[[noreturn]] void report_invalid_export(const FileSpan& span, const std::string& got)
{
    report_error(0007, span, InvalidExport { .lexeme = got });
}

std::string format_diagnostic(const Diagnostic& diagnostic)
{
    const auto location = format_location(diagnostic.span.end);
    const auto severity = format_severity(diagnostic.severity);
    const auto code = std::format("{:04}", diagnostic.code);
    const auto message = std::visit([&]<typename T>(T& arg) {
        using type_t = std::decay_t<T>;
        if constexpr (std::is_same_v<type_t, UnexpectedCharacter>)
           return "Unexpected character: '" + std::string(1, arg.character) + '\'';
        else if constexpr (std::is_same_v<type_t, MalformedNumber>)
            return "Malformed number: '" + arg.malformed + '\'';
        else if constexpr (std::is_same_v<type_t, UnterminatedString>)
        {
            auto body = static_cast<std::string>(arg.body);
            body.erase(body.find_last_not_of('\n') + 1);
            
            return "Unterminated string: '" + body + '\'';
        }
        else if constexpr (std::is_same_v<type_t, UnexpectedSyntax>)
            return "Unexpected token '" + arg.lexeme + '\'';
        else if constexpr (std::is_same_v<type_t, UnexpectedEOF>)
            return std::string("Unexpected end of file");
        else if constexpr (std::is_same_v<type_t, ExpectedDifferentSyntax>)
        {
            const std::string quote = arg.quote_expected ? "'" : "";
            return "Expected " + quote + arg.expected + quote + ", got '" + arg.got + '\'';
        }
        else if constexpr (std::is_same_v<type_t, InvalidAssignment>)
            return "Cannot assign to '" + arg.lexeme + '\'';
        else if constexpr (std::is_same_v<type_t, InvalidExport>)
            return "Only declarations may be exported, got '" + arg.lexeme + '\'';
    }, diagnostic.data);

    return location + " - " + severity + " ION" + code + ": " + message;
}
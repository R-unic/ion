#include <iostream>
#include <iomanip>
#include <complex>
#include <format>
#include <regex>

#include "ion/diagnostics.h"
#include "ion/source_file.h"
#include "ion/utility/basic.h"

[[noreturn]] static void error(const std::string& message, const uint8_t code)
{
    std::cout << message << '\n';
    exit(code);
}

static void print(const std::string& message)
{
    std::cout << message << '\n';
}

constexpr const char* get_severity_color(const DiagnosticSeverity severity)
{
    switch (severity)
    {
        case DiagnosticSeverity::Error:
            return Color::red;
        case DiagnosticSeverity::Warning:
            return Color::yellow;
        case DiagnosticSeverity::Debug:
            return Color::purple;
        default:
            return Color::light_blue;
    }
}

static std::string format_severity(const DiagnosticSeverity severity)
{
    const auto color_code = std::format("{}{}", Color::bold, get_severity_color(severity));
    switch (severity)
    {
        case DiagnosticSeverity::Error:
            return color("error", color_code.c_str());
        case DiagnosticSeverity::Warning:
            return color("warning", color_code.c_str());
        case DiagnosticSeverity::Info:
            return color("info", color_code.c_str());
        case DiagnosticSeverity::Debug:
            return color("debug", color_code.c_str());
    }

    return "???";
}

static void report_warning(const uint8_t code, const FileSpan& span, diagnostic_data_t data)
{
    const Diagnostic diagnostic { code, DiagnosticSeverity::Warning, span, std::move(data) };
    const auto message = format_diagnostic(diagnostic);
    print(message);
}

[[noreturn]] static void report_error(const uint8_t code, const FileSpan& span, diagnostic_data_t data)
{
    const Diagnostic diagnostic { code, DiagnosticSeverity::Error, span, std::move(data) };
    const auto message = format_diagnostic(diagnostic);
    error(message, diagnostic.code);
}

[[noreturn]] void report_compiler_error(const std::string& message)
{
    error("Compiler error: " + message, -1);
}

[[noreturn]] void report_unexpected_character(const FileSpan& span, const char character)
{
    report_error(1, span, UnexpectedCharacter { character });
}

[[noreturn]] void report_malformed_number(const FileSpan& span, const std::string& malformed)
{
    report_error(2, span, MalformedNumber { malformed });
}

[[noreturn]] void report_unterminated_string(const FileSpan& span, const std::string& body)
{
    report_error(3, span, UnterminatedString { body });
}

GENERATE_ERROR_NODE_OVERLOADS_WITH_TEXT(report_unexpected_syntax);

[[noreturn]] void report_unexpected_syntax(const FileSpan& span, const std::string& lexeme)
{
    report_error(4, span, UnexpectedSyntax { lexeme });
}

[[noreturn]] void report_unexpected_eof(const FileSpan& span)
{
    report_error(5, span, UnexpectedEOF {});
}

[[noreturn]] void report_expected_different_syntax(const FileSpan& span, const std::string& expected,
                                                   const std::string& got, const bool quote_expected)
{
    report_error(6, span, ExpectedDifferentSyntax { .expected = expected, .got = got, .quote_expected = quote_expected });
}

GENERATE_ERROR_NODE_OVERLOADS_WITH_TEXT(report_invalid_assignment);

[[noreturn]] void report_invalid_assignment(const FileSpan& span, const std::string& got)
{
    report_error(7, span, InvalidAssignment { .lexeme = got });
}

GENERATE_ERROR_NODE_OVERLOADS_WITH_TEXT(report_invalid_export);

[[noreturn]] void report_invalid_export(const FileSpan& span, const std::string& got)
{
    report_error(8, span, InvalidExport { .lexeme = got });
}

GENERATE_ERROR_NODE_OVERLOADS_WITH_TEXT(report_invalid_nameof);

[[noreturn]] void report_invalid_nameof(const FileSpan& span, const std::string& got)
{
    report_error(9, span, InvalidNameOf { .lexeme = got });
}

GENERATE_ERROR_NODE_OVERLOADS(report_invalid_decorator_target);

[[noreturn]] void report_invalid_decorator_target(const FileSpan& span)
{
    report_error(10, span, InvalidDecoratorTarget {});
}

GENERATE_ERROR_NODE_OVERLOADS_WITH_TEXT(report_duplicate_variable);

void report_duplicate_variable(const FileSpan& span, const std::string& name)
{
    report_error(11, span, DuplicateVariable { .name = name });
}

GENERATE_ERROR_NODE_OVERLOADS_WITH_TEXT(report_variable_not_found);

void report_variable_not_found(const FileSpan& span, const std::string& name)
{
    report_error(12, span, VariableNotFound { .name = name });
}

GENERATE_NODE_OVERLOADS(report_variable_read_in_own_initializer);

void report_variable_read_in_own_initializer(const FileSpan& span)
{
    report_error(13, span, VariableReadInOwnInitializer {});
}

GENERATE_NODE_OVERLOADS(report_invalid_break);

void report_invalid_break(const FileSpan& span)
{
    report_error(14, span, InvalidBreak {});
}

GENERATE_NODE_OVERLOADS(report_invalid_continue);

void report_invalid_continue(const FileSpan& span)
{
    report_error(15, span, InvalidContinue {});
}

GENERATE_NODE_OVERLOADS(report_invalid_return);

void report_invalid_return(const FileSpan& span)
{
    report_error(16, span, InvalidReturn {});
}

GENERATE_NODE_OVERLOADS(report_invalid_await);

void report_invalid_await(const FileSpan& span)
{
    report_error(18, span, InvalidAwait {});
}

GENERATE_NODE_OVERLOADS(warn_unreachable_code);

void warn_unreachable_code(const FileSpan& span)
{
    report_warning(100, span, UnreachableCode {});
}

GENERATE_NODE_OVERLOADS(warn_ambiguous_equals);

void warn_ambiguous_equals(const FileSpan& span)
{
    report_warning(101, span, AmbiguousEquals {});
}

constexpr std::string get_diagnostic_message(const Diagnostic& diagnostic)
{
    return std::visit([&]<typename T>(T& arg)
    {
        using type_t = std::decay_t<T>;
        if constexpr (std::is_same_v<type_t, UnexpectedCharacter>)
            return "Unexpected character: '" + std::string(1, arg.character) + "'";
        else if constexpr (std::is_same_v<type_t, MalformedNumber>)
            return "Malformed number: '" + arg.malformed + "'";
        else if constexpr (std::is_same_v<type_t, UnterminatedString>)
        {
            auto body = static_cast<std::string>(arg.body);
            body.erase(body.find_last_not_of('\n') + 1);

            return "Unterminated string: '" + body + "'";
        }
        else if constexpr (std::is_same_v<type_t, UnexpectedSyntax>)
            return "Unexpected token '" + arg.lexeme + "'";
        else if constexpr (std::is_same_v<type_t, UnexpectedEOF>)
            return std::string("Unexpected end of file.");
        else if constexpr (std::is_same_v<type_t, ExpectedDifferentSyntax>)
        {
            const std::string quote = arg.quote_expected ? "'" : "";
            return "Expected " + quote + arg.expected + quote + ", got '" + arg.got + "'.";
        }
        else if constexpr (std::is_same_v<type_t, InvalidAssignment>)
            return "Cannot assign to '" + arg.lexeme + "'.";
        else if constexpr (std::is_same_v<type_t, InvalidExport>)
            return "Only declarations may be exported, got '" + arg.lexeme + "'.";
        else if constexpr (std::is_same_v<type_t, InvalidNameOf>)
            return "Cannot get name of '" + arg.lexeme + "', only identifiers and member accesses.";
        else if constexpr (std::is_same_v<type_t, InvalidDecoratorTarget>)
            return std::string("Invalid decorator target. Decorators may only be used on functions.");
        else if constexpr (std::is_same_v<type_t, DuplicateVariable>)
            return "Name '" + arg.name + "' is already declared in this scope.";
        else if constexpr (std::is_same_v<type_t, VariableNotFound>)
            return "Cannot find name '" + arg.name + "'";
        else if constexpr (std::is_same_v<type_t, VariableReadInOwnInitializer>)
            return std::string("Cannot read variable in it's own initializer.");
        else if constexpr (std::is_same_v<type_t, InvalidBreak>)
            return std::string("Cannot 'break' outside of a loop.");
        else if constexpr (std::is_same_v<type_t, InvalidContinue>)
            return std::string("Cannot 'continue' outside of a loop.");
        else if constexpr (std::is_same_v<type_t, InvalidReturn>)
            return std::string("Cannot 'return' outside of a function.");
        else if constexpr (std::is_same_v<type_t, InvalidAwait>)
            return std::string("Cannot 'await' outside of an async function.");
        else if constexpr (std::is_same_v<type_t, UnreachableCode>)
            return std::string("Unreachable code.");
        else if constexpr (std::is_same_v<type_t, AmbiguousEquals>)
            return std::string(
                "Equals operator is ambiguous here -- are you trying to compare or assign? Move to a different line to silence.");
    }, diagnostic.data);
}

std::string format_diagnostic(const Diagnostic& diagnostic)
{
    const auto location = format_location(diagnostic.span.end, true, true);
    const auto severity = format_severity(diagnostic.severity);
    const auto code = color(std::format("ION{:04}: ", diagnostic.code), Color::gray);
    const auto message = get_diagnostic_message(diagnostic);
    const auto line_text = diagnostic.span.get_line();
    const auto span_text = diagnostic.span.get_text();
    const auto start_column = diagnostic.span.start.column;
    const auto underline_size = std::max<size_t>(1, diagnostic.span.end.column - start_column);
    const std::string underline_indent(start_column + 1, ' ');
    const auto underline_color = std::format("{}{}", Color::bold, get_severity_color(diagnostic.severity));
    const auto underline = underline_indent + color(std::string(underline_size, '~'), underline_color.c_str());

    const auto line_number = std::to_string(diagnostic.span.start.line);
    const auto gutter_width = line_number.size();
    const auto line_number_color = std::format("{}{}", Color::background_white, Color::black);
    const auto colored_line_number = color(gutter_width > 1 ? line_number : line_number + " ", line_number_color.c_str());

    const auto line_info = colored_line_number + "    " + line_text + '\n' + std::string(gutter_width + 4, ' ') + underline;
    return location + " - " + severity + " " + code + message + "\n\n" + line_info + '\n';
}
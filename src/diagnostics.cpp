#include <iostream>
#include <iomanip>
#include <complex>
#include <format>

#include "diagnostics.h"

#include <regex>


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

[[noreturn]] void report_compiler_error(const std::string& message)
{
    error("Compiler error: " + message, -1);
}

[[noreturn]] void report_unexpected_character(const FileLocation& location, const char character)
{
    report(Diagnostic {
        .code     = 0001,
        .severity = DiagnosticSeverity::Error,
        .location = location,
        .data     = UnexpectedCharacter { character }
    });
}

std::string format_diagnostic(const Diagnostic& diagnostic)
{
    const auto location = format_location(diagnostic.location);
    const auto severity = format_severity(diagnostic.severity);
    const auto code = std::format("{:04}", diagnostic.code);
    const auto message = std::visit([&]<typename T>(T& arg) {
        using type_t = std::decay_t<T>;
        if constexpr (std::is_same_v<type_t, UnexpectedCharacter>)
           return "Unexpected character: '" + std::string(1, arg.character) + '\'';
        
        report_compiler_error("Unhandled diagnostic type: " + std::to_string(diagnostic.data.index()));
    }, diagnostic.data);

    return location + " - " + severity + " ION" + code + ": " + message;
}
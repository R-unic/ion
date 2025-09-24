#pragma once
#include <cstdint>
#include <variant>
#include <string>

#include "file_location.h"

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

using diagnostic_data_t = std::variant<UnexpectedCharacter, MalformedNumber>;
struct Diagnostic
{
    uint8_t code;
    DiagnosticSeverity severity;
    FileLocation location;
    diagnostic_data_t data;
};

[[noreturn]] void report_compiler_error(const std::string&);
[[noreturn]] void report_unexpected_character(const FileLocation&, char);
[[noreturn]] void report_malformed_number(const FileLocation&, const std::string&);
std::string format_diagnostic(const Diagnostic&);

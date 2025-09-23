#include <fstream>
#include <string>

#include "diagnostics.h"
#include "ast/source_file.h"

static std::string read_file(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        report_compiler_error("Failed to open file at: " + path);

    std::string line;
    std::string contents;
    while (std::getline(file, line))
        contents += line + '\n';

    file.close();
    return contents;
}

SourceFile create_file(const std::string& path)
{
    const auto text = read_file(path);
    return SourceFile { .path = path, .text = text };
}
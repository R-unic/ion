#include <fstream>
#include <string>

#include "ion/diagnostics.h"
#include "ion/source_file.h"

#include "ion/logger.h"

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

SourceFile* create_file(const std::string& path)
{
    const auto text = read_file(path);
    logger::info("Created source file from path: " + path);

    return new SourceFile(path, text);
}

std::string format_location(const FileLocation& location, const bool include_file_path)
{
    auto numeric_location = std::to_string(location.line) + ':' + std::to_string(location.column);
    if (include_file_path)
        return location.file->path + ':' + numeric_location;

    return numeric_location;
}
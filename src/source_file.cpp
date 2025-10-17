#include <fstream>
#include <string>

#include "ion/diagnostics.h"
#include "ion/source_file.h"

#include "ion/logger.h"
#include "ion/utility/basic.h"

static std::string read_file(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        logger::error("Failed to open file at: " + path);

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
    logger::info("Created source file from path: " + path);

    return SourceFile(path, text);
}

std::string format_location(const FileLocation& location, const bool include_file_path, const bool colors)
{
    const auto line_text = colors ? color(std::to_string(location.line), Color::yellow) : std::to_string(location.line);
    const auto column_text = colors ? color(std::to_string(location.column), Color::yellow) : std::to_string(location.column);
    auto numeric_location = line_text + ':' + column_text;
    if (include_file_path)
    {
        const auto file_path_text = colors ? color(location.file->path, Color::light_blue) : location.file->path;
        return file_path_text + ':' + numeric_location;
    }

    return numeric_location;
}
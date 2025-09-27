#include "ion/file_location.h"

#include <sstream>

#include "ion/source_file.h"

std::string FileSpan::get_text() const
{
    const auto start_position = start.position;
    return start.file->text.substr(start_position, end.position - start_position);
}

std::string FileSpan::get_line() const
{
    const auto file_text = start.file->text;
    const auto line_index = start.line - 1;
    std::istringstream stream(file_text);
    std::string line;

    auto current_line_index = 0;
    while (std::getline(stream, line))
        if (current_line_index++ == line_index)
            return line;

    return "";
}

FileLocation get_start_location(const SourceFile* file)
{
    return FileLocation { .file = file };
}

FileSpan create_span(const FileLocation& start, const FileLocation& end)
{
    return FileSpan { .start = start, .end = end };
}
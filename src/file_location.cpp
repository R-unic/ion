#include "file_location.h"

std::string format_location(const FileLocation& location, const bool include_file_path)
{
    auto numeric_location = std::to_string(location.column) + ':' + std::to_string(location.line);
    if (include_file_path)
        return location.file.path + ':' + numeric_location;
    
    return numeric_location;
}

FileLocation get_start_location(const SourceFile& file)
{
    return FileLocation {
        .position = 0,
        .line = 1,
        .column = 0,
        .file = file
    };
}
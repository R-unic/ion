#pragma once
#include <string>

#include "ast/source_file.h"

struct FileLocation
{
    int position;
    int line;
    int column;
    SourceFile file;
};

struct FileSpan
{
    FileLocation start;
    FileLocation end;
};

std::string format_location(const FileLocation&, bool = true);
FileLocation get_start_location(const SourceFile&);
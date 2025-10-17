#pragma once
#include <string>

struct Token;
struct SourceFile;

struct FileLocation
{
    int position = 0;
    int line = 1;
    int column = 0;
    const SourceFile* file {};

    FileLocation operator+(const FileLocation other) const
    {
        return FileLocation {
            .position = position + other.position,
            .line = line,
            .column = column + other.column,
            .file = file
        };
    }

    FileLocation operator+(const int offset) const
    {
        return FileLocation {
            .position = position + offset,
            .line = line,
            .column = column + offset,
            .file = file
        };
    }

    FileLocation operator-(const FileLocation other) const
    {
        return FileLocation {
            .position = position - other.position,
            .line = line,
            .column = column - other.column,
            .file = file
        };
    }

    FileLocation operator-(const int offset) const
    {
        return FileLocation {
            .position = position - offset,
            .line = line,
            .column = column - offset,
            .file = file
        };
    }

    bool operator<(const FileLocation other) const
    {
        return position < other.position;
    }

    bool operator>(const FileLocation other) const
    {
        return position < other.position;
    }

    bool operator==(const FileLocation& other) const
    {
        return eq(other);
    }

private:
    bool eq(const FileLocation& other) const
    {
        return position == other.position && line == other.line && column == other.column; // ignore source file for now
    }
};

struct FileSpan
{
    FileLocation start;
    FileLocation end;

    [[nodiscard]] std::string get_text() const;
    [[nodiscard]] std::string get_line() const;
    [[nodiscard]] bool has_line_break_between(const Token& other) const;
    [[nodiscard]] bool has_line_break_between(const FileSpan& other) const;

    bool operator==(const FileSpan& other) const
    {
        return start == other.start && end == other.end;
    }
};

FileLocation get_start_location(const SourceFile&);
FileSpan create_span(const FileLocation&, const FileLocation&);
#pragma once
#include <string>
#include <vector>

#include "ast/node.h"

struct SourceFile
{
    std::string path, text;
    std::vector<statement_ptr_t> statements;

    SourceFile(std::string path, std::string text, std::vector<statement_ptr_t> statements = {})
        : path(std::move(path)),
          text(std::move(text)),
          statements(std::move(statements))
    {
    }

    ~SourceFile() = default;
    SourceFile(SourceFile&&) noexcept = default;
    SourceFile& operator=(SourceFile&&) noexcept = default;
    SourceFile(const SourceFile&) = delete;
    SourceFile& operator=(const SourceFile&) = delete;
};

SourceFile create_file(const std::string&);
std::string format_location(const FileLocation&, bool = true, bool = false);
#pragma once
#include <string>

#include "syntax/token_stream.h"
#include "ast/abstract/expression.h"

class source_file
{
public:
    std::string path;
    std::string source;

    explicit source_file(const std::string& file_path);

    [[nodiscard]] token_stream tokenize() const;
    [[nodiscard]] expression parse() const;
};

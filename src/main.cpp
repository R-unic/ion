#include <iostream>

#include "token.h"

int main(int argc, char* argv[])
{
    const auto file = SourceFile { .path = "my-file.ion", .text = "69 + 420" };
    constexpr auto span = Span {
        .start = Location { .position = 3, .line = 1, .column = 3 },
        .end = Location { .position = 4, .line = 1, .column = 4 }
    };

    const auto token = Token { .kind = SyntaxKind::Plus, .span = span, .file = file };
    std::cout << get_text(token) << '\n';
    
    return 0;
}

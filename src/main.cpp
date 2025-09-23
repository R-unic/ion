#include "diagnostics.h"
#include "token.h"

int main(int argc, char* argv[])
{
    const auto file = create_file("test.ion");
    const auto start = get_start_location(file);
    report_unexpected_character(start, '&');
    
    return 0;
}

#pragma once

struct Location
{
    int position;
    int line;
    int column;
};

struct Span
{
    Location start;
    Location end;
};
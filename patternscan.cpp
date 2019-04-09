// clang-format off
#include "headers.h"
#include "main.h"
#include "patternscan.h"
// clang-format on

unsigned char hex_digit_to_byte(char c)
{
    if (c >= '0' && c <= '9')
    {
        c -= '0';
    }
    else if (c >= 'A' && c <= 'F')
    {
        c -= 'A';
        c += 10;
    }
    else if (c >= 'a' && c <= 'f')
    {
        c -= 'a';
        c += 10;
    }
    else
    {
        c = 0xFF;
    }

    return c;
}

int hex_char_to_byte(char c1, char c2)
{
    c1 = hex_digit_to_byte(c1);
    c2 = hex_digit_to_byte(c2);

    if ((unsigned char)c1 == 0xFF || (unsigned char)c2 == 0xFF)
        return 0x100; // Wrong char.

    return ((int)c2 + ((int)c1 * 0x10));
}

void swap_endian(unsigned char* addr, size_t len)
{
    size_t i;
    unsigned char backup_byte;

    for (i = 0; i < len / 2; i++)
    {
        backup_byte = addr[i];
        addr[i] = addr[(len - 1) - i];
        addr[(len - 1) - i] = backup_byte;
    }
}
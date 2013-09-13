// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 2011-06-12 19:45:35
// Description:

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "common/base/stdint.h"

#ifdef __unix__
#include <unistd.h>
#endif

class FilePrinter
{
public:
    explicit FilePrinter(FILE* fp) : m_fp(fp) {}
    void Print(const char* s)
    {
        fprintf(m_fp, "%s", s);
    }
private:
    FILE* m_fp;
};

class FdPrinter
{
public:
    explicit FdPrinter(int fd) : m_fd(fd) {}
    void Print(const char* s)
    {
#ifdef __unix__
        write(m_fd, s, strlen(s));
#endif
#ifdef _MSC_VER
        _write(m_fd, s, strlen(s));
#endif
    }
private:
    int m_fd;
};

template <typename PrinterType>
void HexDumpTemplate(
    PrinterType* printer,
    const void* buffer,
    size_t size,
    bool print_address
)
{
    const int kAddressWidth = 6;   // '0000 ', 4 characters and 1 space
    const int kBytesPerLine = 16;
    const int kWidthPerByte = 3;    // 'F0 ', 2 characters and 1 space

    static const char kHexAlphabet[] = "0123456789ABCDEF";
    const unsigned char* data = (const unsigned char*) buffer;

    uint32_t address = 0;
    if (print_address)
    {
        printf("Starting address: %p\n", buffer);
        address = (uint32_t)(uintptr_t) buffer;
    }

    size_t char_start_pos =
        kAddressWidth +
        kBytesPerLine * kWidthPerByte + 1;

    for (size_t i = 0; i < size; i += kBytesPerLine)
    {
        // DebugView display each DbgPrint output in one line,
        // so I gather them in one line
        char line[
            kAddressWidth +  // Address width
            kBytesPerLine * (kWidthPerByte + 1)  // contents
            + 3  // tail NULL
        ];

        uint32_t a = address + i;
        // only print the low 2 byte of address
        line[0] = kHexAlphabet[(a>>12) & 0x0F];
        line[1] = kHexAlphabet[(a>>8) & 0x0F];
        line[2] = kHexAlphabet[(a>>4) & 0x0F];
        line[3] = kHexAlphabet[(a) & 0x0F];
        line[4] = ':';
        line[5] = ' ';

        // print the hex-char delimiter
        line[char_start_pos - 1] = ' ';

        for (int j = 0; j < kBytesPerLine; ++j)
        {
            size_t hex_pos = kAddressWidth + j * kWidthPerByte;
            size_t char_pos = char_start_pos + j;

            if (i + j < size)
            {
                unsigned char c = data[i + j];
                line[hex_pos + 0] = kHexAlphabet[c >> 4];
                line[hex_pos + 1] = kHexAlphabet[c & 0x0F];
                line[hex_pos + 2] = ' ';
                line[char_pos] = isprint(c) ? c : '.';

                // print middle indicator
                if (j == 8)
                    line[hex_pos - 1] = '-';
            }
            else
            {
                line[hex_pos + 0] = ' ';
                line[hex_pos + 1] = ' ';
                line[hex_pos + 2] = ' ';
                line[char_pos] = ' ';
            }
        }

        size_t end_pos = kAddressWidth + kBytesPerLine * (kWidthPerByte + 1) + 1;
        line[end_pos] = '\n';

        line[sizeof(line)-1] = 0;
        printer->Print(line);
    }
}

void HexDump(FILE* fp, const void* buffer, size_t size, bool print_address)
{
    FilePrinter printer(fp);
    HexDumpTemplate(&printer, buffer, size, print_address);
}

void HexDump(int fd, const void* buffer, size_t size, bool print_address)
{
    FdPrinter printer(fd);
    HexDumpTemplate(&printer, buffer, size, print_address);
}


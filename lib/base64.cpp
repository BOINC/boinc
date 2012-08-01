// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#include "base64.h"

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#else
#include "config.h"
#endif

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

using std::string;

// Table of characters coding the 64 values.
static char base64_value_to_char[64] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',   //  0- 9
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',   // 10-19
    'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',   // 20-29
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',   // 30-39
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',   // 40-49
    'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',   // 50-59
    '8', '9', '+', '/'                                  // 60-63
};

// Table of base64 values for first 128 characters.
static short base64_char_to_value[128] =
{
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,    //   0-  9
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,    //  10- 19
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,    //  20- 29
    -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,    //  30- 39
    -1,  -1,  -1,  62,  -1,  -1,  -1,  63,  52,  53,    //  40- 49
    54,  55,  56,  57,  58,  59,  60,  61,  -1,  -1,    //  50- 59
    -1,  -1,  -1,  -1,  -1,  0,   1,   2,   3,   4,     //  60- 69
    5,   6,   7,   8,   9,   10,  11,  12,  13,  14,    //  70- 79
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,    //  80- 89
    25,  -1,  -1,  -1,  -1,  -1,  -1,  26,  27,  28,    //  90- 99
    29,  30,  31,  32,  33,  34,  35,  36,  37,  38,    // 100-109
    39,  40,  41,  42,  43,  44,  45,  46,  47,  48,    // 110-119
    49,  50,  51,  -1,  -1,  -1,  -1,  -1               // 120-127
};

/* The following diagram shows the logical steps by which three octets
   get transformed into four base64 characters.

		 .--------.  .--------.  .--------.
		 |aaaaaabb|  |bbbbcccc|  |ccdddddd|
		 `--------'  `--------'  `--------'
                    6   2      4   4       2   6
	       .--------+--------+--------+--------.
	       |00aaaaaa|00bbbbbb|00cccccc|00dddddd|
	       `--------+--------+--------+--------'

	       .--------+--------+--------+--------.
	       |AAAAAAAA|BBBBBBBB|CCCCCCCC|DDDDDDDD|
	       `--------+--------+--------+--------'

   The octets are divided into 6 bit chunks, which are then encoded into
   base64 characters.  */

string r_base64_encode (const char* from, size_t length)
{
    string result;
    result.reserve(length + length/3 + 1);
    size_t i = 0;
    int c;
    unsigned int value;

    while (i < length)
    {
        c = from[i++];

        // Process first byte of a triplet.
        result += base64_value_to_char[0x3f & c >> 2];
        value = (0x03 & c) << 4;

        // Process second byte of a triplet.
        if (i == length)
	{
            result += base64_value_to_char[value];
            result += '=';
            result += '=';
            break;
	}

        c = from[i++];

        result += base64_value_to_char[value | (0x0f & c >> 4)];
        value = (0x0f & c) << 2;

        // Process third byte of a triplet.
        if (i == length)
	{
            result += base64_value_to_char[value];
            result += '=';
            break;
	}

        c = from[i++];

        result += base64_value_to_char[value | (0x03 & c >> 6)];
        result += base64_value_to_char[0x3f & c];
    }

    return result;
}

#define IS_ASCII(Character) \
  ((Character) < 128)
#define IS_BASE64(Character) \
  (IS_ASCII (Character) && base64_char_to_value[Character] >= 0)
#define IS_BASE64_IGNORABLE(Character) \
  ((Character) == ' ' || (Character) == '\t' || (Character) == '\n' \
   || (Character) == '\f' || (Character) == '\r')

#define READ_QUADRUPLET_BYTE()                                    \
    do {                                                          \
        if (i == length) return result;                           \
        c = from[i++];                                            \
    }                                                             \
    while (IS_BASE64_IGNORABLE (c))

string r_base64_decode (const char* from, size_t length)
{
    size_t i = 0;
    string result;
    unsigned char c;
    unsigned long value;

    while (true)
    {
        // Process first byte of a quadruplet.

        READ_QUADRUPLET_BYTE();

        if (!IS_BASE64 (c))
            throw InvalidBase64Exception();
        value = base64_char_to_value[c] << 18;

        // Process second byte of a quadruplet.

        READ_QUADRUPLET_BYTE();

        if (!IS_BASE64 (c))
            throw InvalidBase64Exception();
        value |= base64_char_to_value[c] << 12;

        c = (unsigned char) (value >> 16);
        result += c;

        // Process third byte of a quadruplet.

        READ_QUADRUPLET_BYTE();
        if (c == '=')
	{
            READ_QUADRUPLET_BYTE();

            if (c != '=')
                throw InvalidBase64Exception();
            continue;
	}

        if (!IS_BASE64 (c))
            throw InvalidBase64Exception();
        value |= base64_char_to_value[c] << 6;

        c = (unsigned char) (0xff & value >> 8);
        result += c;

        // Process fourth byte of a quadruplet.

        READ_QUADRUPLET_BYTE();

        if (c == '=')
            continue;

        if (!IS_BASE64 (c))
            throw InvalidBase64Exception();
        value |= base64_char_to_value[c];

        c = (unsigned char) (0xff & value);
        result += c;
    }
}


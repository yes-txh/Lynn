#ifndef __unix__

#include "common/base/compatible/time.h"
#include <stddef.h>
#include <ctype.h>
#include "common/base/compatible/string.h"

// Implement strptime under windows
static const char* kWeekFull[] =
{
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
};

static const char* kWeekAbbr[] =
{
    "Sun", "Mon", "Tue", "Wed",
    "Thu", "Fri", "Sat"
};

static const char* kMonthFull[] =
{
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

static const char* kMonthAbbr[] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static const char* parse_num(const char* s, int low, int high, int* value)
{
    const char* p = s;
    for (*value = 0; *p != '\0' && isdigit(*p); ++p)
    {
        *value = (*value) * 10 + static_cast<int>(*p) - static_cast<int>('0');
    }

    if (p == s || *value < low || *value > high) return NULL;
    return p;
}

char* strptime(const char *s, const char *format, struct tm *tm)
{
    while (*format != '\0' && *s != '\0')
    {
        if (*format != '%')
        {
            if (*s != *format)
                return NULL;

            ++format;
            ++s;
            continue;
        }

        ++format;
        int len = 0;
        switch (*format)
        {
        // weekday name.
        case 'a':
        case 'A':
            tm->tm_wday = -1;
            for (int i = 0; i < 7; ++i)
            {
                len = static_cast<int>(strlen(kWeekAbbr[i]));
                if (strncasecmp(kWeekAbbr[i], s, len) == 0)
                {
                    tm->tm_wday = i;
                    break;
                }

                len = static_cast<int>(strlen(kWeekFull[i]));
                if (strncasecmp(kWeekFull[i], s, len) == 0)
                {
                    tm->tm_wday = i;
                    break;
                }
            }
            if (tm->tm_wday == -1)
                return NULL;
            s += len;
            break;

        // month name.
        case 'b':
        case 'B':
        case 'h':
            tm->tm_mon = -1;
            for (int i = 0; i < 12; ++i)
            {
                len = static_cast<int>(strlen(kMonthAbbr[i]));
                if (strncasecmp(kMonthAbbr[i], s, len) == 0)
                {
                    tm->tm_mon = i;
                    break;
                }

                len = static_cast<int>(strlen(kMonthFull[i]));
                if (strncasecmp(kMonthFull[i], s, len) == 0)
                {
                    tm->tm_mon = i;
                    break;
                }
            }
            if (tm->tm_mon == -1)
                return NULL;
            s += len;
            break;

            // month [1, 12].
        case 'm':
            s = parse_num(s, 1, 12, &tm->tm_mon);
            if (s == NULL) return NULL;
            --tm->tm_mon;
            break;

            // day [1, 31].
        case 'd':
        case 'e':
            s = parse_num(s, 1, 31, &tm->tm_mday);
            if (s == NULL) return NULL;
            break;

            // hour [0, 23].
        case 'H':
            s = parse_num(s, 0, 23, &tm->tm_hour);
            if (s == NULL) return NULL;
            break;

            // minute [0, 59]
        case 'M':
            s = parse_num(s, 0, 59, &tm->tm_min);
            if (s == NULL) return NULL;
            break;

            // seconds [0, 60]. 60 is for leap year.
        case 'S':
            s = parse_num(s, 0, 60, &tm->tm_sec);
            if (s == NULL) return NULL;
            break;

            // year [1900, 9999].
        case 'Y':
            s = parse_num(s, 1900, 9999, &tm->tm_year);
            if (s == NULL) return NULL;
            tm->tm_year -= 1900;
            break;

            // year [0, 99].
        case 'y':
            s = parse_num(s, 0, 99, &tm->tm_year);
            if (s == NULL) return NULL;
            if (tm->tm_year <= 68) {
                tm->tm_year += 100;
            }
            break;

            // arbitray whitespace.
        case 't':
        case 'n':
            while (isspace(*s)) ++s;
            break;

            // '%'.
        case '%':
            if (*s != '%') return NULL;
            ++s;
            break;

            // All the other format are not supported.
        default:
            return NULL;
        }
        ++format;
    }

    if (*format != '\0')
    {
        return NULL;
    }
    else
    {
        return const_cast<char*>(s);
    }
}
#endif // __unix__

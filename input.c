#include "input.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int input_line(const char *prompt, char *buffer, size_t capacity)
{
    size_t length;
    int ch;

    if (capacity < 2) {
        fputs("Internal input buffer error.\n", stderr);
        return 0;
    }
    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buffer, (int)capacity, stdin) == NULL) {
        return 0;
    }
    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[--length] = '\0';
    } else if (!feof(stdin)) {
        ch = getchar();
        if (ch != '\n' && ch != EOF) {
            while ((ch = getchar()) != '\n' && ch != EOF) {
            }
            puts("Input is too long; this entry was cancelled.");
            return 0;
        }
    }
    if (length > 0 && buffer[length - 1] == '\r') {
        buffer[length - 1] = '\0';
    }
    return 1;
}

int input_int(const char *prompt, int *value)
{
    char buffer[64];
    char *end;
    long parsed;

    if (!input_line(prompt, buffer, sizeof(buffer))) {
        return 0;
    }
    while (isspace((unsigned char)*buffer)) {
        memmove(buffer, buffer + 1, strlen(buffer));
    }
    if (buffer[0] == '\0') {
        puts("Please enter a whole number.");
        return 0;
    }
    errno = 0;
    parsed = strtol(buffer, &end, 10);
    while (isspace((unsigned char)*end)) {
        ++end;
    }
    if (*end != '\0' || end == buffer || errno == ERANGE ||
        parsed < 1 || parsed > INT_MAX) {
        puts("Please enter a positive whole number.");
        return 0;
    }
    *value = (int)parsed;
    return 1;
}

static int leap_year(int year)
{
    return year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
}

static int valid_date(const char *date)
{
    int year;
    int month;
    int day;
    int days_in_month;

    if (strlen(date) != 10 || date[4] != '-' || date[7] != '-') {
        return 0;
    }
    for (int i = 0; i < 10; ++i) {
        if (i != 4 && i != 7 && !isdigit((unsigned char)date[i])) {
            return 0;
        }
    }
    year = (date[0] - '0') * 1000 + (date[1] - '0') * 100 +
           (date[2] - '0') * 10 + date[3] - '0';
    month = (date[5] - '0') * 10 + date[6] - '0';
    day = (date[8] - '0') * 10 + date[9] - '0';
    if (year < 1900 || month < 1 || month > 12) {
        return 0;
    }
    days_in_month = month == 2 ? 28 + leap_year(year) :
                    (month == 4 || month == 6 || month == 9 || month == 11
                         ? 30
                         : 31);
    return day >= 1 && day <= days_in_month;
}

int input_date(const char *prompt, char date[11])
{
    char buffer[64];
    if (!input_line(prompt, buffer, sizeof(buffer))) {
        return 0;
    }
    if (!valid_date(buffer)) {
        puts("Enter a valid calendar date in YYYY-MM-DD format.");
        return 0;
    }
    memcpy(date, buffer, 11);
    return 1;
}

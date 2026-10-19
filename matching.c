#include "matching.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

enum {
    CATEGORY_POINTS = 40,
    LOCATION_POINTS = 25,
    KEYWORD_POINTS = 10,
    MAX_KEYWORD_POINTS = 30,
    DATE_POINTS = 5,
    MAX_MATCH_DAYS = 7,
    MIN_MATCH_SCORE = 20
};

static int is_stop_word(const char *word)
{
    static const char *const stop_words[] = {
        "a", "an", "and", "at", "by", "for", "from", "in", "is", "it",
        "of", "on", "or", "the", "to", "with"
    };
    size_t count = sizeof(stop_words) / sizeof(stop_words[0]);

    for (size_t i = 0; i < count; ++i) {
        if (strcmp(word, stop_words[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int next_token(const char **cursor, char token[256])
{
    size_t length = 0;
    const unsigned char *text = (const unsigned char *)*cursor;

    while (*text != '\0' && !isalnum(*text)) {
        ++text;
    }
    if (*text == '\0') {
        *cursor = (const char *)text;
        return 0;
    }
    while (*text != '\0' && isalnum(*text)) {
        if (length < 255) {
            token[length++] = (char)tolower(*text);
        }
        ++text;
    }
    token[length] = '\0';
    *cursor = (const char *)text;
    return 1;
}

static int token_in_text(const char *text, const char *wanted)
{
    const char *cursor = text;
    char token[256];

    while (next_token(&cursor, token)) {
        if (strcmp(token, wanted) == 0) {
            return 1;
        }
    }
    return 0;
}

static int shared_keywords(const char *first, const char *second)
{
    const char *cursor = first;
    char token[256];
    char seen[64][256];
    int seen_count = 0;
    int shared = 0;

    while (next_token(&cursor, token)) {
        int duplicate = 0;

        if (strlen(token) < 3 || is_stop_word(token)) {
            continue;
        }
        for (int i = 0; i < seen_count; ++i) {
            if (strcmp(seen[i], token) == 0) {
                duplicate = 1;
                break;
            }
        }
        if (duplicate) {
            continue;
        }
        if (seen_count < 64) {
            memcpy(seen[seen_count++], token, strlen(token) + 1);
        }
        if (token_in_text(second, token)) {
            ++shared;
        }
    }
    return shared;
}

static long date_ordinal(const char *date)
{
    static const int month_days[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    int year = (date[0] - '0') * 1000 + (date[1] - '0') * 100 +
               (date[2] - '0') * 10 + date[3] - '0';
    int month = (date[5] - '0') * 10 + date[6] - '0';
    int day = (date[8] - '0') * 10 + date[9] - '0';
    long prior_years = year - 1;
    long days = prior_years * 365 + prior_years / 4 - prior_years / 100 +
                prior_years / 400 + day;

    for (int current_month = 1; current_month < month; ++current_month) {
        days += month_days[current_month - 1];
        if (current_month == 2 &&
            (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0))) {
            ++days;
        }
    }
    return days;
}

static int categories_match(const char *first, const char *second)
{
    while (*first != '\0' && *second != '\0') {
        if (tolower((unsigned char)*first) != tolower((unsigned char)*second)) {
            return 0;
        }
        ++first;
        ++second;
    }
    return *first == '\0' && *second == '\0';
}

static void print_match(const Item *lost, const Item *found, int *match_count)
{
    int category_matches = categories_match(lost->category, found->category);
    int location_matches = shared_keywords(lost->location, found->location);
    int shared_description_words =
        shared_keywords(lost->description, found->description);
    long date_gap = date_ordinal(lost->date) - date_ordinal(found->date);
    int keyword_score = shared_description_words * KEYWORD_POINTS;
    int score = category_matches ? CATEGORY_POINTS : 0;

    if (date_gap < 0) {
        date_gap = -date_gap;
    }
    if (location_matches > 0) {
        score += LOCATION_POINTS;
    }
    if (keyword_score > MAX_KEYWORD_POINTS) {
        keyword_score = MAX_KEYWORD_POINTS;
    }
    score += keyword_score;
    if (date_gap <= MAX_MATCH_DAYS) {
        score += DATE_POINTS;
    }
    if (score < MIN_MATCH_SCORE) {
        return;
    }

    ++*match_count;
    printf("Possible match (score %d/100): lost #%d \"%s\" <-> "
           "found #%d \"%s\"\n",
           score, lost->id, lost->name, found->id, found->name);
    printf("  Reasons:");
    if (category_matches) {
        printf(" category matches");
    }
    if (location_matches > 0) {
        printf("%s similar location",
               category_matches ? ";" : "");
    }
    if (shared_description_words > 0) {
        printf("%s %d shared description keyword%s",
               (category_matches || location_matches > 0) ? ";" : "",
               shared_description_words,
               shared_description_words == 1 ? "" : "s");
    }
    if (date_gap <= MAX_MATCH_DAYS) {
        printf("%s dates are within %d days",
               (category_matches || location_matches > 0 ||
                shared_description_words > 0)
                   ? ";"
                   : "",
               MAX_MATCH_DAYS);
    }
    putchar('\n');
}

static void compare_lost_with_tree(const Item *lost, const ItemNode *root,
                                   int *match_count)
{
    if (root == NULL) {
        return;
    }
    if (root->item.type == ITEM_FOUND && root->item.status == ITEM_OPEN) {
        print_match(lost, &root->item, match_count);
    }
    compare_lost_with_tree(lost, root->left, match_count);
    compare_lost_with_tree(lost, root->right, match_count);
}

static void find_matches_in_tree(const ItemNode *root, int *match_count)
{
    if (root == NULL) {
        return;
    }
    if (root->item.type == ITEM_LOST && root->item.status == ITEM_OPEN) {
        compare_lost_with_tree(&root->item, root, match_count);
    }
    find_matches_in_tree(root->left, match_count);
    find_matches_in_tree(root->right, match_count);
}

void matching_display_possible(const ItemNode *items)
{
    int matches = 0;

    find_matches_in_tree(items, &matches);
    if (matches == 0) {
        puts("No possible matches found. Add open lost and found reports "
             "to compare.");
    } else {
        printf("Found %d possible match%s.\n", matches,
               matches == 1 ? "" : "es");
    }
}

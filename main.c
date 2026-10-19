#include "input.h"
#include "matching.h"
#include "model.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ItemNode *items;
static ClaimQueue pending = {NULL, NULL};
static ClaimHistory resolved = {NULL, NULL};
static int next_claim_id = 1;

static int read_required(const char *prompt, char *value, size_t capacity)
{
    if (!input_line(prompt, value, capacity)) {
        return 0;
    }
    if (value[0] == '\0') {
        puts("This field cannot be empty; the operation was cancelled.");
        return 0;
    }
    return 1;
}

static void report_item(ItemType type)
{
    Item item = {0};
    int inserted = 0;

    if (!input_int("Enter a unique positive item ID: ", &item.id)) {
        return;
    }
    if (item_search(items, item.id) != NULL) {
        puts("That item ID already exists.");
        return;
    }
    if (!read_required("Item name: ", item.name, sizeof(item.name)) ||
        !read_required("Category (for example, Electronics): ", item.category,
                       sizeof(item.category)) ||
        !read_required("Description: ", item.description,
                       sizeof(item.description)) ||
        !read_required("Location: ", item.location, sizeof(item.location)) ||
        !input_date("Date (YYYY-MM-DD): ", item.date)) {
        return;
    }
    item.type = type;
    item.status = ITEM_OPEN;
    items = item_insert(items, &item, &inserted);
    if (!inserted) {
        fputs("Unable to allocate memory for the item; no record was added.\n",
              stderr);
        return;
    }
    printf("%s item recorded successfully with ID %d.\n",
           type == ITEM_LOST ? "Lost" : "Found", item.id);
}

static void search_item(void)
{
    int id;
    ItemNode *node;
    if (!input_int("Enter item ID to search: ", &id)) {
        return;
    }
    node = item_search(items, id);
    if (node == NULL) {
        printf("No item found with ID %d.\n", id);
        return;
    }
    print_item(&node->item);
}

static void register_claim(void)
{
    int lost_id;
    int found_id;
    ItemNode *lost;
    ItemNode *found;
    ClaimNode *node;

    if (!input_int("Lost item ID: ", &lost_id) ||
        !input_int("Found item ID: ", &found_id)) {
        return;
    }
    lost = item_search(items, lost_id);
    found = item_search(items, found_id);
    if (lost == NULL || found == NULL) {
        puts("Claim rejected: both item IDs must exist.");
        return;
    }
    if (lost->item.type != ITEM_LOST || found->item.type != ITEM_FOUND) {
        puts("Claim rejected: select one LOST item followed by one FOUND item.");
        return;
    }
    if (lost->item.status != ITEM_OPEN || found->item.status != ITEM_OPEN) {
        puts("Claim rejected: both items must still be open.");
        return;
    }
    if (claim_pair_exists(&pending, &resolved, lost_id, found_id)) {
        puts("Claim rejected: a claim for this item pair already exists.");
        return;
    }
    if (next_claim_id == INT_MAX) {
        puts("Claim rejected: no more claim IDs are available.");
        return;
    }
    node = malloc(sizeof(*node));
    if (node == NULL) {
        fputs("Unable to allocate memory for the claim; no claim was added.\n",
              stderr);
        return;
    }
    memset(node, 0, sizeof(*node));
    if (!read_required("Claimant name: ", node->claim.claimant,
                       sizeof(node->claim.claimant)) ||
        !read_required("Evidence or identifying details: ", node->claim.note,
                       sizeof(node->claim.note))) {
        free(node);
        return;
    }
    node->claim.id = next_claim_id++;
    node->claim.lost_item_id = lost_id;
    node->claim.found_item_id = found_id;
    node->claim.outcome = 'P';
    if (pending.rear == NULL) {
        pending.front = node;
    } else {
        pending.rear->next = node;
    }
    pending.rear = node;
    printf("Claim #%d added to the pending queue.\n", node->claim.id);
}

static void resolve_claim(void)
{
    int id;
    int approved;
    char decision[16];
    ClaimNode *previous;
    ClaimNode *node;
    ItemNode *lost;
    ItemNode *found;

    if (pending.front == NULL) {
        puts("There are no pending claims to resolve.");
        return;
    }
    claim_queue_print(&pending);
    if (!input_int("Claim ID to resolve: ", &id)) {
        return;
    }
    node = claim_queue_find(&pending, id, &previous);
    if (node == NULL) {
        puts("No pending claim has that ID; resolved claims cannot be resolved again.");
        return;
    }
    if (!input_line("Approve this claim? Enter y or n: ", decision,
                    sizeof(decision))) {
        return;
    }
    if (strlen(decision) != 1 ||
        (tolower((unsigned char)decision[0]) != 'y' &&
         tolower((unsigned char)decision[0]) != 'n')) {
        puts("Enter y or n; the claim remains pending.");
        return;
    }
    approved = tolower((unsigned char)decision[0]) == 'y';
    lost = item_search(items, node->claim.lost_item_id);
    found = item_search(items, node->claim.found_item_id);
    if (lost == NULL || found == NULL) {
        fputs("Cannot resolve claim: a referenced item is missing.\n", stderr);
        return;
    }
    if (approved &&
        (lost->item.status != ITEM_OPEN || found->item.status != ITEM_OPEN)) {
        puts("Cannot approve claim: one or both items are no longer open.");
        return;
    }

    if (previous == NULL) {
        pending.front = node->next;
    } else {
        previous->next = node->next;
    }
    if (pending.rear == node) {
        pending.rear = previous;
    }
    if (pending.front == NULL) {
        pending.rear = NULL;
    }
    node->next = NULL;
    node->claim.outcome = approved ? 'A' : 'R';
    if (approved) {
        lost->item.status = ITEM_CLAIMED;
        found->item.status = ITEM_CLAIMED;
    }
    if (resolved.tail == NULL) {
        resolved.head = node;
    } else {
        resolved.tail->next = node;
    }
    resolved.tail = node;
    printf("Claim #%d resolved: %s.\n", id,
           approved ? "approved; both items marked CLAIMED" : "rejected");
}

static void display_statistics(void)
{
    int total = 0;
    int lost = 0;
    int found = 0;
    int claimed = 0;
    int pending_count = 0;
    int resolved_count = 0;
    const ClaimNode *node;

    item_count_status(items, &total, &lost, &found, &claimed);
    for (node = pending.front; node != NULL; node = node->next) {
        ++pending_count;
    }
    for (node = resolved.head; node != NULL; node = node->next) {
        ++resolved_count;
    }
    puts("System statistics:");
    printf("Items: %d total | %d lost | %d found | %d claimed\n",
           total, lost, found, claimed);
    printf("Claims: %d pending | %d resolved\n", pending_count, resolved_count);
}

static void print_menu(void)
{
    puts("\n=== Campus Lost & Found Management System ===");
    puts("1. Report a Lost Item");
    puts("2. Report a Found Item");
    puts("3. Search Items by ID");
    puts("4. Display All Items");
    puts("5. Find Possible Matches");
    puts("6. Register a Claim");
    puts("7. View Pending Claims");
    puts("8. Resolve a Claim");
    puts("9. View Resolved Claims");
    puts("10. Display Statistics");
    puts("11. Exit");
}

int main(void)
{
    int choice;
    int running = 1;

    while (running) {
        print_menu();
        if (!input_int("Choose an option (1-11): ", &choice)) {
            if (feof(stdin)) {
                break;
            }
            continue;
        }
        switch (choice) {
        case 1:
            report_item(ITEM_LOST);
            break;
        case 2:
            report_item(ITEM_FOUND);
            break;
        case 3:
            search_item();
            break;
        case 4:
            if (items == NULL) {
                puts("There are no item reports.");
            } else {
                item_display_inorder(items);
            }
            break;
        case 5:
            matching_display_possible(items);
            break;
        case 6:
            register_claim();
            break;
        case 7:
            claim_queue_print(&pending);
            break;
        case 8:
            resolve_claim();
            break;
        case 9:
            claim_history_print(&resolved);
            break;
        case 10:
            display_statistics();
            break;
        case 11:
            running = 0;
            break;
        default:
            puts("Invalid choice. Enter a number from 1 to 11.");
            break;
        }
    }

    item_tree_free(items);
    claim_queue_free(&pending);
    claim_history_free(&resolved);
    puts("Goodbye. All allocated memory has been released.");
    return 0;
}

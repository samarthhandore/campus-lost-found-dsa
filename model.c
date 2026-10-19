#include "model.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ItemNode *item_insert(ItemNode *root, const Item *item, int *inserted)
{
    if (root == NULL) {
        ItemNode *node = malloc(sizeof(*node));
        if (node == NULL) {
            return NULL;
        }
        node->item = *item;
        node->left = NULL;
        node->right = NULL;
        *inserted = 1;
        return node;
    }

    if (item->id < root->item.id) {
        ItemNode *child = item_insert(root->left, item, inserted);
        if (*inserted) {
            root->left = child;
        }
    } else if (item->id > root->item.id) {
        ItemNode *child = item_insert(root->right, item, inserted);
        if (*inserted) {
            root->right = child;
        }
    }
    return root;
}

ItemNode *item_search(ItemNode *root, int id)
{
    while (root != NULL) {
        if (id == root->item.id) {
            return root;
        }
        root = id < root->item.id ? root->left : root->right;
    }
    return NULL;
}

void print_item(const Item *item)
{
    printf("ID: %d | %s | %s | %s\n", item->id,
           item->type == ITEM_LOST ? "LOST" : "FOUND",
           item->status == ITEM_OPEN ? "OPEN" : "CLAIMED", item->name);
    printf("  Category: %s | Date: %s | Location: %s\n",
           item->category, item->date, item->location);
    printf("  Description: %s\n", item->description);
}

void item_display_inorder(const ItemNode *root)
{
    if (root == NULL) {
        return;
    }
    item_display_inorder(root->left);
    print_item(&root->item);
    item_display_inorder(root->right);
}

void item_count_status(const ItemNode *root, int *total, int *lost,
                       int *found, int *claimed)
{
    if (root == NULL) {
        return;
    }
    ++*total;
    if (root->item.type == ITEM_LOST) {
        ++*lost;
    } else {
        ++*found;
    }
    if (root->item.status == ITEM_CLAIMED) {
        ++*claimed;
    }
    item_count_status(root->left, total, lost, found, claimed);
    item_count_status(root->right, total, lost, found, claimed);
}

void item_tree_free(ItemNode *root)
{
    if (root == NULL) {
        return;
    }
    item_tree_free(root->left);
    item_tree_free(root->right);
    free(root);
}

static int pair_matches(const Claim *claim, int lost_id, int found_id)
{
    return claim->lost_item_id == lost_id &&
           claim->found_item_id == found_id;
}

int claim_pair_exists(const ClaimQueue *queue, const ClaimHistory *history,
                      int lost_id, int found_id)
{
    const ClaimNode *node = queue->front;
    while (node != NULL) {
        if (pair_matches(&node->claim, lost_id, found_id)) {
            return 1;
        }
        node = node->next;
    }
    node = history->head;
    while (node != NULL) {
        if (pair_matches(&node->claim, lost_id, found_id)) {
            return 1;
        }
        node = node->next;
    }
    return 0;
}

ClaimNode *claim_queue_find(const ClaimQueue *queue, int claim_id,
                            ClaimNode **previous)
{
    ClaimNode *node = queue->front;
    *previous = NULL;
    while (node != NULL && node->claim.id != claim_id) {
        *previous = node;
        node = node->next;
    }
    return node;
}

void claim_queue_print(const ClaimQueue *queue)
{
    const ClaimNode *node = queue->front;
    if (node == NULL) {
        puts("There are no pending claims.");
        return;
    }
    puts("Pending claims (oldest first):");
    while (node != NULL) {
        const Claim *claim = &node->claim;
        printf("Claim #%d | Lost item #%d | Found item #%d | Claimant: %s\n",
               claim->id, claim->lost_item_id, claim->found_item_id,
               claim->claimant);
        printf("  Reason: %s\n", claim->note);
        node = node->next;
    }
}

static void print_claim(const Claim *claim)
{
    printf("Claim #%d | Lost item #%d | Found item #%d | Claimant: %s\n",
           claim->id, claim->lost_item_id, claim->found_item_id,
           claim->claimant);
    printf("  Result: %s | Reason: %s\n",
           claim->outcome == 'A' ? "APPROVED" : "REJECTED", claim->note);
}

void claim_history_print(const ClaimHistory *history)
{
    const ClaimNode *node = history->head;
    if (node == NULL) {
        puts("There are no resolved claims.");
        return;
    }
    puts("Resolved claims (resolution order):");
    while (node != NULL) {
        print_claim(&node->claim);
        node = node->next;
    }
}

void claim_history_free(ClaimHistory *history)
{
    ClaimNode *node = history->head;
    while (node != NULL) {
        ClaimNode *next = node->next;
        free(node);
        node = next;
    }
    history->head = NULL;
    history->tail = NULL;
}

void claim_queue_free(ClaimQueue *queue)
{
    ClaimNode *node = queue->front;
    while (node != NULL) {
        ClaimNode *next = node->next;
        free(node);
        node = next;
    }
    queue->front = NULL;
    queue->rear = NULL;
}

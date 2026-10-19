#ifndef MODEL_H
#define MODEL_H

#define ITEM_NAME_LEN 80
#define CATEGORY_LEN 48
#define DESCRIPTION_LEN 256
#define LOCATION_LEN 96
#define CLAIMANT_LEN 80
#define CLAIM_NOTE_LEN 160

typedef enum {
    ITEM_LOST,
    ITEM_FOUND
} ItemType;

typedef enum {
    ITEM_OPEN,
    ITEM_CLAIMED
} ItemStatus;

typedef struct {
    int id;
    char name[ITEM_NAME_LEN];
    char category[CATEGORY_LEN];
    char description[DESCRIPTION_LEN];
    char location[LOCATION_LEN];
    char date[11];
    ItemType type;
    ItemStatus status;
} Item;

typedef struct ItemNode {
    Item item;
    struct ItemNode *left;
    struct ItemNode *right;
} ItemNode;

typedef struct {
    int id;
    int lost_item_id;
    int found_item_id;
    char claimant[CLAIMANT_LEN];
    char note[CLAIM_NOTE_LEN];
    char outcome;
} Claim;

typedef struct ClaimNode {
    Claim claim;
    struct ClaimNode *next;
} ClaimNode;

typedef struct {
    ClaimNode *front;
    ClaimNode *rear;
} ClaimQueue;

typedef struct {
    ClaimNode *head;
    ClaimNode *tail;
} ClaimHistory;

ItemNode *item_insert(ItemNode *root, const Item *item, int *inserted);
ItemNode *item_search(ItemNode *root, int id);
void item_display_inorder(const ItemNode *root);
void item_count_status(const ItemNode *root, int *total, int *lost,
                       int *found, int *claimed);
void item_tree_free(ItemNode *root);
void print_item(const Item *item);

int claim_pair_exists(const ClaimQueue *queue, const ClaimHistory *history,
                      int lost_id, int found_id);
ClaimNode *claim_queue_find(const ClaimQueue *queue, int claim_id,
                            ClaimNode **previous);
void claim_queue_print(const ClaimQueue *queue);
void claim_history_print(const ClaimHistory *history);
void claim_history_free(ClaimHistory *history);
void claim_queue_free(ClaimQueue *queue);

#endif

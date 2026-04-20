#include "tree.h"
#include "pes.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ---------- SORT FUNCTION ----------
int compare_entries(const void *a, const void *b) {
    const TreeEntry *ea = (const TreeEntry *)a;
    const TreeEntry *eb = (const TreeEntry *)b;
    return strcmp(ea->name, eb->name);
}

// ---------- SERIALIZE ----------
int tree_serialize(const Tree *tree, void **data_out, size_t *len_out) {
    size_t total = 0;

    // calculate size
    for (int i = 0; i < tree->count; i++) {
        total += strlen(tree->entries[i].name) + 1; // name + '\0'
        total += 6 + 1; // mode + space
        total += HASH_SIZE; // hash
    }

    char *buf = malloc(total);
    if (!buf) return -1;

    // create sorted copy
    Tree sorted = *tree;
    qsort(sorted.entries, sorted.count, sizeof(TreeEntry), compare_entries);

    char *ptr = buf;

    for (int i = 0; i < sorted.count; i++) {
        // mode
        int written = sprintf(ptr, "%06o ", sorted.entries[i].mode);
        ptr += written;

        // name
        strcpy(ptr, sorted.entries[i].name);
        ptr += strlen(sorted.entries[i].name) + 1;

        // hash
        memcpy(ptr, sorted.entries[i].hash.hash, HASH_SIZE);
        ptr += HASH_SIZE;
    }

    *data_out = buf;
    *len_out = total;
    return 0;
}

// ---------- PARSE ----------
int tree_parse(const void *data, size_t len, Tree *tree_out) {
    const char *ptr = data;
    const char *end = ptr + len;

    tree_out->count = 0;

    while (ptr < end) {
        TreeEntry *e = &tree_out->entries[tree_out->count];

        // mode
        sscanf(ptr, "%o", &e->mode);

        // move to name
        while (*ptr != ' ') ptr++;
        ptr++;

        // name
        int i = 0;
        while (*ptr != '\0') {
            e->name[i++] = *ptr++;
        }
        e->name[i] = '\0';
        ptr++;

        // hash
        memcpy(e->hash.hash, ptr, HASH_SIZE);
        ptr += HASH_SIZE;

        tree_out->count++;
    }

    return 0;
}

// ---------- FROM INDEX ----------
int tree_from_index(ObjectID *id_out) {
    const char *data = "";
    size_t len = 0;

    if (object_write(OBJ_TREE, data, len, id_out) != 0)
        return -1;

    return 0;
}

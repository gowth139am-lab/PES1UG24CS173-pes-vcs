#include "commit.h"
#include "pes.h"
#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ---------- CREATE COMMIT ----------
int commit_create(const char *message, ObjectID *id_out) {

    // create tree from current index
    ObjectID tree_id;
    if (tree_from_index(&tree_id) != 0)
        return -1;

    char buffer[1024];
    time_t now = time(NULL);

    char tree_hex[65];
    hash_to_hex(&tree_id, tree_hex);

    int len = snprintf(buffer, sizeof(buffer),
        "tree %s\n"
        "author student\n"
        "time %lu\n\n"
        "%s\n",
        tree_hex, (unsigned long)now, message);

    // store commit object
    if (object_write(OBJ_COMMIT, buffer, len, id_out) != 0)
        return -1;

    // update HEAD
    FILE *f = fopen(".pes/HEAD", "w");
    if (f) {
        char hex[65];
        hash_to_hex(id_out, hex);
        fprintf(f, "%s\n", hex);
        fclose(f);
    }

    return 0;
}

// ---------- READ COMMIT ----------
int commit_read(const ObjectID *id, Commit *out) {
    void *data;
    size_t len;
    ObjectType type;

    if (object_read(id, &type, &data, &len) != 0)
        return -1;

    if (type != OBJ_COMMIT) {
        free(data);
        return -1;
    }

    char *ptr = (char *)data;

    // parse tree
    char tree_hex[65];
    sscanf(ptr, "tree %64s", tree_hex);
    hex_to_hash(tree_hex, &out->tree);

    // parse author
    char *author = strstr(ptr, "author ");
    if (author)
        sscanf(author, "author %[^\n]", out->author);

    // parse time
    char *time_str = strstr(ptr, "time ");
    if (time_str)
        sscanf(time_str, "time %lu", &out->timestamp);

    // parse message
    char *msg = strstr(ptr, "\n\n");
    if (msg)
        strcpy(out->message, msg + 2);

    free(data);
    return 0;
}

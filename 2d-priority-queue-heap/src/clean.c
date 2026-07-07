#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAXN 5000

typedef struct {
    char id[32];
    int key1;
    int key2;
} Entry;

Entry entries[MAXN];
int count = 0;

int entry_cmp(const Entry *a, const Entry *b) {
    if (a->key1 != b->key1) return a->key1 - b->key1;
    if (a->key2 != b->key2) return a->key2 - b->key2;
    return strcmp(a->id, b->id);
}

int parse_int_field(const char *value) {
    if (value == NULL || strlen(value) == 0) return 9999;
    char *end;
    int result = (int)strtol(value, &end, 10);
    if (*end != '\0') return 9999;
    return result;
}

int parse_line(char *line, Entry *e) {
    e->id[0] = '\0';
    e->key1  = -1;
    e->key2  = -1;

    char *token = strtok(line, ",");
    while (token != NULL) {
        char field[32] = {0}, value[64] = {0};

        while (*token == ' ') token++;

        if (sscanf(token, "%[^=]=%s", field, value) < 1) {
            token = strtok(NULL, ",");
            continue;
        }

        int flen = strlen(field); //trim ending spaces
        while (flen > 0 && field[flen - 1] == ' ') field[--flen] = '\0';//reduce until none

        if (strcmp(field, "id") == 0) {
            if (strlen(value) == 0) return 0;       // Rule 1: empty id
            strncpy(e->id, value, sizeof(e->id) - 1);

        } else if (strcmp(field, "key1") == 0) {
            e->key1 = parse_int_field(value);        // Rule 2

        } else if (strcmp(field, "key2") == 0) {
            e->key2 = parse_int_field(value);        // Rule 3
        }

        token = strtok(NULL, ",");
    }

    if (e->id[0] == '\0') return 0;                 // Rule 1: missing id

    if (e->key1 == -1) e->key1 = 9999;              // Rule 2: missing key1
    if (e->key2 == -1) e->key2 = 9999;              // Rule 3: missing key2

    return 1;
}

// ─── Insert or replace in global entries array (Rule 4) ──────────────────────
void insert_or_replace(Entry *e) {
    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].id, e->id) == 0) {
            // Duplicate: keep the smaller one in heap-order
            if (entry_cmp(e, &entries[i]) < 0)
                entries[i] = *e;
            return;
        }
    }
    // No duplicate found
    entries[count++] = *e;
}

// ─── Process one log file ─────────────────────────────────────────────────────
void process_file(const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "Warning: could not open %s\n", filepath);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        Entry e;
        if (parse_line(line, &e))
            insert_or_replace(&e);
    }

    fclose(f);
}

// ─── qsort comparator wrapper ─────────────────────────────────────────────────
int cmp_qsort(const void *a, const void *b) {
    return entry_cmp((const Entry *)a, (const Entry *)b);
}

// ─── Main ─────────────────────────────────────────────────────────────────────
int main() {
    const char *dir_path = "events_raw";

    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", dir_path, entry->d_name);
        process_file(filepath);
    }
    closedir(dir);

    qsort(entries, count, sizeof(Entry), cmp_qsort);

    // Write cleaned_events.txt
    FILE *out = fopen("cleaned_events.txt", "w");
    fprintf(out, "event_id key1 key2\n");
    if (!out) { perror("fopen output"); return 1; }

    for (int i = 0; i < count; i++)
        fprintf(out, "%s %d %d\n", entries[i].id, entries[i].key1, entries[i].key2);

    fclose(out);
    printf("Done. %d entries written to cleaned_events.txt\n", count);
    return 0;
}

#include <stdio.h>
#include <string.h>

#define MAXN 5000

/* ── Data structure ──────────────────────────────────────────────────────── */

struct HeapNode {
    int  key1;
    int  key2;
    char event_id[32];
};

struct HeapNode heap[MAXN];
int heap_size = 0;

// Global file pointers to fulfill strict assignment function signatures
FILE *f_out = NULL;
FILE *f_log = NULL;

/* ── Priority comparator ─────────────────────────────────────────────────── */

struct HeapNode *priority(struct HeapNode *a, struct HeapNode *b) {
    if (a->key1 != b->key1)
        return (a->key1 < b->key1) ? a : b;
    if (a->key2 != b->key2)
        return (a->key2 < b->key2) ? a : b;
    return (strcmp(a->event_id, b->event_id) <= 0) ? a : b;
}

/* ── Index helpers ───────────────────────────────────────────────────────── */

int parent(int i) { return (i - 1) / 2; }
int left(int i)   { return 2 * i + 1;   }
int right(int i)  { return 2 * i + 2;   }

void swap_nodes(struct HeapNode *A, int i, int j) {
    struct HeapNode tmp = A[i]; A[i] = A[j]; A[j] = tmp;
}

/* ── Snapshot helper ─────────────────────────────────────────────────────── */

static void log_snapshot(const char *label) {
    if (!f_log) return;
    fprintf(f_log, "=== %s (size=%d) ===\n", label, heap_size);
    for (int i = 0; i < heap_size; i++)
        fprintf(f_log, "  [%d] %s %d %d\n", i,
                heap[i].event_id, heap[i].key1, heap[i].key2);
    fprintf(f_log, "\n");
}

/* ── Heap helpers ────────────────────────────────────────────────────────── */

void bubble_up(struct HeapNode *A, int i) {
    while (i > 0) {
        int p = parent(i);
        if (priority(&A[i], &A[p]) == &A[i] &&
            strcmp(A[i].event_id, A[p].event_id) != 0) {
            swap_nodes(A, i, p);
            i = p;
        } else break;
    }
}

void bubble_down(struct HeapNode *A, int i) {
    while (1) {
        int smallest = i, l = left(i), r = right(i);
        if (l < heap_size &&
            priority(&A[l], &A[smallest]) == &A[l] &&
            strcmp(A[l].event_id, A[smallest].event_id) != 0)
            smallest = l;
        if (r < heap_size &&
            priority(&A[r], &A[smallest]) == &A[r] &&
            strcmp(A[r].event_id, A[smallest].event_id) != 0)
            smallest = r;
        if (smallest != i) { swap_nodes(A, i, smallest); i = smallest; }
        else break;
    }
}

int find_index(struct HeapNode *A, const char *event_id) {
    for (int i = 0; i < heap_size; i++)
        if (strcmp(A[i].event_id, event_id) == 0) return i;
    return -1;
}

/* ── Mandatory Public operations ─────────────────────────────────────────── */

void insert(struct HeapNode *A, struct HeapNode x) {
    if (heap_size >= MAXN) return;

    A[heap_size++] = x;
    bubble_up(A, heap_size - 1);

    char label[64];
    snprintf(label, sizeof(label), "INSERT %s", x.event_id);
    log_snapshot(label);
}

void decrease_key(struct HeapNode *A, char event_id[], int new_key1, int new_key2) {
    int idx = find_index(A, event_id);
    if (idx == -1) return;

    struct HeapNode candidate = A[idx];
    candidate.key1 = new_key1;
    candidate.key2 = new_key2;

    if (priority(&candidate, &A[idx]) != &candidate) return;

    A[idx].key1 = new_key1;
    A[idx].key2 = new_key2;
    bubble_up(A, idx);

    char label[64];
    snprintf(label, sizeof(label), "DECREASE_KEY %s", event_id);
    log_snapshot(label);
}

struct HeapNode extract_min(struct HeapNode *A) {
    if (heap_size == 0) {
        struct HeapNode empty = {-1, -1, "NULL"};
        return empty;
    }
    struct HeapNode min = A[0];
    A[0] = A[--heap_size];
    bubble_down(A, 0);

    log_snapshot("EXTRACT_MIN");
    return min;
}

void print_heap_array(struct HeapNode *A, FILE *fp) {
    for (int i = 0; i < heap_size; i++)
        fprintf(fp, "%s %d %d\n", A[i].event_id, A[i].key1, A[i].key2);
    log_snapshot("PRINT_HEAP_ARRAY");
}

void build_heap(FILE *fp) {
    char line[128];
    if (!fgets(line, sizeof(line), fp)) return;

    while (fgets(line, sizeof(line), fp)) {
        struct HeapNode node;
        node.key1 = 9999;
        node.key2 = 9999;
        node.event_id[0] = '\0';

        if (sscanf(line, "%31s %d %d", node.event_id, &node.key1, &node.key2) < 1)
            continue;
        if (node.event_id[0] == '\0') continue;

        insert(heap, node);
    }
}

/* ── Query file parser ───────────────────────────────────────────────────── */

void process_queries(FILE *qf) {
    char line[256];
    while (fgets(line, sizeof(line), qf)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;

        char op[32], eid[32];
        int k1, k2;

        if (sscanf(line, "%31s", op) < 1) continue;

        if (strcmp(op, "INSERT") == 0) {
            sscanf(line, "%*s %31s %d %d", eid, &k1, &k2);
            struct HeapNode node;
            strcpy(node.event_id, eid);
            node.key1 = k1; node.key2 = k2;
            insert(heap, node);
            if (f_out) fprintf(f_out, "INSERT %s %d %d → SUCCESS\n", eid, k1, k2);

        } else if (strcmp(op, "EXTRACT_MIN") == 0) {
            extract_min(heap);
            if (f_out) fprintf(f_out, "EXTRACT_MIN → SUCCESS\n");

        } else if (strcmp(op, "DECREASE_KEY") == 0) {
            sscanf(line, "%*s %31s %d %d", eid, &k1, &k2);
            decrease_key(heap, eid, k1, k2);
            if (f_out) fprintf(f_out, "DECREASE_KEY %s %d %d → SUCCESS\n", eid, k1, k2);

        } else if (strcmp(op, "PRINT_HEAP_ARRAY") == 0) {
            print_heap_array(heap, f_out);
            if (f_out) fprintf(f_out, "PRINT_HEAP_ARRAY → SUCCESS\n");

        } else if (strcmp(op, "BUILD_HEAP") == 0) {
            heap_size = 0;
            FILE *cleaned = fopen("cleaned_events.txt", "r");
            if (cleaned) {
                build_heap(cleaned);
                fclose(cleaned);
                if (f_out) fprintf(f_out, "BUILD_HEAP → SUCCESS\n");
            } else {
                if (f_out) fprintf(f_out, "BUILD_HEAP → FAILURE\n");
            }
        }
    }
}

/* ── Main ────────────────────────────────────────────────────────────────── */

int main(void) {
    f_log = fopen("heap_log.txt", "w");
    if (!f_log) { perror("fopen heap_log.txt"); return 1; }

    FILE *queries = fopen("queries.txt", "r");
    if (!queries) { perror("fopen queries.txt"); fclose(f_log); return 1; }

    f_out = fopen("final_output.txt", "w");
    if (!f_out) { perror("fopen final_output.txt"); fclose(queries); fclose(f_log); return 1; }

    process_queries(queries);

    fclose(queries);
    fclose(f_out);
    fclose(f_log);

    printf("Done. Output in final_output.txt, snapshots in heap_log.txt\n");
    return 0;
}

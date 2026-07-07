# 2D Priority Queue — City Meteorological Department

A binary min-heap implemented from scratch in C, where each node is ordered
by **two priority keys** instead of one — built for a Data Structures &
Algorithms assignment modeling a disaster-response dispatch system.

## Problem

A weather department receives typhoon disaster events from multiple
monitoring stations. Each event carries:

- **`key1` (Event Priority)** — disaster severity (lower = more severe)
- **`key2` (Ease Priority)** — how easy mitigation is in that region (lower = easier)
- **`event_id`** — unique alphanumeric identifier

Node `A` outranks node `B` in the heap if, in order:
1. `key1(A) < key1(B)`, or
2. `key1` ties and `key2(A) < key2(B)`, or
3. both tie — broken lexicographically by `event_id`

The heap is array-backed and every operation is hand-implemented — no
standard library heap/priority-queue utilities.

## Two stages

### 1. `src/clean.c` — data cleaning
Ingests 8–15 raw, inconsistently-formatted log files from `events_raw/`
(fields out of order, missing fields, non-integer keys, duplicate IDs,
corrupted/empty IDs) and produces a single clean, sorted file:

- Drops entries with a missing/empty `event_id`
- Defaults non-integer or missing `key1`/`key2` to `9999`
- Resolves duplicate IDs by keeping the entry that ranks smaller in heap-order
- Outputs `cleaned_events.txt`, sorted by `(key1, key2, event_id)`

### 2. `src/heap.c` — the min-heap
Reads `cleaned_events.txt` to build the initial heap, then executes a
sequence of commands from `queries.txt`:

| Operation | Behavior |
|---|---|
| `INSERT id key1 key2` | Bubble-up insert |
| `DECREASE_KEY id key1 key2` | Lowers a node's priority, bubbles up |
| `EXTRACT_MIN` | Removes the root, bubbles down the replacement |
| `PRINT_HEAP_ARRAY` | Dumps current heap array to `final_output.txt` |
| `BUILD_HEAP` | (Re)builds the heap from `cleaned_events.txt` |

Every operation appends a full heap-array snapshot to `heap_log.txt`, so the
entire sequence of heap mutations can be traced and verified.

## Example

**Input (`events_raw/region_00_log.txt`):**
```
id=E82,key1=5,key2=12
key2=2,id=E77,key1=1
id=E13,key1=error,key2=9
key1=9,key2=6,id=E77          # duplicate id E77
id=E46,key1=3                 # key2 missing
id=,key1=7,key2=8             # corrupted id, discarded
```

**Output (`cleaned_events.txt`):**
```
event_id key1 key2
E77 1 2
E46 3 9999
E82 5 12
E13 9999 9
```

## Build & run

```bash
cd src
gcc -Wall -o clean clean.c
gcc -Wall -o heap heap.c

cd ..                    # run from project root — clean.c reads ./events_raw
./src/clean               # events_raw/ -> cleaned_events.txt
./src/heap                 # queries.txt -> final_output.txt, heap_log.txt
```

## Files

```
src/
  clean.c              # log parsing, error correction, dedup, sort
  heap.c                # binary min-heap: insert, extract-min, decrease-key
events_raw/            # sample raw station logs (input)
queries.txt              # sample operation sequence (input)
cleaned_events.txt        # generated: cleaned + sorted events
final_output.txt          # generated: query results
heap_log.txt               # generated: heap snapshot after every operation
```

## Notes

- Pure C standard library only (`stdio.h`, `stdlib.h`, `string.h`, `dirent.h`) — no external dependencies.
- Comparator logic (`priority()` in `heap.c`, `entry_cmp()` in `clean.c`) is shared conceptually but implemented independently in each file to keep the two stages decoupled.
- Originally built as a course assignment; sample `events_raw/` and `queries.txt` here are illustrative test data, not the original grading set.
# 2d-priority-queue-heap-project

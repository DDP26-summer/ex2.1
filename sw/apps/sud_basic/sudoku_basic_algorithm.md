# Iterative Sudoku Solver — Algorithm Description

## Overview

This solver uses **iterative backtracking** to fill a 9×9 Sudoku board without any recursive function calls. The core insight is that recursion is just an implicit call stack — by managing an explicit stack of `Frame` entries, we can replicate the same logic with a plain `while` loop.

---

## Data Structures

### The Board

The board is represented as a 9×9 integer array. Empty cells hold the value `0`; given clues hold digits `1–9`.

```c
int board[9][9];
```

### The Stack Frame

Each entry on the stack represents one decision point — a single empty cell being filled:

```c
typedef struct {
    int row, col;  /* position of the cell being tried */
    int digit;     /* last digit successfully placed here */
} Frame;
```

The `digit` field is critical: when we revisit a frame during backtracking, we resume trying from `digit + 1`, avoiding re-checking candidates that already failed.

The stack can hold at most 81 frames (one per cell).

---

## Core Helpers

### `is_valid(board, row, col, num)`

Checks whether placing `num` at `(row, col)` violates any Sudoku constraint:

- **Row rule** — `num` must not already appear in the same row.
- **Column rule** — `num` must not already appear in the same column.
- **Box rule** — `num` must not already appear in the same 3×3 sub-grid.

All three checks are performed in a single loop over `i = 0..8`:

```
board[row][i]              → row check
board[i][col]              → column check
board[br + i/3][bc + i%3]  → box check  (br, bc = top-left corner of the box)
```

Returns `1` if placement is legal, `0` otherwise.

### `next_empty(board, &row, &col)`

Scans the board left-to-right, top-to-bottom for the first cell containing `0`. Writes its coordinates into `row` and `col`, and returns `1`. Returns `0` if no empty cell exists (the board is complete).

---

## Algorithm Step-by-Step

```
1.  Find the first empty cell.
    If none → the board is already solved; return success.

2.  Push a Frame { row, col, digit=0 } onto the stack.

3.  LOOP while the stack is not empty:

    a.  Peek the top frame F.

    b.  SEARCH for a valid digit d in (F.digit+1 .. 9):
            if is_valid(board, F.row, F.col, d):
                place d at board[F.row][F.col]
                update F.digit = d
                mark "placed = true"
                break

    c.  If NO valid digit was found (dead end):
            clear board[F.row][F.col] = 0
            pop the stack  (backtrack)
            continue to next iteration

    d.  If a digit WAS placed:
            find the next empty cell
            if none → SOLVED; return success
            push Frame { next_row, next_col, digit=0 }

4.  If the stack empties without solving → no solution exists.
```

---

## Visualised Execution

```
Stack (top → bottom)       Board state          Action
──────────────────────     ────────────────     ──────────────────────────
[ (0,2) d=0 ]              . . _ ...            Push first empty cell
[ (0,2) d=4 ]              . . 4 ...            Place 4 at (0,2), push next
[ (0,3) d=6 ]              . . 4 6 ...          Place 6 at (0,3), push next
...
[ (0,3) d=0 ]              . . 4 _ ...          Dead end at some cell,
                                                 backtrack → pop, clear
[ (0,2) d=4 ]              . . 4 ...            Resume from d=5 next iter
```

Backtracking naturally "undoes" decisions by popping frames and clearing the cells they wrote — no recursion needed.

---

## Complexity

| Dimension | Value |
|-----------|-------|
| Time (worst case) | O(9^m) where *m* = number of empty cells |
| Space (stack) | O(m) — at most 81 frames |
| Space (board) | O(1) — in-place, fixed 9×9 array |

In practice, the `is_valid` constraint check prunes the search tree aggressively, so typical puzzles solve in microseconds.

---

## Key Differences vs. Recursive Approach

| Aspect | Recursive | Iterative (this solver) |
|--------|-----------|-------------------------|
| Call stack | Implicit (OS call stack) | Explicit `Frame` array |
| Backtrack mechanism | Function return | `pop` + `continue` |
| Stack overflow risk | Yes, for deep boards | No — bounded at 81 |
| Readability | Concise | More explicit control flow |
| Performance | Equivalent | Equivalent |

---

## Building and Running

```bash
gcc -Wall -Wextra -o sudoku sudoku_solver.c
./sudoku
```

Modify the `board` initialiser in `main()` to try different puzzles. Use `0` for empty cells.

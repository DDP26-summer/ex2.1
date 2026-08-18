
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <k5_libs.h>
#include <sud_lib.h>


/* -- Stack entry ------------------------------------------- */

typedef struct {
    int row, col;   /* cell being tried          */
    int digit;      /* last digit placed here    */
} Frame;


//----------------------------------------------------------------------------------

void print_board_assign(unsigned char board[SIZE][SIZE], int r, int c, int d) {
  #ifdef TRACK_PRINT
  printf("board[%d][%d] <- %d\n\n",r,c,d);
  #endif
}

void print_stack(Frame stack[SIZE*SIZE], int top) {
  #ifdef TRACK_PRINT
  for (int i=top;i>=0;i--) printf("stack[%d]={[%d,%d],%d}\n",i,stack[i].row,stack[i].col,stack[i].digit);  
  printf("\n");
  #endif
}


/* -- Validity helpers -------------------------------------- */
static int is_valid(unsigned char board[SIZE][SIZE], int row, int col, int num)
{
    int br = (row / 3) * 3;
    int bc = (col / 3) * 3;

    for (int i = 0; i < SIZE; i++) {
        if (board[row][i] == num) return 0;   /* row conflict   */
        if (board[i][col] == num) return 0;   /* col conflict   */
        if (board[br + i/3][bc + i%3] == num) return 0; /* box conflict */
    }
    return 1;
}

/* -- Find next empty cell ---------------------------------- */

static int next_empty(unsigned char board[SIZE][SIZE], int *row, int *col)
{
    for (int r = 0; r < SIZE; r++)
        for (int c = 0; c < SIZE; c++)
            if (board[r][c] == 0) { *row = r; *col = c; return 1; }
    return 0;   /* no empty cell → solved */
}


/* -- Iterative solver (explicit stack, no recursion) ------- */

int solve(unsigned char board[SIZE][SIZE])
{ 
    Frame stack[SIZE * SIZE];   /* at most 81 empty cells */
    int   top = -1;

    int row, col;

    /* Push the first empty cell onto the stack */
    if (!next_empty(board, &row, &col))
        return 1;   /* board is already complete */

    /* Seed: start trying digit 1 at the first empty cell */
    stack[++top] = (Frame){ row, col, 0 };
    print_stack(stack,top); 

    while (top >= 0) {
        Frame *f = &stack[top];

        /* Try the next digit (resume after last placed digit) */
        int placed = 0;
        for (int d = f->digit + 1; d <= 9; d++) {
            if (is_valid(board, f->row, f->col, d)) {
              board[f->row][f->col] = d;       
              print_board_assign(board,f->row,f->col,d);               
              f->digit = d;          /* remember what we placed */
              print_stack(stack,top); // Tracking mode               
              placed = 1;
              break;
            }
        }

        if (!placed) {
            /* Dead end – clear this cell and backtrack */
            board[f->row][f->col] = 0;
            print_board_assign(board,f->row,f->col,0);
            top--;
            print_stack(stack,top); 
            continue;
        }

        /* Find the next empty cell */
        if (!next_empty(board, &row, &col))
            return 1;   /* solved! */

        /* Push the next empty cell */
        stack[++top] = (Frame){ row, col, 0 };
        print_stack(stack,top); // Tracking mode 
    }
    return 0;   /* no solution */
}


/* -- Main -------------------------------------------------- */

int main(void)
{
    printf("HELLO SUDOKU SOLVER\n"); 
      
    /* 0 = empty cell */
    

   unsigned char  board[SIZE][SIZE] ;

    // FILE_REF file_ref ; // assigned by load_hex_file( ... OPEN*) 
    // // TODO call some python to generate sudoku_input.txt randomly ...
    // load_hex_file("app_src_dir/sudoku_input.txt", &file_ref, (char *)board, SIZE*SIZE, OPEN_LOAD_CLOSE); // Keep Open

     load_sud_board(board) ;

    printf("=== Input ===\n");
    print_board(board);

    printf("\nSolving in progress...\n");
    
    reset_report_performance(); 

    char solved = solve(board) ;
    report_task_performance("Sudoku solve"); // Report Performance
    
    if (solved) {
        printf("\n=== Solved ===\n\n");
        print_board(board);
    } else {
        printf("\nNo solution found.\n");
    }
    
    bm_quit_app(); 

}
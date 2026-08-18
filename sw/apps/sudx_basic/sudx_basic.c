
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <k5_libs.h>
#include <sud_lib.h>
#include "sudx_basic.h"

#ifndef HLCM
#include "sudx_basic_enums.svh"
#endif

//-------------------------------------------------------------------------

// Stack entry 

typedef struct {
    int row, col;   /* cell being tried          */
    int digit;      /* last digit placed here    */
} Frame;

//-------------------------------------------------------------------------

// legality helpers

char is_legal_nox(uint8_t board[SIZE][SIZE], int row, int col, int num)
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

//-------------------------------------------------------------------------

char is_legal_xlr(int row, int col, int num) { // Board is already loaded in XLR HW

    // xlr check if legal, also update xlr board copy if legal.

    start_reg_t start_reg ;
    start_reg.full = 0;
    start_reg.part.num =  num;
    start_reg.part.row =  row; 
    start_reg.part.col =  col;    
    start_reg.part.cmd =  CHECK_LEGAL ; 
    //printf("DBG  start_reg.full = %08x\n",  start_reg.full);  
    HOST_REG(XLR_START_RI) = start_reg.full; 
    
    done_reg_t done_reg;
    char done = 0;
    while (!done) {
       done_reg.full = HOST_REG(XLR_DONE_RI) ;
       done = done_reg.part.status;
       //printf("DBG setup is_legal_xlr Polling ...done_reg.full=%08x\n",done_reg.full); // uncomment for debug
    } 
    return  done_reg.part.result;  
}

//-------------------------------------------------------------------------

void clear_cell_xlr(int row, int col) {

    start_reg_t start_reg ;
    start_reg.full = 0;
    start_reg.part.row =  row; 
    start_reg.part.col =  col;    
    start_reg.part.cmd =  CLEAR_CELL ; 
    //printf("DBG  start_reg.full = %08x\n",  start_reg.full);  
    HOST_REG(XLR_START_RI) = start_reg.full; 
    
    done_reg_t done_reg;
    char done = 0;
    while (!done) {
       done_reg.full = HOST_REG(XLR_DONE_RI) ;
       done = done_reg.part.status;
       //printf("DBG setup is_clear_cell_xlr Polling ...done_reg.full=%08x\n",done_reg.full); // uncomment for debug
    } 

}


//-------------------------------------------------------------------------

char is_legal(uint8_t board[SIZE][SIZE], int row, int col, int num) {
  char ret_val;
  #ifdef XON
      ret_val = is_legal_xlr(row, col, num);
  #else
      ret_val = is_legal_nox(board, row, col, num);
  #endif
  //printf("DBG is_legal = %d , row=%d, col=%d, num=%d\n" , ret_val,row,col,num) ;
  return ret_val ;
  
}

//-------------------------------------------------------------------------

// Find next empty cell 

static int next_empty(uint8_t board[SIZE][SIZE], int *row, int *col)  {
    
    for (int r = 0; r < SIZE; r++)
        for (int c = 0; c < SIZE; c++)
            if (board[r][c] == 0) { *row = r; *col = c; return 1; }
    return 0;   /* no empty cell -> solved */
}


//-------------------------------------------------------------------------

void xlr_setup(uint8_t* xmem_board_addr) {    

    HOST_REG(XMEM_BOARD_ADDR_RI) = (unsigned int)xmem_board_addr;
     
    start_reg_t start_reg ;
    start_reg.full = 0;  
    start_reg.part.cmd = SETUP ;       
    HOST_REG(XLR_START_RI) = start_reg.full; 
        
    done_reg_t done_reg;
    char done = 0 ;
    while (!done) {
       done_reg.full = HOST_REG(XLR_DONE_RI) ;
       done = done_reg.part.status;
       //printf("DBG setup done Polling ...done_reg.full =%08x\n",done_reg.full); // uncomment for debug
    } 
}

//----------------------------------------------------------------------------------

/// Iterative solver (explicit stack, no recursion) 

int solve(uint8_t board[SIZE][SIZE])
{ 
    Frame stack[SIZE * SIZE];   /* at most 81 empty cells */
    int   top = -1;

    int row, col;

    #if defined(XON) 
      xlr_setup((uint8_t*)board) ;
    #endif
    
    /* Push the first empty cell onto the stack */
    if (!next_empty(board, &row, &col)) return 1;   /* board is already complete */

    /* Seed: start trying digit 1 at the first empty cell */
    stack[++top] = (Frame){ row, col, 0 };

    while (top >= 0) {
        Frame *f = &stack[top];
        

        /* Try the next digit (resume after last placed digit) */
        int placed = 0;
        for (int d = f->digit + 1; d <= 9; d++) {
            if (is_legal(board, f->row, f->col, d)) {
              board[f->row][f->col] = d;                
              f->digit = d;          /* remember what we placed */             
              placed = 1;
              break;
            }
        }

        if (!placed) {
            /* Dead end – clear this cell and backtrack */
            board[f->row][f->col] = 0;
            #ifdef XON
            clear_cell_xlr(f->row,f->col) ;    
            #endif
            //printf("DBG clear, row=%d, col=%d\n" , row,col) ;
            top--;
            continue;
        }

        /* Find the next empty cell */
        if (!next_empty(board, &row, &col))  return 1 ;   // Puzzle solved if none are empty

        /* Push the next empty cell */
        stack[++top] = (Frame){ row, col, 0 };
    }
    return 0;   /* no solution */
}


//----------------------------------------------------------------------------------

// Main 

int main(void) {
    
    printf("HELLO SUDOKU SOLVER\n"); 
    
    alloc_init();
          
    uint8_t (*board)[SIZE];
    
    board = alloc_get(SIZE*SIZE, "board");

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
       
    } else printf("\nNo solution found.\n");
    
      alloc_free((void*)board, "board"); 
   
    bm_quit_app(); 
}
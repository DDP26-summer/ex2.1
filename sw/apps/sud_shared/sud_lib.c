#include <k5_libs.h>
#include <sud_lib.h>

//-----------------------------------------------------------------------------------------------------------

void load_sud_board(unsigned char board[SIZE][SIZE]) {
         
    #ifdef HLCM    
      char sud_in_file_path[200] ;          
      char *k5_path = getenv("MY_K5_PROJ");
      #ifndef _GP_VAL_   
      bm_sprintf(sud_in_file_path,"%s/%s",k5_path,"$MY_K5_PROJ/sw/apps/sud_shared/sudoku_input_20blanks.txt"); // Easy one
      #else    
      bm_sprintf(sud_in_file_path,"%s/sudoku_input_%s.txt",sud_shared_path,EXPAND_AND_STRINGIFY(_GP_VAL_));  
      #endif 
      
    #else 
    char sud_in_file_path[80] ;                 
      #ifndef _GP_VAL_   
      bm_sprintf(sud_in_file_path,"%s","$MY_K5_PROJ/sw/apps/sud_shared/sudoku_input_20blanks.txt"); // Easy one
      #else    
      bm_sprintf(sud_in_file_path,"$MY_K5_PROJ/sw/apps/sud_shared/sudoku_input_%s.txt",EXPAND_AND_STRINGIFY(_GP_VAL_));  
      #endif
      
    #endif
          
    FILE_REF file_ref ; // assigned by load_hex_file( ... OPEN*) 
    // TODO call some python to generate sudoku_input.txt randomly ...
    load_hex_file(sud_in_file_path, &file_ref, (char *)board, SIZE*SIZE, OPEN_LOAD_CLOSE); // Keep Open 
   
}       

//-----------------------------------------------------------------------------------------------------------

void print_board(unsigned char board[SIZE][SIZE]) {
    for (int r = 0; r < SIZE; r++) {
        if (r % 3 == 0 && r != 0)
            printf(" ------+-------+------\n");
        for (int c = 0; c < SIZE; c++) {
            if (c % 3 == 0 && c != 0) printf(" |");
            printf(" %d", board[r][c]);
        }
        printf("\n");
    }
    printf("\n\n");   
}

//----------------------------------------------------------------
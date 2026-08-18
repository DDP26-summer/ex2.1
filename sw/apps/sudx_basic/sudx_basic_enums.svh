

typedef enum  {
    XLR_START_RI,
    XLR_DONE_RI,  
    XMEM_BOARD_ADDR_RI    
}   sudx_host_regs_idx_t;


// We use the start register to indicate command type
typedef enum  {
    NO_CMD, // Mist be first
    SETUP,    
    CHECK_LEGAL, 
    CLEAR_CELL,
    NUM_SUDX_CMDS  // Just to indicate max index of commands
}   sudx_cmd_t;

import xbox_def_pkg::*;
import sudx_def_pkg::*;

module sudx_basic (
  input   clk,
  input   rst_n,  
 
  // Command Status Register Interface
  host_regs_intrf.xlr host_regs_intrf, 

  // muxed interfaces
  mem_intf_read.client_read   mem_intf_read,
  mem_intf_write.client_write mem_intf_write
);

  enum {IDLE, 
        LOAD,
        CHECK,       // Check Validity
        CLEAR,       // clear cell
        DONE
       } next_state, state; //state machine 

  // MUST BE SAME AS SW , notice unlike C the first maos to msb
    
    typedef struct packed {
           logic [7:0] num ; 
           logic [7:0] row ;           
           logic [7:0] col ; 
           logic [7:0] cmd ;                            
    } start_reg_t;
           
    typedef struct packed {
           logic [15:0] unused ; 
           logic  [7:0] result ;            
           logic  [7:0] status ;          
    } done_reg_t;
     
          
  localparam BOARD_DIM = 9 ; 
  localparam BOX_DIM = 3 ;   
  localparam MUM_BOARD_ELEM = BOARD_DIM*BOARD_DIM ; 
   
  logic sudx_start;  
  start_reg_t start_reg;    
  logic clear_done_on_read;
  
  logic [5:0] num_bytes_requested ;
  
  sudx_cmd_t sudx_cmd ;
  
    
  logic [XMEM_ADDR_WIDTH-1:0] xmem_board_addr ;
  logic [$clog2(MUM_BOARD_ELEM)-1:0] loaded_start_idx , next_load_start_idx;
  
  logic [$clog2(BOARD_DIM)-1:0]  cmd_col, cmd_row, cmd_num ;
  
  logic is_legal, is_legal_ps ;
  
  done_reg_t done_reg  ;
  
  //--------------------------------------------------------------------------------------------------------
  

  // Internal board, each bit in element represent a nibble per index 
  // Notice this is different then the SW representation in xmem where each element is held in a byte
  logic [BOARD_DIM-1:0][BOARD_DIM-1:0][3:0] board, board_ps ; // sampled, and pre-sampled

  logic [MUM_BOARD_ELEM-1:0][3:0] board_flat_ps ; // for load convenience
    
  //--------------------------------------------------------------------------------------------------------
  
  // Host Regs Interface 
   
  assign sudx_start          = host_regs_intrf.host_regs_valid_pulse[XLR_START_RI] ; 
  assign start_reg           = start_reg_t'(host_regs_intrf.host_regs[XLR_START_RI]);
  assign sudx_cmd            = sudx_cmd_t'(start_reg.cmd[$clog2(NUM_SUDX_CMDS)-1:0])  ;  
  assign clear_done_on_read  = host_regs_intrf.host_regs_read_pulse[XLR_DONE_RI] ; 

  assign xmem_board_addr     = host_regs_intrf.host_regs[XMEM_BOARD_ADDR_RI]; 
 
  //======================================================================================================== 
 
  //State Machine Comb (most simple non-piped implementation)  
  always_comb begin
  
   // State-Machine Comb logic outputs defaults 

   next_state = state;
   
   next_load_start_idx = loaded_start_idx + num_bytes_requested;

   mem_intf_read.mem_size_bytes  = 32;              // default assuming MUM_BOARD_ELEM>32  
   mem_intf_read.mem_start_addr  = xmem_board_addr; // default board xmem address 
   mem_intf_read.mem_req = 0;
    
   // Initially accelerator does not write back to xmem
   mem_intf_write.mem_size_bytes = 0;
   mem_intf_write.mem_data       = 0;
   mem_intf_write.mem_start_addr = 0;
   mem_intf_write.mem_req        = 0;   
   
   //  default host regs output 
   host_regs_intrf.host_regs_data_out  = 0 ;
   host_regs_intrf.host_regs_valid_out = 0 ;
  
   board_ps = board;
   board_flat_ps = board;
   
   done_reg = 0 ;
   
   is_legal_ps = is_legal;
   
   case (state) // State Machine case
    
      IDLE: if (sudx_start) begin
       if      (sudx_cmd==SETUP) next_state = LOAD;           // Setup only, loading board, pending for execution
       else if (sudx_cmd==CHECK_LEGAL) next_state = CHECK;    // Check Validity  
       else if (sudx_cmd==CLEAR_CELL)  next_state = CLEAR;    // Clear cell  
      end
      
      LOAD: begin // Loading board from XMEM , in case of 9X9=81 elements it is 3 transactions: 32+32+17

        mem_intf_read.mem_req = 1;          
        mem_intf_read.mem_start_addr = xmem_board_addr + next_load_start_idx ;  
      
        if (mem_intf_read.mem_valid) begin
          integer i;
          for (i=0;i<32;i++) begin  
             if ((loaded_start_idx+i) < MUM_BOARD_ELEM) 
               board_flat_ps[loaded_start_idx+i] = mem_intf_read.mem_data[i][3:0] ;               
          end
          
          board_ps = board_flat_ps;
                                        
          if ((next_load_start_idx+32) >= MUM_BOARD_ELEM) // Overwrite default 32
            mem_intf_read.mem_size_bytes = MUM_BOARD_ELEM - next_load_start_idx ;   
          
          if (next_load_start_idx == MUM_BOARD_ELEM)  begin                       
            next_state = DONE;   
            mem_intf_read.mem_req = 0;            
          end
                                     
        end // if (mem_intf_read.mem_valid) 
        
      end // LOAD
      
      CHECK : begin 
        is_legal_ps = check_is_legal(cmd_row, cmd_col, cmd_num);
        next_state = DONE;
      end
      
      CLEAR : begin 
        next_state = DONE;
        board_ps[cmd_row][cmd_col] = 0 ;
      end
      

      DONE: begin
        done_reg.status = 1;   
        done_reg.result = is_legal ; 
        if (is_legal) begin
          board_ps[cmd_row][cmd_col] = cmd_num ; 
        end          
        if (clear_done_on_read) begin
          done_reg = 0; 
          is_legal_ps = 0 ;          
          next_state = IDLE;            
        end       
        done_reg.status = 1 ;    
        host_regs_intrf.host_regs_data_out[XLR_DONE_RI] = done_reg ; 
        host_regs_intrf.host_regs_valid_out[XLR_DONE_RI] = 1 ;
      end 
 
   endcase
   
  end // always

  //------------------------------------------------------------------------

 // Sequential
  always @(posedge clk or negedge rst_n) begin
  
    if(!rst_n) begin  
      state               <= IDLE;  
      board               <= 0 ;   
      loaded_start_idx    <= 0;  
      num_bytes_requested <= 0 ;
      is_legal            <= 0;
    end else begin     
      state      <= next_state ;
      board      <= board_ps ;     
      if (mem_intf_read.mem_req) begin
        loaded_start_idx <= next_load_start_idx ;
        num_bytes_requested <= mem_intf_read.mem_size_bytes;
      end
      is_legal <= is_legal_ps ;      
    end    
  end
   
  //------------------------------------------------------------------------
  
  // located at upper part of start register exactly agreed with SW
  assign cmd_col = start_reg.col ;
  assign cmd_row = start_reg.row ;
  assign cmd_num = start_reg.num ;
  
  //------------------------------------------------------------------------  

  // Comb Function to check_validity
    
  function automatic logic check_is_legal ;
  
    input[3:0] row, col, num ;  
  
    logic match ; 
    logic [3:0] box_orig_row, box_orig_col ; 
    logic r_in_box , c_in_box ;      

    integer i;     
    integer r, c;
      
    match = 0 ; // Default
    
    /* check row /col conflict */
    
    for (i = 0; i < BOARD_DIM; i++) begin    
        if (board[row][i] == num) match = 1 ;   /* row conflict   */
        if (board[i][col] == num) match = 1 ;   /* col conflict   */      
    end    
            
    /* check box conflict */
    
    box_orig_row = 0;
    box_orig_col = 0;
    
    for (i = 0; i < BOARD_DIM; i=i+BOX_DIM) begin
      if ((row>=i) && (row<(i+BOX_DIM))) box_orig_row = i ;
      if ((col>=i) && (col<(i+BOX_DIM))) box_orig_col = i ;   
    end
    
    r_in_box = 0 ;
    c_in_box = 0 ;
    for (r = 0; r < BOARD_DIM; r++) begin
        r_in_box = (r>=box_orig_row) && (r < (box_orig_row+BOX_DIM)) ;
        for (c = 0; c < BOARD_DIM; c++) begin
          c_in_box = (c>=box_orig_col) && (c < (box_orig_col+BOX_DIM)) ;
          if (r_in_box && c_in_box && (board[r][c]==num)) match = 1 ;
        end   
    end
           
    check_is_legal = !match ;
 
  endfunction  

 //--------------------------------------------------------------

endmodule

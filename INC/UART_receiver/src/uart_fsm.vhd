-- uart_fsm.vhd: UART controller - finite state machine
-- Author(s): Samuel Dobron (xdobro23), FIT VUT
--
library ieee;
use ieee.std_logic_1164.all;

-------------------------------------------------
entity UART_FSM is
port(
   CLK          : in std_logic;
   RST          : in std_logic;
   DIN          : in std_logic;
   CNT_SAMPLING : in std_logic_vector(4 downto 0);  -- ticks counter
   WORD_INDEX   : in std_logic_vector(3 downto 0);  -- used for indexing bit in byte
   READ         : out std_logic;                    -- boolean for determine RECEIVING state in receiver
   SAMPLE       : out std_logic;                    -- boolean for counting ticks for sampling bits
   VALID        : out std_logic                     -- used for DOUT_VLD
   );
end entity UART_FSM;

-------------------------------------------------
architecture behavioral of UART_FSM is
  type POSSIBLE_STATES is (AWAITING_START, AWAITING_FIRST, RECEIVING, AWAITING_STOP, VALIDATING);
  signal next_state   : POSSIBLE_STATES := AWAITING_START;
  signal actual_state : POSSIBLE_STATES := AWAITING_START;
  signal reset        : std_logic       := '0';
begin
  reset_process:
    process (RST) begin
      if RST = '1' then
        reset <= '1';
      elsif falling_edge(RST) then
        reset <= '0';
      end if;
    end process;
  
  states:
    process (CLK) begin
      if (rising_edge(CLK)) and (reset = '0') then
        case actual_state is
          when AWAITING_START =>
            if DIN = '0' then 
              next_state <= AWAITING_FIRST;
            end if;
          when AWAITING_FIRST =>
            if CNT_SAMPLING = "10110" then  -- this is 22. tick, one tick need READ <= 1 (23. tick); so bit is sampled at 24. tick
              next_state <= RECEIVING;
            end if;
          when RECEIVING =>
            if WORD_INDEX = "1000" then
              next_state <= AWAITING_STOP;
            end if;
          when AWAITING_STOP =>
            if DIN = '1' then
              next_state <= VALIDATING;
            end if;
          when VALIDATING =>
            next_state <= AWAITING_START;
          when others => null;
        end case;
      elsif reset = '1' then
        next_state <= AWAITING_START;
      end if;
    end process;
  
  next_state_change:
    process(next_state) begin
      if (reset = '0') and (next_state = AWAITING_START or next_state = AWAITING_FIRST or next_state = RECEIVING or
          next_state = AWAITING_STOP  or next_state = VALIDATING)
      then 
        actual_state <= next_state;
      else                        
        actual_state <= AWAITING_START;  -- in case, smth wrong happen
      end if;
    end process;
  
  output:
    process (actual_state) begin
      if (actual_state = VALIDATING) and (reset = '0') then
        VALID <= '1';
      else
        VALID <= '0';
      end if;
    
      if (actual_state = RECEIVING) and (reset = '0') then
        READ <= '1';
      else
        READ <= '0';
      end if;
    
      if (actual_state = AWAITING_FIRST or actual_state = RECEIVING) and (reset = '0') then
        SAMPLE <= '1';
      else
        SAMPLE <= '0';
      end if;
    end process;

end behavioral;

-- uart.vhd: UART controller - receiving part
-- Author(s): Samuel Dobron (xdobro23), FIT VUT
--
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

-------------------------------------------------
entity UART_RX is
port(	
      CLK       : in std_logic;
      RST       : in std_logic;
      DIN       : in std_logic;
      DOUT      : out std_logic_vector(7 downto 0);
      DOUT_VLD  : out std_logic
    );
end UART_RX;  

-------------------------------------------------
architecture behavioral of UART_RX is
  signal CNT_SAMPLING : std_logic_vector(4 downto 0)  := "00000";  -- ticks
  signal WORD_INDEX   : std_logic_vector(3 downto 0)  := "0000";   -- bit index in word
  signal READ         : std_logic                     := '0';
  signal SAMPLE       : std_logic                     := '0';

begin
    FSM: entity work.UART_FSM(behavioral)
    port map (
        CLK           => CLK,
        RST           => RST,
        DIN           => DIN,
        CNT_SAMPLING  => CNT_SAMPLING,
        WORD_INDEX    => WORD_INDEX,
        READ          => READ,
        SAMPLE        => SAMPLE,
        VALID         => DOUT_VLD
    );
    
    process (CLK) begin
      sampling:
        if rising_edge(CLK) and SAMPLE = '1' then
            CNT_SAMPLING <= CNT_SAMPLING + 1;
        end if;
      
       
      receiving: 
        if rising_edge(CLK) then
          if READ = '1' and CNT_SAMPLING(4) = '1' then
              if WORD_INDEX(3) = '0' then -- just making sure nothing like "undefined" is there
                DOUT(to_integer(unsigned(WORD_INDEX))) <= DIN;
              end if;
              CNT_SAMPLING <= "00000";
              WORD_INDEX <= WORD_INDEX + 1; -- move index of next bit in byte
          elsif READ /= '1' then
            WORD_INDEX <= "0000";
          end if;
        end if;
      
    end process;
end behavioral;

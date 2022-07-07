-- cpu.vhd: Simple 8-bit CPU (BrainLove interpreter)
-- Copyright (C) 2021 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Samuel Dobron (xdobro23)
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity cpu is
 port (
   CLK   : in std_logic;  -- hodinovy signal
   RESET : in std_logic;  -- asynchronni reset procesoru
   EN    : in std_logic;  -- povoleni cinnosti procesoru
 
   -- synchronni pamet ROM
   CODE_ADDR : out std_logic_vector(11 downto 0); -- adresa do pameti
   CODE_DATA : in std_logic_vector(7 downto 0);   -- CODE_DATA <- rom[CODE_ADDR] pokud CODE_EN='1'
   CODE_EN   : out std_logic;                     -- povoleni cinnosti
   
   -- synchronni pamet RAM
   DATA_ADDR  : out std_logic_vector(9 downto 0); -- adresa do pameti
   DATA_WDATA : out std_logic_vector(7 downto 0); -- ram[DATA_ADDR] <- DATA_WDATA pokud DATA_EN='1'
   DATA_RDATA : in std_logic_vector(7 downto 0);  -- DATA_RDATA <- ram[DATA_ADDR] pokud DATA_EN='1'
   DATA_WREN  : out std_logic;                    -- cteni z pameti (DATA_WREN='0') / zapis do pameti (DATA_WREN='1')
   DATA_EN    : out std_logic;                    -- povoleni cinnosti
   
   -- vstupni port
   IN_DATA   : in std_logic_vector(7 downto 0);   -- IN_DATA obsahuje stisknuty znak klavesnice pokud IN_VLD='1' a IN_REQ='1'
   IN_VLD    : in std_logic;                      -- data platna pokud IN_VLD='1'
   IN_REQ    : out std_logic;                     -- pozadavek na vstup dat z klavesnice
   
   -- vystupni port
   OUT_DATA : out  std_logic_vector(7 downto 0);  -- zapisovana data
   OUT_BUSY : in std_logic;                       -- pokud OUT_BUSY='1', LCD je zaneprazdnen, nelze zapisovat,  OUT_WREN musi byt '0'
   OUT_WREN : out std_logic                       -- LCD <- OUT_DATA pokud OUT_WE='1' a OUT_BUSY='0'
 );
end cpu;


-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of cpu is
----- PC
	signal pc_out : std_logic_vector(11 downto 0);  -- out
	signal pc_inc : std_logic;  -- inc
	signal pc_dec : std_logic;  -- dec
----- PC
	
----- CNT
	signal cnt_reg : std_logic_vector(11 downto 0);  -- out
	signal cnt_inc : std_logic;  -- inc
	signal cnt_dec : std_logic;  -- dec
----- CNT

----- PTR
	signal ptr_reg : std_logic_vector(9 downto 0);  -- out
	signal ptr_inc : std_logic;  -- inc
	signal ptr_dec : std_logic;  -- dec
----- PTR

----- MX
	signal mx_out : std_logic_vector(7 downto 0);
	signal mx_sel : std_logic_vector(1 downto 0) := "00";
----- MX

----- FSM
	type fsm_state is (
		FSM_START, FSM_FETCH, FSM_DEC,
		
------ EXEC STATES
		EXEC_PTR_INC,  -- >
		EXEC_PTR_DEC,  -- <
		EXEC_CELL_INC, EXEC_CELL_INC_MX_MOVE, EXEC_CELL_INC_STORE_MOVED,  -- +
		EXEC_CELL_DEC, EXEC_CELL_DEC_MX_MOVE, EXEC_CELL_DEC_STORE,  -- -
		EXEC_LOOP_START, EXEC_LOOP_WHILE_START_CHECK, EXEC_LOOP_WHILE, EXEC_LOOP_WHILE_END,  -- [
		EXEC_LOOP_WHILE_END_START, EXEC_LOOP_WHILE_END_START_CHECK, EXEC_LOOP_WHILE_END_WHILE, EXEC_LOOP_WHILE_END_CNT, EXEC_LOOP_WHILE_END_END,  -- ]
		EXEC_PRINT, EXEC_PRINT_WHILE,  -- .
		EXEC_LOAD, EXEC_LOAD_INVLD,  --  ,
		EXEC_BREAK_START, EXEC_BREAK, EXEC_BREAK_END,  -- ~
		EXEC_NULL,  -- null
		EXEC_INVALID  -- invalid
	);
	signal fsm_actual : fsm_state := FSM_START;
	signal fsm_next : fsm_state;
begin
PC: process (pc_inc, pc_dec, CLK, RESET)
	begin
		if RESET = '1' then
			pc_out <= "000000000000";
		elsif rising_edge(CLK) then
			if pc_dec = '1' then
				pc_out <= pc_out - 1;
			elsif pc_inc = '1' then
				pc_out <= pc_out + 1;
			end if;
		end if;
end process;
CODE_ADDR <= pc_out;
	
	
CNT: process (cnt_inc, cnt_dec, CLK, RESET)
	begin
		if RESET = '1' then
			cnt_reg <= "000000000000";
		elsif rising_edge(CLK) then
			if cnt_dec = '1' then
				cnt_reg <= cnt_reg - 1;
			elsif cnt_inc = '1' then
				cnt_reg <= cnt_reg + 1;
			end if;
		end if;
end process;

PTR: process (ptr_inc, ptr_dec, CLK, RESET)
	begin
		if RESET = '1' then
			ptr_reg <= "0000000000";
		elsif rising_edge(CLK) then
			if ptr_dec = '1' then
				ptr_reg <= ptr_reg - 1;
			elsif ptr_inc = '1' then
				ptr_reg <= ptr_reg + 1;
			end if;
		end if;
end process;
DATA_ADDR <= ptr_reg;
OUT_DATA <= DATA_RDATA;

	

MX: process (mx_sel, CLK, RESET)
	begin
		if RESET = '1' then
			mx_out <= "00000000";
		elsif rising_edge(CLK) then
			case mx_sel is
				when "10"	=> mx_out <= DATA_RDATA - 1;
				when "01"	=> mx_out <= DATA_RDATA + 1;
				when "00"	=> mx_out <= IN_DATA; -- data from input				
				when others => mx_out <= "00000000";
			end case;
		end if;
end process;
DATA_WDATA <= mx_out;

	fsm_actual_state: process (EN, CLK, RESET)
	begin
		if RESET = '1' then
			fsm_actual <= FSM_START;
		elsif rising_edge(CLK) then
			if EN = '1' then
				fsm_actual <= fsm_next;
			end if;
		end if;
	end process;

	fsm_main_process: process (fsm_actual, cnt_reg, IN_VLD, OUT_BUSY, DATA_RDATA, CODE_DATA)
			variable fsm_temp : fsm_state := FSM_START;
	begin
		pc_inc	<= '0';
		pc_dec	<= '0';
		ptr_inc	<= '0';
		ptr_dec	<= '0';
		cnt_inc	<= '0';
		cnt_dec	<= '0';
		DATA_EN	<= '0';
		DATA_WREN<= '0';
		OUT_WREN <= '0';
		IN_REQ	<= '0';
		CODE_EN	<= '0';
		
		case fsm_actual is
			when FSM_START =>
				fsm_next <= FSM_FETCH;

			when FSM_FETCH =>
				CODE_EN <= '1';
				fsm_next <= FSM_DEC;

			when FSM_DEC =>
				case CODE_DATA is
					when X"3E" => fsm_next <= EXEC_PTR_INC;					-- >
					when X"3C" => fsm_next <= EXEC_PTR_DEC;					-- <
					when X"2B" => fsm_next <= EXEC_CELL_INC;					-- +
					when X"2D" => fsm_next <= EXEC_CELL_DEC;					-- -
					when X"5B" => fsm_next <= EXEC_LOOP_START;				-- [
					when X"5D" => fsm_next <= EXEC_LOOP_WHILE_END_START;  -- ]
					when X"2E" => fsm_next <= EXEC_PRINT;						-- .
					when X"2C" => fsm_next <= EXEC_LOAD;						-- ,
					when X"7E" => fsm_next <= EXEC_BREAK_START;				-- ~
					when X"00" => fsm_next <= EXEC_NULL;						-- null
					when others=> fsm_next <= EXEC_INVALID;
				end case;


----------- >
			when EXEC_PTR_INC =>
				ptr_inc	<= '1';  -- PTR <- PTR+1
				pc_inc	<= '1';  -- PC <- PC+1
				fsm_next <= FSM_FETCH;

----------- <
			when EXEC_PTR_DEC =>
				ptr_dec	<= '1';  -- PTR <- PTR-1
				pc_inc	<= '1';  -- PC <- PC+1
				fsm_next <= FSM_FETCH;

----------- +
			when EXEC_CELL_INC =>
				DATA_EN	<= '1';
				DATA_WREN<= '0';  -- DATA_RDATA <- ram[PTR]
				fsm_next <= EXEC_CELL_INC_MX_MOVE;

			when EXEC_CELL_INC_MX_MOVE =>
				mx_sel	<= "01";  -- move MX to DATA_RDATA + 1
				fsm_next <= EXEC_CELL_INC_STORE_MOVED;

			when EXEC_CELL_INC_STORE_MOVED =>
				DATA_EN	<= '1';
				DATA_WREN<= '1';  -- ram[PTR] = MX (it "stores" DATA_RDATA + 1)
				pc_inc	<= '1';  -- PC <- PC+1
				fsm_next <= FSM_FETCH;

----------- -
			when EXEC_CELL_DEC =>
				DATA_EN	<= '1';
				DATA_WREN<= '0';  -- DATA_RDATA <- ram[PTR]
				fsm_next <= EXEC_CELL_DEC_MX_MOVE;
			
			when EXEC_CELL_DEC_MX_MOVE =>
				mx_sel	<= "10";
				fsm_next <= EXEC_CELL_DEC_STORE;

			when EXEC_CELL_DEC_STORE =>
				DATA_EN	<= '1';
				DATA_WREN<= '1';
				--mx_sel <= "10";  -- move MX to DATA_RDATA - 1
				pc_inc <= '1';  -- PC <- PC+1

				fsm_next <= FSM_FETCH;

----------- [
			when EXEC_LOOP_START =>
				pc_inc	<= '1';  -- PC <- PC+1
				DATA_EN	<= '1';
				DATA_WREN<= '0';
				fsm_next <= EXEC_LOOP_WHILE_START_CHECK;

			when EXEC_LOOP_WHILE_START_CHECK =>
				if DATA_RDATA = "00000000" then  -- if (ram[PTR] == 0)
					CODE_EN	<= '1';
					cnt_inc 	<= '1';
					fsm_temp := EXEC_LOOP_WHILE;
				else
					fsm_temp	:= FSM_FETCH;
				end if;
				fsm_next	<= fsm_temp;

			when EXEC_LOOP_WHILE =>
				pc_inc <= '1';  -- PC <- PC+1
			
				if cnt_reg = "00000000" then  -- while (CNT == 0)
					fsm_temp := FSM_FETCH;  -- end loop
				else -- while (CNT != 0)
					if CODE_DATA = X"5B" then  -- [
						cnt_inc <= '1';  -- CNT <- CNT+1
					elsif CODE_DATA = X"5D" then  -- ]
						cnt_dec <= '1';  -- CNT <- CNT+1
					end if;

					fsm_temp := EXEC_LOOP_WHILE_END;
				end if;
				fsm_next		<= fsm_temp;

			when EXEC_LOOP_WHILE_END =>
				CODE_EN <= '1';
				fsm_next<= EXEC_LOOP_WHILE;


----------- ]
			when EXEC_LOOP_WHILE_END_START =>
				DATA_EN	<= '1';
				DATA_WREN<= '0';  -- load ram[PTR]

				fsm_next	<= EXEC_LOOP_WHILE_END_START_CHECK;

			when EXEC_LOOP_WHILE_END_START_CHECK =>
				if DATA_RDATA /= "00000000" then  -- if (ram[PTR] != 0)
					cnt_inc	<= '1';  -- CNT <- CNT + 1
					pc_dec	<= '1';  -- PC <- PC-1
					fsm_temp	:= EXEC_LOOP_WHILE_END_END;
				else -- if (ram[PTR] == 0)
					pc_inc	<= '1';  -- PC <- PC+1
					fsm_temp	:= FSM_FETCH;
				end if;
				fsm_next		<= fsm_temp;

			when EXEC_LOOP_WHILE_END_WHILE =>
				if cnt_reg /= "00000000" then  -- while (CNT != 0)
					if CODE_DATA = X"5D" then  -- ]
						cnt_inc	<= '1';  -- CNT <- CNT+1
					elsif CODE_DATA = X"5B" then -- [
						cnt_dec	<= '1';  -- CNT <- CNT-1
					end if;

					fsm_temp		:= EXEC_LOOP_WHILE_END_CNT;
				else
					fsm_temp		:= FSM_FETCH;
				end if;
				fsm_next			<= fsm_temp;

			when EXEC_LOOP_WHILE_END_CNT =>
				if cnt_reg = "00000000" then  -- if (CNT == 0)
					pc_inc	<= '1';  -- PC <- PC+1
				else -- if (CNT != 0)
					pc_dec	<= '1';  -- PC <- PC-1
				end if;
				fsm_next		<= EXEC_LOOP_WHILE_END_END;

			when EXEC_LOOP_WHILE_END_END =>
				CODE_EN	<= '1';
				fsm_next	<= EXEC_LOOP_WHILE_END_WHILE;


----------- .
			when EXEC_PRINT =>
				DATA_EN	<= '1';
				DATA_WREN<= '0';  -- load ram[PTR]
				fsm_next	<= EXEC_PRINT_WHILE;

			when EXEC_PRINT_WHILE =>
				if OUT_BUSY /= '1' then  -- while(OUT_BUSY == 0)
					pc_inc	<= '1';  -- PC <- PC+1
					OUT_WREN <= '1';  -- OUT_DATA <- ram[PTR]
					fsm_temp := FSM_FETCH;
				else
					DATA_EN	<= '1';
					DATA_WREN<= '0';
					fsm_temp := EXEC_PRINT_WHILE;  -- wait till OUT_BUSY == 0
				end if;
				fsm_next		<= fsm_temp;


----------- ,
			when EXEC_LOAD =>
				IN_REQ <= '1';
				mx_sel <= "00";  -- change MX to read from IN
				fsm_next <= EXEC_LOAD_INVLD;

			when EXEC_LOAD_INVLD =>
				if IN_VLD = '1' then
					DATA_EN	<= '1';
					DATA_WREN<= '1';
					pc_inc	<= '1';
					fsm_temp	:= FSM_FETCH;					
				else
					IN_REQ	<= '1';
					mx_sel	<= "00";  -- set MX back
					fsm_temp	:= EXEC_LOAD_INVLD;
				end if;
				fsm_next		<= fsm_temp;


----------- ~
			when EXEC_BREAK_START =>
				cnt_inc	<= '1';
				pc_inc	<= '1';
				fsm_next	<= EXEC_BREAK_END;

			when EXEC_BREAK =>
				if cnt_reg /= "00000000" then  -- if (CNT != 0)
					if CODE_DATA = X"5B" then  -- [
						cnt_inc <= '1';
					elsif CODE_DATA = X"5D" then -- ]
						cnt_dec <= '1';
					end if;

					pc_inc <= '1';

					fsm_temp := EXEC_BREAK_END;
				else -- if (CNT == 0)
					fsm_temp := FSM_FETCH;
				end if;
				fsm_next <= fsm_temp;

			when EXEC_BREAK_END =>
				CODE_EN <= '1';
				fsm_next <= EXEC_BREAK;


			when EXEC_NULL =>
				fsm_next <= EXEC_NULL;

			when EXEC_INVALID =>
				pc_inc <= '1';
				fsm_next <= FSM_FETCH;
			
			when others =>
				null;

		end case;
	end process;

end behavioral;
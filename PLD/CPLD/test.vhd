--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   09:20:53 04/19/2019
-- Design Name:   
-- Module Name:   /home/artyom/Desktop/Projects/BBlitz/VHDL/Mux_actual/test.vhd
-- Project Name:  Blitz
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: mux
-- 
-- Dependencies:
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
--
-- Notes: 
-- This testbench has been automatically generated using types std_logic and
-- std_logic_vector for the ports of the unit under test.  Xilinx recommends
-- that these types always be used for the top-level I/O of a design in order
-- to guarantee that the testbench will bind correctly to the post-implementation 
-- simulation model.
--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY test IS
END test;
 
ARCHITECTURE behavior OF test IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT mux
    PORT(
         I_DIR : OUT  std_logic;
         I_MEMW : OUT  std_logic;
         I_MEMR : OUT  std_logic;
         I_IOW : OUT  std_logic;
         I_IOR : OUT  std_logic;
         I_NOWS : IN  std_logic;
         I_ALE : OUT  std_logic;
         I_IOCHRDY : IN  std_logic;
         R_BTN : IN  std_logic;
         RESET : OUT  std_logic;
         HALT : OUT  std_logic;
         READ_n : OUT  std_logic;
         RW_N : IN  std_logic;
         CLK : IN  std_logic;
         ECS : IN  std_logic;
         CDIS : OUT  std_logic;
         RW0 : OUT  std_logic;
         nRW0 : OUT  std_logic;
         RW1 : OUT  std_logic;
         nRW1 : OUT  std_logic;
         a0 : IN  std_logic;
         a1 : IN  std_logic;
         a16 : IN  std_logic;
         a19 : IN  std_logic;
         a20 : IN  std_logic;
         a21 : IN  std_logic;
         a26 : IN  std_logic;
         a27 : IN  std_logic;
         SIZ0 : IN  std_logic;
         SIZ1 : IN  std_logic;
         FC0 : IN  std_logic;
         FC1 : IN  std_logic;
         FC2 : IN  std_logic;
         AS : IN  std_logic;
         DS : IN  std_logic;
         DSACK0 : OUT  std_logic;
         DSACK1 : OUT  std_logic;
         STERM : OUT  std_logic;
         DRAM_DTACK : IN  std_logic;
         ROM : OUT  std_logic;
         FPU : OUT  std_logic;
         FLOPPY : OUT  std_logic;
         IDE : OUT  std_logic;
         KBD_CS : OUT  std_logic;
         IO_CS : OUT  std_logic;
         VGA_CS : OUT  std_logic;
         DRAM_CS : OUT  std_logic;
         ETHRNT : OUT  std_logic;
         BYTE0 : OUT  std_logic;
         BYTE1 : OUT  std_logic;
         BYTE2 : OUT  std_logic;
         BYTE3 : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal I_NOWS : std_logic := '0';
   signal I_IOCHRDY : std_logic := '0';
   signal R_BTN : std_logic := '0';
   signal RW_N : std_logic := '0';
   signal CLK : std_logic := '0';
   signal ECS : std_logic := '0';
   signal a0 : std_logic := '0';
   signal a1 : std_logic := '0';
   signal a16 : std_logic := '0';
   signal a19 : std_logic := '0';
   signal a20 : std_logic := '0';
   signal a21 : std_logic := '0';
   signal a26 : std_logic := '0';
   signal a27 : std_logic := '0';
   signal SIZ0 : std_logic := '0';
   signal SIZ1 : std_logic := '0';
   signal FC0 : std_logic := '0';
   signal FC1 : std_logic := '0';
   signal FC2 : std_logic := '0';
   signal AS : std_logic := '0';
   signal DS : std_logic := '0';
   signal DRAM_DTACK : std_logic := '0';

 	--Outputs
   signal I_DIR : std_logic;
   signal I_MEMW : std_logic;
   signal I_MEMR : std_logic;
   signal I_IOW : std_logic;
   signal I_IOR : std_logic;
   signal I_ALE : std_logic;
   signal RESET : std_logic;
   signal HALT : std_logic;
   signal READ_n : std_logic;
   signal CDIS : std_logic;
   signal RW0 : std_logic;
   signal nRW0 : std_logic;
   signal RW1 : std_logic;
   signal nRW1 : std_logic;
   signal DSACK0 : std_logic;
   signal DSACK1 : std_logic;
   signal STERM : std_logic;
   signal ROM : std_logic;
   signal FPU : std_logic;
   signal FLOPPY : std_logic;
   signal IDE : std_logic;
   signal KBD_CS : std_logic;
   signal IO_CS : std_logic;
   signal VGA_CS : std_logic;
   signal DRAM_CS : std_logic;
   signal ETHRNT : std_logic;
   signal BYTE0 : std_logic;
   signal BYTE1 : std_logic;
   signal BYTE2 : std_logic;
   signal BYTE3 : std_logic;

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: mux PORT MAP (
          I_DIR => I_DIR,
          I_MEMW => I_MEMW,
          I_MEMR => I_MEMR,
          I_IOW => I_IOW,
          I_IOR => I_IOR,
          I_NOWS => I_NOWS,
          I_ALE => I_ALE,
          I_IOCHRDY => I_IOCHRDY,
          R_BTN => R_BTN,
          RESET => RESET,
          HALT => HALT,
          READ_n => READ_n,
          RW_N => RW_N,
          CLK => CLK,
          ECS => ECS,
          CDIS => CDIS,
          RW0 => RW0,
          nRW0 => nRW0,
          RW1 => RW1,
          nRW1 => nRW1,
          a0 => a0,
          a1 => a1,
          a16 => a16,
          a19 => a19,
          a20 => a20,
          a21 => a21,
          a26 => a26,
          a27 => a27,
          SIZ0 => SIZ0,
          SIZ1 => SIZ1,
          FC0 => FC0,
          FC1 => FC1,
          FC2 => FC2,
          AS => AS,
          DS => DS,
          DSACK0 => DSACK0,
          DSACK1 => DSACK1,
          STERM => STERM,
          DRAM_DTACK => DRAM_DTACK,
          ROM => ROM,
          FPU => FPU,
          FLOPPY => FLOPPY,
          IDE => IDE,
          KBD_CS => KBD_CS,
          IO_CS => IO_CS,
          VGA_CS => VGA_CS,
          DRAM_CS => DRAM_CS,
          ETHRNT => ETHRNT,
          BYTE0 => BYTE0,
          BYTE1 => BYTE1,
          BYTE2 => BYTE2,
          BYTE3 => BYTE3
        );

   -- Clock process definitions
   CLK_process :process
   begin
		CLK <= '0';
		wait for CLK_period/2;
		CLK <= '1';
		wait for CLK_period/2;
   end process;
 

   -- Stimulus process
   stim_proc: process
   begin		
      -- hold reset state for 100 ns.
      wait for 100 ns;	

      wait for CLK_period*10;

      -- insert stimulus here 

      wait;
   end process;

END;

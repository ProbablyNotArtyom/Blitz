library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity mux is
  port (
	 I_DIR		: out std_logic;
	 I_MEMW		: out std_logic;
	 I_MEMR		: out std_logic;
	 I_IOW		: out std_logic;
	 I_IOR		: out std_logic;
	 I_RESET	: out std_logic;
	 I_NOWS		: in std_logic;
	 I_ALE		: out std_logic;
	 I_IOCHRDY	: in std_logic;

	 R_BTN		: in std_logic;

	 RESET		: out std_logic;
	 HALT		: out std_logic;

     RW_N 		: in	std_logic;
	 CLK		: in	std_logic;
	 ECS		: in	std_logic;
	 MMUDIS		: out	std_logic;
	 CDIS		: out	std_logic;

	 RW0 	: out	std_logic;
	 nRW0	: out	std_logic;
	 RW1 	: out	std_logic;
	 nRW1	: out	std_logic;

     a0		: in	std_logic;
	 a1		: in	std_logic;
	 a19	: in	std_logic;
	 a20	: in	std_logic;
	 a21	: in	std_logic;
	 a26	: in	std_logic;

	 SIZ0	: in	std_logic;
	 SIZ1	: in	std_logic;
	 FC0	: in	std_logic;
	 FC1	: in	std_logic;
	 FC2	: in	std_logic;
	 IPL0	: out 	std_logic;
	 IPL1	: out 	std_logic;
	 IPL2	: out 	std_logic;

	 AS	: in	std_logic;
	 DS	: in	std_logic;

	 DMA		: in std_logic;
	 DSACK0		: out std_logic;
	 DSACK1		: out std_logic;
	 STERM		: out std_logic;
	 DELAY		: in	std_logic;
	 READY		: in std_logic;
	 READ		: out std_logic;

	 LATCH		: out	std_logic;

	 ROM		: out	std_logic;
	 FPU		: out	std_logic;
	 RAM0		: out	std_logic;
	 RAM1		: out	std_logic;
	 RAM2		: out	std_logic;
	 RAM3		: out	std_logic;
	 IDE		: out 	std_logic
    );
end mux;

architecture rtl of mux is
	signal cntr			: std_logic_vector(3 downto 0);
	signal bus_32bit	: std_logic;
	signal isa_io_active	: std_logic := '1';
	signal isa_mem_active	: std_logic := '1';
	signal isa_ready		: std_logic := '1';
begin

	STERM <= '1';
	RW0 <= RW_N;
	nRW0 <= not RW_N;
	RW1 <= '1';
	nRW1 <= '1';

	RESET <= R_BTN;
	HALT <= R_BTN;
	MMUDIS <= '0';
	CDIS <= '0';

	IPL0 <= '1';
	IPL1 <= '1';
	IPL2 <= '1';

	isa_mem_active <= not (not AS and not DS and not a19 and a20 and not a21 and not a26);
	isa_io_active <= not (not AS and not DS and not a19 and a20 and a21 and not a26);

	LATCH <= (not AS and not a19 and not a20 and not a21 and not a26) and not RW_N;
	ROM <= not ((not AS and not a19 and not a20 and not a21 and not a26) and RW_N);
	FPU <= '1';
	RAM3 <= not (not AS and not DS and a26 and not a0 and not a1);
	RAM2 <= not (not AS and not DS and a26 and (
		(a0 and not a1)
		or (not a1 and not SIZ0)
		or (not a1 and SIZ1)
		));
	RAM1 <= not (not AS and not DS and a26 and (
		(not a0 and a1)
		or (not a1 and not SIZ0 and not SIZ1)
		or (not a1 and SIZ0 and SIZ1)
		or (not a1 and a0 and not SIZ0)
		));
	RAM0 <= not (not AS and not DS and a26 and (
		(a0 and a1)
		or (a0 and SIZ0 and SIZ1)
		or (not SIZ0 and not SIZ1)
		or (a1 and SIZ1)
		));
	bus_32bit <= not a26;

BUSack: process (CLK, ECS, R_BTN)
begin
	if (ECS = '0' or R_BTN = '0') then
		cntr <= "0000";
	elsif rising_edge(CLK) then
			cntr <= cntr + 1;
	end if;

	if (isa_io_active = '0' or isa_mem_active = '0') then
		DSACK0 <= isa_ready;
		DSACK1 <= '1';
	elsif (cntr >= "0011") then
		DSACK0 <= '0';
		if (bus_32bit = '0') then
			DSACK1 <= '0';
		else
			DSACK1 <= '1';
		end if;
	else
		DSACK0 <= '1';
		DSACK1 <= '1';
	end if;

end process;

ISAbus: process (ECS, R_BTN, cntr)
begin
	if (R_BTN = '0') then
		I_RESET <= '1';
		I_DIR <= '0';
		I_ALE <= '0';
		I_IOW <= '1';
		I_IOR <= '1';
		I_MEMW <= '1';
		I_MEMR <= '1';
	elsif (ECS = '0') then
		isa_ready <= '1';
		I_DIR <= '0';
		I_ALE <= '0';
		I_RESET <= '0';
		I_IOW <= '1';
		I_IOR <= '1';
		I_MEMW <= '1';
		I_MEMR <= '1';
	else
		I_RESET <= '0';
	end if;

	if (isa_io_active = '0' or isa_mem_active = '0') then
		I_DIR <= RW_N;
		if (cntr = "0010") then
			I_ALE <= '1';
		elsif (cntr = "0100") then
			I_ALE <= '0';
		elsif (cntr >= "0101" and cntr < "1100") then
			I_ALE <= '1';
			if (isa_mem_active = '0') then
				I_MEMW <= RW_N;
				I_MEMR <= not RW_N;
				I_IOW <= '1';
				I_IOR <= '1';
			elsif (isa_io_active = '0') then
				I_IOW <= RW_N;
				I_IOR <= not RW_N;
				I_MEMW <= '1';
				I_MEMR <= '1';
			end if;
			if (I_NOWS = '0' or I_IOCHRDY = '1') then
				isa_ready <= '0';
			end if ;
		elsif (cntr >= "1100") then
			isa_ready <= '0';
		end if;
	else
		I_ALE <= '0'
		I_DIR <= '0';
		I_IOW <= '1';
		I_IOR <= '1';
		I_MEMW <= '1';
		I_MEMR <= '1';
	end if;
end process;

end rtl;

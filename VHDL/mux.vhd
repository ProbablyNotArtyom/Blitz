library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity mux is
  port (
	 I_DIR		: out std_logic;
	 I_MEMW		: out std_logic;
	 I_MEMR		: out std_logic;
	 I_IOW		: out std_logic;
	 I_IOR		: out std_logic;
	 I_ALE		: out std_logic;
	 I_IOCHRDY	: in std_logic;

	 R_BTN	: in std_logic;
	 RESET	: out std_logic;
	 HALT		: out std_logic;
	 READ_n	: out std_logic;

    RW_N 	: in	std_logic;
	 CLK		: in	std_logic;
	 ECS		: in	std_logic;
	 CDIS		: out	std_logic;

	 RW0 	: out	std_logic;
	 nRW0	: out	std_logic;
	 RW1 	: out	std_logic;
	 nRW1	: out	std_logic;

    a0	: in	std_logic;
	 a1	: in	std_logic;
	 a16	: in	std_logic;
	 a19	: in	std_logic;
	 a20	: in	std_logic;
	 a21	: in	std_logic;
	 a26	: in	std_logic;
	 a27	: in	std_logic;

	 SIZ0	: in	std_logic;
	 SIZ1	: in	std_logic;
	 FC0	: in	std_logic;
	 FC1	: in	std_logic;
	 FC2	: in	std_logic;

	 AS	: in	std_logic;
	 DS	: in	std_logic;

	 DSACK0			: out std_logic;
	 DSACK1			: out std_logic;
	 STERM			: out std_logic;
	 DRAM_DTACK		: in	std_logic;

	 ROM				: out	std_logic;
	 FPU				: out	std_logic;
	 FLOPPY			: out	std_logic;
	 IDE				: out	std_logic;
	 KBD_CS			: out	std_logic;
	 IO_CS			: out	std_logic;
	 VGA_CS 			: out	std_logic;
	 DRAM_CS			: out	std_logic;
	 DRAM_CONFIG	: out	std_logic;
	 ETHRNT			: out	std_logic;

	 BYTE0			: out	std_logic;
	 BYTE1			: out	std_logic;
	 BYTE2			: out	std_logic;
	 BYTE3			: out	std_logic
    );
end mux;

architecture rtl of mux is
	signal dram_cs_async		: std_logic := '1';
	signal dram_config_sel	: std_logic;
	signal cntr					: unsigned (3 downto 0);
	signal bus_32bit			: std_logic;
	signal bus_16bit			: std_logic;
	signal isa_io_active		: std_logic := '1';
	signal isa_mem_active	: std_logic := '1';
	signal isa_ready			: std_logic := '1';
	signal wait_long			: std_logic;
	
begin

	STERM <= '1';

	RESET <= R_BTN;
	HALT <= R_BTN;
	CDIS <= '0';

	READ_n <= not RW_N;

	BYTE3 <= not (not a0 and not a1);
	BYTE2 <= not (
		(a0 and not a1)
		or (not a1 and not SIZ0)
		or (not a1 and SIZ1)
		);
	BYTE1 <= not (
		(not a0 and a1)
		or (not a1 and not SIZ0 and not SIZ1)
		or (not a1 and SIZ0 and SIZ1)
		or (not a1 and a0 and not SIZ0)
		);
	BYTE0 <= not (
		(a0 and a1)
		or (a0 and SIZ0 and SIZ1)
		or (not SIZ0 and not SIZ1)
		or (a1 and SIZ1)
		);

DRAMConfig: process(ECS, R_BTN)
begin
	if (R_BTN = '0') then
		dram_config_sel <= '0';
	elsif (dram_cs_async = '0' and rising_edge(RW_N)) then
		dram_config_sel <= '1';
	end if;
end process;
	
CSgen: process (CLK, ECS, AS, DS)
	variable a_vec : std_logic_vector(2 downto 0);
begin
	a_vec := a21 & a20 & a19;
	if (AS = '0' and not (FC0 = '1' and FC1 = '1' and FC2 = '1') and ECS = '1' and R_BTN = '1') then
		if (a27 = '0') then
			DRAM_CS <= '1';
			DRAM_CONFIG <= '1';
			dram_cs_async <= '1';
			RW0 	<= '1';
			nRW0 	<= '1';
			RW1 	<= '1';
			nRW1 	<= '1';
			bus_32bit <= '1';
			if (a26 = '0') then
				isa_io_active 		<= '1';
				isa_mem_active 		<= '1';
				case (a_vec) is
					when "000" =>
						ROM 		<= '0';
						FPU			<= '1';
						FLOPPY	<= '1';
						IDE			<= '1';
						KBD_CS	<= '1';
						IO_CS		<= '1';
						VGA_CS	<= '1';
						ETHRNT	<= '1';
						
						wait_long <= '1';
						bus_16bit <= '1';
					when "001" =>
						ROM 		<= '1';
						FPU			<= '1';
						FLOPPY	<= '0';
						IDE			<= '1';
						KBD_CS	<= '1';
						IO_CS		<= '0';
						VGA_CS	<= '1';
						ETHRNT	<= '1';
						
						wait_long <= '0';
						bus_16bit <= '1';
					when "010" =>
						ROM 		<= '1';
						FPU			<= '1';
						FLOPPY	<= '1';
						IDE			<= '0';
						KBD_CS	<= '1';
						IO_CS		<= '0';
						VGA_CS	<= '1';
						ETHRNT	<= '1';
						
						wait_long <= '0';
						if (a16 = '0') then
							bus_16bit <= '0';
						else
							bus_16bit <= '1';
						end if;
					when "011" =>
						ROM 		<= '1';
						FPU			<= '1';
						FLOPPY	<= '1';
						IDE			<= '1';
						KBD_CS	<= '0';
						IO_CS		<= '1';
						VGA_CS	<= '1';
						ETHRNT	<= '1';
						
						wait_long <= '0';
						bus_16bit <= '1';
					when "100" =>
						ROM 		<= '1';
						FPU			<= '1';
						FLOPPY	<= '1';
						IDE			<= '1';
						KBD_CS	<= '1';
						IO_CS		<= '1';
						VGA_CS	<= '0';
						ETHRNT	<= '1';
						
						wait_long <= '1';
						bus_16bit <= '1';
					when "101" =>
						ROM 		<= '1';
						FPU			<= '1';
						FLOPPY	<= '1';
						IDE			<= '1';
						KBD_CS	<= '1';
						IO_CS		<= '1';
						VGA_CS	<= '1';
						ETHRNT	<= '0';
						
						wait_long <= '1';
						bus_16bit <= '1';
					when "110" =>
						ROM 		<= '1';
						FPU			<= '1';
						FLOPPY	<= '1';
						IDE			<= '1';
						KBD_CS	<= '1';
						IO_CS		<= '1';
						VGA_CS	<= '1';
						ETHRNT	<= '1';
						
						wait_long <= '1';
						bus_16bit <= '1';
					when "111" =>
						ROM 		<= '1';
						FPU			<= '1';
						FLOPPY	<= '1';
						IDE			<= '1';
						KBD_CS	<= '1';
						IO_CS		<= '1';
						VGA_CS	<= '1';
						ETHRNT	<= '1';
						
						wait_long <= '1';
						bus_16bit <= '1';
					when others =>
						ROM 		<= '1';
						FPU			<= '1';
						FLOPPY	<= '1';
						IDE			<= '1';
						KBD_CS	<= '1';
						IO_CS		<= '1';
						VGA_CS	<= '1';
						ETHRNT	<= '1';
						
						wait_long <= '1';
						bus_16bit <= '1';
				end case;
			else
				ROM 		<= '1';
				FPU			<= '1';
				FLOPPY	<= '1';
				IDE			<= '1';
				KBD_CS	<= '1';
				IO_CS		<= '1';
				VGA_CS	<= '1';
				ETHRNT	<= '1';
				if (a21 = '0') then
					isa_io_active 		<= '0';
					isa_mem_active 		<= '1';
				else
					isa_io_active 		<= '1';
					isa_mem_active 		<= '0';
				end if;
				
				wait_long <= '1';
				bus_16bit <= '1';
			end if;
		else
			ROM 	<= '1';
			FPU		<= '1';
			FLOPPY	<= '1';
			IDE		<= '1';
			KBD_CS	<= '1';
			IO_CS	<= '1';
			VGA_CS	<= '1';
			ETHRNT	<= '1';
			isa_io_active 	<= '1';
			isa_mem_active 	<= '1';
			if (a26 = '0') then
				if (a21 = '0') then
					RW0 <= RW_N;
					nRW0 <= not RW_N;
					RW1 <= '1';
					nRW1 <= '1';
				else
					RW0 <= '1';
					nRW0 <= '1';
					RW1 <= RW_N;
					nRW1 <= not RW_N;
				end if;
				DRAM_CS <= '1';
				DRAM_CONFIG <= '1';
				dram_cs_async <= '1';
				
				wait_long <= '1';
			else
				RW0 	<= '1';
				nRW0 	<= '1';
				RW1 	<= '1';
				nRW1 	<= '1';
				if (dram_config_sel = '0') then
					DRAM_CS <= '1';
					DRAM_CONFIG <= '0';
					dram_cs_async <= '0';
					wait_long <= '0';
				else
					DRAM_CS <= '0';
					DRAM_CONFIG <= '1';
					dram_cs_async <= '0';
					wait_long <= '1';
				end if;
			end if;
			
			bus_32bit <= '0';
			bus_16bit <= '1';
		end if;
	elsif (FC0 = '1' and FC1 = '1' and FC2 = '1') and a16 = '0' then
		ROM 		<= '1';
		FPU			<= '0';
		FLOPPY	<= '1';
		IDE			<= '1';
		KBD_CS	<= '1';
		IO_CS		<= '1';
		VGA_CS	<= '1';
		DRAM_CS	<= '1';
		DRAM_CONFIG	<= '1';
		dram_cs_async <= '1';
		ETHRNT	<= '1';
		isa_io_active 		<= '1';
		isa_mem_active 	<= '1';

		RW0 	<= '1';
		nRW0 	<= '1';
		RW1 	<= '1';
		nRW1 	<= '1';
		
		bus_32bit <= '1';
		bus_16bit <= '1';
		wait_long <= '1';
	else
		ROM 		<= '1';
		FPU			<= '1';
		FLOPPY	<= '1';
		IDE			<= '1';
		KBD_CS	<= '1';
		IO_CS		<= '1';
		VGA_CS	<= '1';
		DRAM_CS	<= '1';
		DRAM_CONFIG	<= '1';
		dram_cs_async <= '1';
		ETHRNT	<= '1';
		isa_io_active 		<= '1';
		isa_mem_active 	<= '1';

		RW0 	<= '1';
		nRW0 	<= '1';
		RW1 	<= '1';
		nRW1 	<= '1';
		
		bus_32bit <= '1';
		bus_16bit <= '1';
		wait_long <= '1';
	end if;
end process;

BUSack: process (CLK, ECS, R_BTN)
begin
	if (ECS = '0' or R_BTN = '0') then
		cntr <= "0000";
	elsif falling_edge(CLK) then
			cntr <= cntr + 1;
	end if;

	if (isa_io_active = '0' or isa_mem_active = '0') then
		DSACK0 <= isa_ready;
		DSACK1 <= '1';
	elsif (dram_cs_async = '0' and dram_config_sel = '1') then
		DSACK0 <= DRAM_DTACK;
		DSACK1 <= DRAM_DTACK;
	elsif (cntr >= "0011" and wait_long = '1') or 
			(cntr >= "1100" and wait_long = '0') then
		if (bus_32bit = '0') then
			DSACK0 <= '0';
			DSACK1 <= '0';
		elsif (bus_16bit = '0') then
			DSACK0 <= '1';
			DSACK1 <= '0';
		else
			DSACK0 <= '0';
			DSACK1 <= '1';
		end if;
	else
		DSACK0 <= '1';
		DSACK1 <= '1';
	end if;

end process;

ISAbus: process (ECS, cntr)
begin
	if (isa_io_active = '0' or isa_mem_active = '0') then
		I_DIR <= RW_N;
		case cntr is
			when "0000" | "0001" | "0010" =>
				isa_ready <= '1';
				I_ALE <= '1';
				I_IOW <= '1';
				I_IOR <= '1';
				I_MEMW <= '1';
				I_MEMR <= '1';
			when "0011" | "0100" | "0101" =>
				isa_ready <= '1';
				I_ALE <= '0';
				I_IOW <= '1';
				I_IOR <= '1';
				I_MEMW <= '1';
				I_MEMR <= '1';
			when "0110" | "0111" | "1000" | "1001" | "1010" | "1011" | "1100" =>
				isa_ready <= '1';
				I_ALE <= '0';
				if (isa_mem_active = '0') then
					I_MEMW <= RW_N;
					I_MEMR <= not RW_N;
					I_IOW <= '1';
					I_IOR <= '1';
				else
					I_IOW <= RW_N;
					I_IOR <= not RW_N;
					I_MEMW <= '1';
					I_MEMR <= '1';
				end if;
			when others =>
				I_ALE <= '0';
				if (isa_mem_active = '0') then
					I_MEMW <= RW_N;
					I_MEMR <= not RW_N;
					I_IOW <= '1';
					I_IOR <= '1';
				else
					I_IOW <= RW_N;
					I_IOR <= not RW_N;
					I_MEMW <= '1';
					I_MEMR <= '1';
				end if;
				if (I_IOCHRDY = '0') then
					isa_ready <= '1';
				else
					isa_ready <= '0';
				end if;
		end case;
	else
		isa_ready <= '1';
		I_ALE <= '0';
		I_DIR <= '0';
		I_IOW <= '1';
		I_IOR <= '1';
		I_MEMW <= '1';
		I_MEMR <= '1';
	end if;
end process;

end rtl;

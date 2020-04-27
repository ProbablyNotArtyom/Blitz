
-- VHDL Instantiation Created from source file mux.vhd -- 05:33:46 04/19/2019
--
-- Notes: 
-- 1) This instantiation template has been automatically generated using types
-- std_logic and std_logic_vector for the ports of the instantiated module
-- 2) To use this template to instantiate this entity, cut-and-paste and then edit

	COMPONENT mux
	PORT(
		I_NOWS : IN std_logic;
		I_IOCHRDY : IN std_logic;
		R_BTN : IN std_logic;
		RW_N : IN std_logic;
		CLK : IN std_logic;
		ECS : IN std_logic;
		a0 : IN std_logic;
		a1 : IN std_logic;
		a16 : IN std_logic;
		a19 : IN std_logic;
		a20 : IN std_logic;
		a21 : IN std_logic;
		a26 : IN std_logic;
		a27 : IN std_logic;
		SIZ0 : IN std_logic;
		SIZ1 : IN std_logic;
		FC0 : IN std_logic;
		FC1 : IN std_logic;
		FC2 : IN std_logic;
		AS : IN std_logic;
		DS : IN std_logic;
		DRAM_DTACK : IN std_logic;          
		I_DIR : OUT std_logic;
		I_MEMW : OUT std_logic;
		I_MEMR : OUT std_logic;
		I_IOW : OUT std_logic;
		I_IOR : OUT std_logic;
		I_ALE : OUT std_logic;
		RESET : OUT std_logic;
		HALT : OUT std_logic;
		READ_n : OUT std_logic;
		CDIS : OUT std_logic;
		RW0 : OUT std_logic;
		nRW0 : OUT std_logic;
		RW1 : OUT std_logic;
		nRW1 : OUT std_logic;
		IPL0 : OUT std_logic;
		IPL1 : OUT std_logic;
		IPL2 : OUT std_logic;
		DSACK0 : OUT std_logic;
		DSACK1 : OUT std_logic;
		STERM : OUT std_logic;
		ROM : OUT std_logic;
		FPU : OUT std_logic;
		FLOPPY : OUT std_logic;
		IDE : OUT std_logic;
		KBD_CS : OUT std_logic;
		IO_CS : OUT std_logic;
		VGA_CS : OUT std_logic;
		DRAM_CS : OUT std_logic;
		ETHRNT : OUT std_logic;
		BYTE0 : OUT std_logic;
		BYTE1 : OUT std_logic;
		BYTE2 : OUT std_logic;
		BYTE3 : OUT std_logic
		);
	END COMPONENT;

	Inst_mux: mux PORT MAP(
		I_DIR => ,
		I_MEMW => ,
		I_MEMR => ,
		I_IOW => ,
		I_IOR => ,
		I_NOWS => ,
		I_ALE => ,
		I_IOCHRDY => ,
		R_BTN => ,
		RESET => ,
		HALT => ,
		READ_n => ,
		RW_N => ,
		CLK => ,
		ECS => ,
		CDIS => ,
		RW0 => ,
		nRW0 => ,
		RW1 => ,
		nRW1 => ,
		a0 => ,
		a1 => ,
		a16 => ,
		a19 => ,
		a20 => ,
		a21 => ,
		a26 => ,
		a27 => ,
		SIZ0 => ,
		SIZ1 => ,
		FC0 => ,
		FC1 => ,
		FC2 => ,
		IPL0 => ,
		IPL1 => ,
		IPL2 => ,
		AS => ,
		DS => ,
		DSACK0 => ,
		DSACK1 => ,
		STERM => ,
		DRAM_DTACK => ,
		ROM => ,
		FPU => ,
		FLOPPY => ,
		IDE => ,
		KBD_CS => ,
		IO_CS => ,
		VGA_CS => ,
		DRAM_CS => ,
		ETHRNT => ,
		BYTE0 => ,
		BYTE1 => ,
		BYTE2 => ,
		BYTE3 => 
	);



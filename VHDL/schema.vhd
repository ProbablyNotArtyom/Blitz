--------------------------------------------------------------------------------
-- Copyright (c) 1995-2013 Xilinx, Inc.  All rights reserved.
--------------------------------------------------------------------------------
--   ____  ____ 
--  /   /\/   / 
-- /___/  \  /    Vendor: Xilinx 
-- \   \   \/     Version : 14.7
--  \   \         Application : sch2hdl
--  /   /         Filename : schema.vhf
-- /___/   /\     Timestamp : 04/19/2019 07:10:43
-- \   \  /  \ 
--  \___\/\___\ 
--
--Command: sch2hdl -intstyle ise -family xc9500xl -flat -suppress -vhdl /home/artyom/Desktop/Projects/BBlitz/VHDL/Mux_actual/schema.vhf -w /home/artyom/Desktop/Projects/BBlitz/VHDL/Mux_actual/schema.sch
--Design Name: schema
--Device: xc9500xl
--Purpose:
--    This vhdl netlist is translated from an ECS schematic. It can be 
--    synthesized and simulated, but it should not be modified. 
--

library ieee;
use ieee.std_logic_1164.ALL;
use ieee.numeric_std.ALL;
library UNISIM;
use UNISIM.Vcomponents.ALL;

entity D2_4E_MXILINX_schema is
   port ( A0 : in    std_logic; 
          A1 : in    std_logic; 
          E  : in    std_logic; 
          D0 : out   std_logic; 
          D1 : out   std_logic; 
          D2 : out   std_logic; 
          D3 : out   std_logic);
end D2_4E_MXILINX_schema;

architecture BEHAVIORAL of D2_4E_MXILINX_schema is
   attribute BOX_TYPE   : string ;
   component AND3
      port ( I0 : in    std_logic; 
             I1 : in    std_logic; 
             I2 : in    std_logic; 
             O  : out   std_logic);
   end component;
   attribute BOX_TYPE of AND3 : component is "BLACK_BOX";
   
   component AND3B1
      port ( I0 : in    std_logic; 
             I1 : in    std_logic; 
             I2 : in    std_logic; 
             O  : out   std_logic);
   end component;
   attribute BOX_TYPE of AND3B1 : component is "BLACK_BOX";
   
   component AND3B2
      port ( I0 : in    std_logic; 
             I1 : in    std_logic; 
             I2 : in    std_logic; 
             O  : out   std_logic);
   end component;
   attribute BOX_TYPE of AND3B2 : component is "BLACK_BOX";
   
begin
   I_36_30 : AND3
      port map (I0=>A1,
                I1=>A0,
                I2=>E,
                O=>D3);
   
   I_36_31 : AND3B1
      port map (I0=>A0,
                I1=>A1,
                I2=>E,
                O=>D2);
   
   I_36_32 : AND3B1
      port map (I0=>A1,
                I1=>A0,
                I2=>E,
                O=>D1);
   
   I_36_33 : AND3B2
      port map (I0=>A0,
                I1=>A1,
                I2=>E,
                O=>D0);
   
end BEHAVIORAL;



library ieee;
use ieee.std_logic_1164.ALL;
use ieee.numeric_std.ALL;
library UNISIM;
use UNISIM.Vcomponents.ALL;

entity D3_8E_MXILINX_schema is
   port ( A0 : in    std_logic; 
          A1 : in    std_logic; 
          A2 : in    std_logic; 
          E  : in    std_logic; 
          D0 : out   std_logic; 
          D1 : out   std_logic; 
          D2 : out   std_logic; 
          D3 : out   std_logic; 
          D4 : out   std_logic; 
          D5 : out   std_logic; 
          D6 : out   std_logic; 
          D7 : out   std_logic);
end D3_8E_MXILINX_schema;

architecture BEHAVIORAL of D3_8E_MXILINX_schema is
   attribute BOX_TYPE   : string ;
   component AND4
      port ( I0 : in    std_logic; 
             I1 : in    std_logic; 
             I2 : in    std_logic; 
             I3 : in    std_logic; 
             O  : out   std_logic);
   end component;
   attribute BOX_TYPE of AND4 : component is "BLACK_BOX";
   
   component AND4B1
      port ( I0 : in    std_logic; 
             I1 : in    std_logic; 
             I2 : in    std_logic; 
             I3 : in    std_logic; 
             O  : out   std_logic);
   end component;
   attribute BOX_TYPE of AND4B1 : component is "BLACK_BOX";
   
   component AND4B2
      port ( I0 : in    std_logic; 
             I1 : in    std_logic; 
             I2 : in    std_logic; 
             I3 : in    std_logic; 
             O  : out   std_logic);
   end component;
   attribute BOX_TYPE of AND4B2 : component is "BLACK_BOX";
   
   component AND4B3
      port ( I0 : in    std_logic; 
             I1 : in    std_logic; 
             I2 : in    std_logic; 
             I3 : in    std_logic; 
             O  : out   std_logic);
   end component;
   attribute BOX_TYPE of AND4B3 : component is "BLACK_BOX";
   
begin
   I_36_30 : AND4
      port map (I0=>A2,
                I1=>A1,
                I2=>A0,
                I3=>E,
                O=>D7);
   
   I_36_31 : AND4B1
      port map (I0=>A0,
                I1=>A2,
                I2=>A1,
                I3=>E,
                O=>D6);
   
   I_36_32 : AND4B1
      port map (I0=>A1,
                I1=>A2,
                I2=>A0,
                I3=>E,
                O=>D5);
   
   I_36_33 : AND4B2
      port map (I0=>A1,
                I1=>A0,
                I2=>A2,
                I3=>E,
                O=>D4);
   
   I_36_34 : AND4B1
      port map (I0=>A2,
                I1=>A0,
                I2=>A1,
                I3=>E,
                O=>D3);
   
   I_36_35 : AND4B2
      port map (I0=>A2,
                I1=>A0,
                I2=>A1,
                I3=>E,
                O=>D2);
   
   I_36_36 : AND4B2
      port map (I0=>A2,
                I1=>A1,
                I2=>A0,
                I3=>E,
                O=>D1);
   
   I_36_37 : AND4B3
      port map (I0=>A2,
                I1=>A1,
                I2=>A0,
                I3=>E,
                O=>D0);
   
end BEHAVIORAL;



library ieee;
use ieee.std_logic_1164.ALL;
use ieee.numeric_std.ALL;
library UNISIM;
use UNISIM.Vcomponents.ALL;

entity schema is
   port ( AS       : in    std_logic; 
          a16      : in    std_logic; 
          a19      : in    std_logic; 
          a20      : in    std_logic; 
          a21      : in    std_logic; 
          a26      : in    std_logic; 
          a27      : in    std_logic; 
          FC0      : in    std_logic; 
          FC1      : in    std_logic; 
          FC2      : in    std_logic; 
          XLXN_307 : in    std_logic; 
          DRAM_CS  : out   std_logic; 
          ETHRNT   : out   std_logic; 
          FLOPPY   : out   std_logic; 
          FPU      : out   std_logic; 
          IDE      : out   std_logic; 
          IO_CS    : out   std_logic; 
          KBD_CS   : out   std_logic; 
          nRW0     : out   std_logic; 
          nRW1     : out   std_logic; 
          ROM      : out   std_logic; 
          RW0      : out   std_logic; 
          RW1      : out   std_logic; 
          VGA_CS   : out   std_logic);
end schema;

architecture BEHAVIORAL of schema is
   attribute HU_SET     : string ;
   attribute BOX_TYPE   : string ;
   signal XLXN_140     : std_logic;
   signal XLXN_141     : std_logic;
   signal XLXN_142     : std_logic;
   signal XLXN_143     : std_logic;
   signal XLXN_144     : std_logic;
   signal XLXN_145     : std_logic;
   signal XLXN_146     : std_logic;
   signal XLXN_147     : std_logic;
   signal XLXN_174     : std_logic;
   signal XLXN_176     : std_logic;
   signal XLXN_177     : std_logic;
   signal XLXN_212     : std_logic;
   signal XLXN_232     : std_logic;
   signal XLXN_250     : std_logic;
   signal XLXN_254     : std_logic;
   signal XLXN_299     : std_logic;
   signal XLXN_309     : std_logic;
   signal XLXN_312     : std_logic;
   signal IDE_DUMMY    : std_logic;
   signal FLOPPY_DUMMY : std_logic;
   component D3_8E_MXILINX_schema
      port ( A0 : in    std_logic; 
             A1 : in    std_logic; 
             A2 : in    std_logic; 
             E  : in    std_logic; 
             D0 : out   std_logic; 
             D1 : out   std_logic; 
             D2 : out   std_logic; 
             D3 : out   std_logic; 
             D4 : out   std_logic; 
             D5 : out   std_logic; 
             D6 : out   std_logic; 
             D7 : out   std_logic);
   end component;
   
   component INV
      port ( I : in    std_logic; 
             O : out   std_logic);
   end component;
   attribute BOX_TYPE of INV : component is "BLACK_BOX";
   
   component D2_4E_MXILINX_schema
      port ( A0 : in    std_logic; 
             A1 : in    std_logic; 
             E  : in    std_logic; 
             D0 : out   std_logic; 
             D1 : out   std_logic; 
             D2 : out   std_logic; 
             D3 : out   std_logic);
   end component;
   
   component OR2
      port ( I0 : in    std_logic; 
             I1 : in    std_logic; 
             O  : out   std_logic);
   end component;
   attribute BOX_TYPE of OR2 : component is "BLACK_BOX";
   
   component NAND2
      port ( I0 : in    std_logic; 
             I1 : in    std_logic; 
             O  : out   std_logic);
   end component;
   attribute BOX_TYPE of NAND2 : component is "BLACK_BOX";
   
   component AND2
      port ( I0 : in    std_logic; 
             I1 : in    std_logic; 
             O  : out   std_logic);
   end component;
   attribute BOX_TYPE of AND2 : component is "BLACK_BOX";
   
   component NAND3
      port ( I0 : in    std_logic; 
             I1 : in    std_logic; 
             I2 : in    std_logic; 
             O  : out   std_logic);
   end component;
   attribute BOX_TYPE of NAND3 : component is "BLACK_BOX";
   
   component OR3
      port ( I0 : in    std_logic; 
             I1 : in    std_logic; 
             I2 : in    std_logic; 
             O  : out   std_logic);
   end component;
   attribute BOX_TYPE of OR3 : component is "BLACK_BOX";
   
   attribute HU_SET of XLXI_10 : label is "XLXI_10_0";
   attribute HU_SET of XLXI_21 : label is "XLXI_21_1";
begin
   FLOPPY <= FLOPPY_DUMMY;
   IDE <= IDE_DUMMY;
   XLXI_10 : D3_8E_MXILINX_schema
      port map (A0=>a19,
                A1=>a20,
                A2=>a21,
                E=>XLXN_174,
                D0=>XLXN_140,
                D1=>XLXN_141,
                D2=>XLXN_142,
                D3=>XLXN_143,
                D4=>XLXN_144,
                D5=>XLXN_145,
                D6=>XLXN_146,
                D7=>XLXN_147);
   
   XLXI_11 : INV
      port map (I=>XLXN_140,
                O=>ROM);
   
   XLXI_12 : INV
      port map (I=>XLXN_141,
                O=>FLOPPY_DUMMY);
   
   XLXI_13 : INV
      port map (I=>XLXN_142,
                O=>IDE_DUMMY);
   
   XLXI_14 : INV
      port map (I=>XLXN_143,
                O=>KBD_CS);
   
   XLXI_15 : INV
      port map (I=>XLXN_144,
                O=>VGA_CS);
   
   XLXI_16 : INV
      port map (I=>XLXN_145,
                O=>ETHRNT);
   
   XLXI_21 : D2_4E_MXILINX_schema
      port map (A0=>a26,
                A1=>a27,
                E=>XLXN_212,
                D0=>XLXN_174,
                D1=>XLXN_254,
                D2=>XLXN_176,
                D3=>XLXN_177);
   
   XLXI_24 : INV
      port map (I=>XLXN_176,
                O=>DRAM_CS);
   
   XLXI_26 : INV
      port map (I=>AS,
                O=>XLXN_212);
   
   XLXI_27 : OR2
      port map (I0=>XLXN_312,
                I1=>XLXN_307,
                O=>RW0);
   
   XLXI_28 : OR2
      port map (I0=>XLXN_232,
                I1=>XLXN_307,
                O=>RW1);
   
   XLXI_29 : INV
      port map (I=>XLXN_307,
                O=>XLXN_309);
   
   XLXI_30 : OR2
      port map (I0=>XLXN_312,
                I1=>XLXN_309,
                O=>nRW0);
   
   XLXI_31 : OR2
      port map (I0=>XLXN_232,
                I1=>XLXN_309,
                O=>nRW1);
   
   XLXI_43 : INV
      port map (I=>a21,
                O=>XLXN_250);
   
   XLXI_45 : NAND2
      port map (I0=>XLXN_254,
                I1=>a21,
                O=>XLXN_312);
   
   XLXI_46 : NAND2
      port map (I0=>XLXN_254,
                I1=>XLXN_250,
                O=>XLXN_232);
   
   XLXI_60 : AND2
      port map (I0=>IDE_DUMMY,
                I1=>FLOPPY_DUMMY,
                O=>IO_CS);
   
   XLXI_63 : NAND3
      port map (I0=>FC2,
                I1=>FC1,
                I2=>FC0,
                O=>XLXN_299);
   
   XLXI_67 : OR3
      port map (I0=>XLXN_299,
                I1=>AS,
                I2=>a16,
                O=>FPU);
   
end BEHAVIORAL;



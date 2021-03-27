--------------------------------------------------------------------------------
-- Copyright (c) 1995-2013 Xilinx, Inc.  All rights reserved.
--------------------------------------------------------------------------------
--   ____  ____ 
--  /   /\/   / 
-- /___/  \  /    Vendor: Xilinx 
-- \   \   \/     Version : 14.7
--  \   \         Application : sch2hdl
--  /   /         Filename : schema.vhf
-- /___/   /\     Timestamp : 04/19/2019 08:10:31
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

entity schema is
   port ( AS             : in    std_logic; 
          a16            : in    std_logic; 
          a19            : in    std_logic; 
          a20            : in    std_logic; 
          a21            : in    std_logic; 
          a26            : in    std_logic; 
          a27            : in    std_logic; 
          FC0            : in    std_logic; 
          FC1            : in    std_logic; 
          FC2            : in    std_logic; 
          RW_N           : in    std_logic; 
          DRAM_CS        : out   std_logic; 
          ETHRNT         : out   std_logic; 
          FLOPPY         : out   std_logic; 
          FPU            : out   std_logic; 
          IDE            : out   std_logic; 
          IO_CS          : out   std_logic; 
          isa_mem_active : out   std_logic; 
          KBD_CS         : out   std_logic; 
          nRW0           : out   std_logic; 
          nRW1           : out   std_logic; 
          ROM            : out   std_logic; 
          RW0            : out   std_logic; 
          RW1            : out   std_logic; 
          VGA_CS         : out   std_logic);
end schema;

architecture BEHAVIORAL of schema is
   attribute VhdlModel  : string ;
   attribute VeriModel  : string ;
   attribute Level      : string ;
   attribute Device     : string ;
   signal XLXN_140              : std_logic;
   signal XLXN_141              : std_logic;
   signal XLXN_142              : std_logic;
   signal XLXN_143              : std_logic;
   signal XLXN_144              : std_logic;
   signal XLXN_145              : std_logic;
   signal XLXN_146              : std_logic;
   signal XLXN_147              : std_logic;
   signal XLXN_174              : std_logic;
   signal XLXN_176              : std_logic;
   signal XLXN_177              : std_logic;
   signal XLXN_212              : std_logic;
   signal XLXN_232              : std_logic;
   signal XLXN_250              : std_logic;
   signal XLXN_254              : std_logic;
   signal XLXN_299              : std_logic;
   signal XLXN_309              : std_logic;
   signal XLXN_312              : std_logic;
   signal XLXI_67_A0_openSignal : std_logic;
   signal XLXI_67_A1_openSignal : std_logic;
   signal XLXI_67_A2_openSignal : std_logic;
   signal XLXI_67_E_openSignal  : std_logic;
   signal XLXI_74_A0_openSignal : std_logic;
   signal XLXI_74_A1_openSignal : std_logic;
   signal XLXI_74_A2_openSignal : std_logic;
   signal XLXI_74_E_openSignal  : std_logic;
   signal XLXI_75_A2_openSignal : std_logic;
   signal XLXI_76_A0_openSignal : std_logic;
   signal XLXI_76_A1_openSignal : std_logic;
   signal XLXI_76_A2_openSignal : std_logic;
   signal XLXI_76_E_openSignal  : std_logic;
   signal XLXI_77_A0_openSignal : std_logic;
   signal XLXI_77_A1_openSignal : std_logic;
   signal XLXI_77_A2_openSignal : std_logic;
   signal XLXI_77_E_openSignal  : std_logic;
   signal XLXI_79_A0_openSignal : std_logic;
   signal XLXI_79_A1_openSignal : std_logic;
   signal XLXI_79_A2_openSignal : std_logic;
   signal XLXI_79_E_openSignal  : std_logic;
   signal XLXI_80_A0_openSignal : std_logic;
   signal XLXI_80_A1_openSignal : std_logic;
   signal XLXI_80_A2_openSignal : std_logic;
   signal XLXI_80_E_openSignal  : std_logic;
   signal XLXI_82_A0_openSignal : std_logic;
   signal XLXI_82_A1_openSignal : std_logic;
   signal XLXI_82_A2_openSignal : std_logic;
   signal XLXI_82_E_openSignal  : std_logic;
   signal XLXI_83_A0_openSignal : std_logic;
   signal XLXI_83_A1_openSignal : std_logic;
   signal XLXI_83_A2_openSignal : std_logic;
   signal XLXI_83_E_openSignal  : std_logic;
   signal XLXI_84_A0_openSignal : std_logic;
   signal XLXI_84_A1_openSignal : std_logic;
   signal XLXI_84_A2_openSignal : std_logic;
   signal XLXI_84_E_openSignal  : std_logic;
   signal XLXI_85_A0_openSignal : std_logic;
   signal XLXI_85_A1_openSignal : std_logic;
   signal XLXI_85_A2_openSignal : std_logic;
   signal XLXI_85_E_openSignal  : std_logic;
   signal XLXI_86_A0_openSignal : std_logic;
   signal XLXI_86_A1_openSignal : std_logic;
   signal XLXI_86_A2_openSignal : std_logic;
   signal XLXI_86_E_openSignal  : std_logic;
   signal XLXI_87_A0_openSignal : std_logic;
   signal XLXI_87_A1_openSignal : std_logic;
   signal XLXI_87_A2_openSignal : std_logic;
   signal XLXI_87_E_openSignal  : std_logic;
   signal XLXI_88_A0_openSignal : std_logic;
   signal XLXI_88_A1_openSignal : std_logic;
   signal XLXI_88_A2_openSignal : std_logic;
   signal XLXI_88_E_openSignal  : std_logic;
   signal XLXI_89_A0_openSignal : std_logic;
   signal XLXI_89_A1_openSignal : std_logic;
   signal XLXI_89_A2_openSignal : std_logic;
   signal XLXI_89_E_openSignal  : std_logic;
   signal XLXI_90_A0_openSignal : std_logic;
   signal XLXI_90_A1_openSignal : std_logic;
   signal XLXI_90_A2_openSignal : std_logic;
   signal XLXI_90_E_openSignal  : std_logic;
   signal XLXI_91_A0_openSignal : std_logic;
   signal XLXI_91_A1_openSignal : std_logic;
   signal XLXI_91_A2_openSignal : std_logic;
   signal XLXI_91_E_openSignal  : std_logic;
   signal XLXI_92_A0_openSignal : std_logic;
   signal XLXI_92_A1_openSignal : std_logic;
   signal XLXI_92_A2_openSignal : std_logic;
   signal XLXI_92_E_openSignal  : std_logic;
   signal XLXI_93_A0_openSignal : std_logic;
   signal XLXI_93_A1_openSignal : std_logic;
   signal XLXI_93_A2_openSignal : std_logic;
   signal XLXI_93_E_openSignal  : std_logic;
   signal XLXI_94_A0_openSignal : std_logic;
   signal XLXI_94_A1_openSignal : std_logic;
   signal XLXI_94_A2_openSignal : std_logic;
   signal XLXI_94_E_openSignal  : std_logic;
   signal XLXI_95_A0_openSignal : std_logic;
   signal XLXI_95_A1_openSignal : std_logic;
   signal XLXI_95_A2_openSignal : std_logic;
   signal XLXI_95_E_openSignal  : std_logic;
   component INV
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
   
   attribute VhdlModel of XLXI_10 : label is "INV";
   attribute VeriModel of XLXI_10 : label is "INV";
   attribute Level of XLXI_10 : label is "XILINX";
   attribute VhdlModel of XLXI_67 : label is "INV";
   attribute VeriModel of XLXI_67 : label is "INV";
   attribute Device of XLXI_67 : label is "INV";
   attribute VhdlModel of XLXI_74 : label is "INV";
   attribute VeriModel of XLXI_74 : label is "INV";
   attribute Device of XLXI_74 : label is "INV";
   attribute VhdlModel of XLXI_75 : label is "INV";
   attribute VeriModel of XLXI_75 : label is "INV";
   attribute Level of XLXI_75 : label is "XILINX";
   attribute VhdlModel of XLXI_76 : label is "INV";
   attribute VeriModel of XLXI_76 : label is "INV";
   attribute Device of XLXI_76 : label is "INV";
   attribute VhdlModel of XLXI_77 : label is "INV";
   attribute VeriModel of XLXI_77 : label is "INV";
   attribute Device of XLXI_77 : label is "INV";
   attribute VhdlModel of XLXI_79 : label is "INV";
   attribute VeriModel of XLXI_79 : label is "INV";
   attribute Device of XLXI_79 : label is "INV";
   attribute VhdlModel of XLXI_80 : label is "INV";
   attribute VeriModel of XLXI_80 : label is "INV";
   attribute Device of XLXI_80 : label is "INV";
   attribute VhdlModel of XLXI_82 : label is "INV";
   attribute VeriModel of XLXI_82 : label is "INV";
   attribute Device of XLXI_82 : label is "INV";
   attribute VhdlModel of XLXI_83 : label is "INV";
   attribute VeriModel of XLXI_83 : label is "INV";
   attribute Device of XLXI_83 : label is "INV";
   attribute VhdlModel of XLXI_84 : label is "INV";
   attribute VeriModel of XLXI_84 : label is "INV";
   attribute Device of XLXI_84 : label is "INV";
begin
   XLXI_10 : INV
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
   
   XLXI_67 : INV
      port map (A0=>XLXI_67_A0_openSignal,
                A1=>XLXI_67_A1_openSignal,
                A2=>XLXI_67_A2_openSignal,
                E=>XLXI_67_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_74 : INV
      port map (A0=>XLXI_74_A0_openSignal,
                A1=>XLXI_74_A1_openSignal,
                A2=>XLXI_74_A2_openSignal,
                E=>XLXI_74_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_75 : INV
      port map (A0=>a26,
                A1=>a27,
                A2=>XLXI_75_A2_openSignal,
                E=>XLXN_212,
                D0=>XLXN_174,
                D1=>XLXN_254,
                D2=>XLXN_176,
                D3=>XLXN_177,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_76 : INV
      port map (A0=>XLXI_76_A0_openSignal,
                A1=>XLXI_76_A1_openSignal,
                A2=>XLXI_76_A2_openSignal,
                E=>XLXI_76_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_77 : INV
      port map (A0=>XLXI_77_A0_openSignal,
                A1=>XLXI_77_A1_openSignal,
                A2=>XLXI_77_A2_openSignal,
                E=>XLXI_77_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_79 : INV
      port map (A0=>XLXI_79_A0_openSignal,
                A1=>XLXI_79_A1_openSignal,
                A2=>XLXI_79_A2_openSignal,
                E=>XLXI_79_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_80 : INV
      port map (A0=>XLXI_80_A0_openSignal,
                A1=>XLXI_80_A1_openSignal,
                A2=>XLXI_80_A2_openSignal,
                E=>XLXI_80_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_82 : INV
      port map (A0=>XLXI_82_A0_openSignal,
                A1=>XLXI_82_A1_openSignal,
                A2=>XLXI_82_A2_openSignal,
                E=>XLXI_82_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_83 : INV
      port map (A0=>XLXI_83_A0_openSignal,
                A1=>XLXI_83_A1_openSignal,
                A2=>XLXI_83_A2_openSignal,
                E=>XLXI_83_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_84 : INV
      port map (A0=>XLXI_84_A0_openSignal,
                A1=>XLXI_84_A1_openSignal,
                A2=>XLXI_84_A2_openSignal,
                E=>XLXI_84_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_85 : INV
      port map (A0=>XLXI_85_A0_openSignal,
                A1=>XLXI_85_A1_openSignal,
                A2=>XLXI_85_A2_openSignal,
                E=>XLXI_85_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_86 : INV
      port map (A0=>XLXI_86_A0_openSignal,
                A1=>XLXI_86_A1_openSignal,
                A2=>XLXI_86_A2_openSignal,
                E=>XLXI_86_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_87 : INV
      port map (A0=>XLXI_87_A0_openSignal,
                A1=>XLXI_87_A1_openSignal,
                A2=>XLXI_87_A2_openSignal,
                E=>XLXI_87_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_88 : INV
      port map (A0=>XLXI_88_A0_openSignal,
                A1=>XLXI_88_A1_openSignal,
                A2=>XLXI_88_A2_openSignal,
                E=>XLXI_88_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_89 : INV
      port map (A0=>XLXI_89_A0_openSignal,
                A1=>XLXI_89_A1_openSignal,
                A2=>XLXI_89_A2_openSignal,
                E=>XLXI_89_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_90 : INV
      port map (A0=>XLXI_90_A0_openSignal,
                A1=>XLXI_90_A1_openSignal,
                A2=>XLXI_90_A2_openSignal,
                E=>XLXI_90_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_91 : INV
      port map (A0=>XLXI_91_A0_openSignal,
                A1=>XLXI_91_A1_openSignal,
                A2=>XLXI_91_A2_openSignal,
                E=>XLXI_91_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_92 : INV
      port map (A0=>XLXI_92_A0_openSignal,
                A1=>XLXI_92_A1_openSignal,
                A2=>XLXI_92_A2_openSignal,
                E=>XLXI_92_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_93 : INV
      port map (A0=>XLXI_93_A0_openSignal,
                A1=>XLXI_93_A1_openSignal,
                A2=>XLXI_93_A2_openSignal,
                E=>XLXI_93_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_94 : INV
      port map (A0=>XLXI_94_A0_openSignal,
                A1=>XLXI_94_A1_openSignal,
                A2=>XLXI_94_A2_openSignal,
                E=>XLXI_94_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
   XLXI_95 : INV
      port map (A0=>XLXI_95_A0_openSignal,
                A1=>XLXI_95_A1_openSignal,
                A2=>XLXI_95_A2_openSignal,
                E=>XLXI_95_E_openSignal,
                D0=>open,
                D1=>open,
                D2=>open,
                D3=>open,
                D4=>open,
                D5=>open,
                D6=>open,
                D7=>open);
   
end BEHAVIORAL;



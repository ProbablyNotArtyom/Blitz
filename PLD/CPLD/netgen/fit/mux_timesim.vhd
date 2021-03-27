--------------------------------------------------------------------------------
-- Copyright (c) 1995-2013 Xilinx, Inc.  All rights reserved.
--------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /    Vendor: Xilinx
-- \   \   \/     Version: P.20131013
--  \   \         Application: netgen
--  /   /         Filename: mux_timesim.vhd
-- /___/   /\     Timestamp: Fri Apr 19 09:40:01 2019
-- \   \  /  \ 
--  \___\/\___\
--             
-- Command	: -intstyle ise -rpw 100 -ar Structure -tm mux -w -dir netgen/fit -ofmt vhdl -sim mux.nga mux_timesim.vhd 
-- Device	: XC9572XL-5-VQ64 (Speed File: Version 3.0)
-- Input file	: mux.nga
-- Output file	: /home/artyom/Desktop/Projects/BBlitz/VHDL/Mux_actual/netgen/fit/mux_timesim.vhd
-- # of Entities	: 1
-- Design Name	: mux.nga
-- Xilinx	: /opt/Xilinx/14.7/ISE_DS/ISE/
--             
-- Purpose:    
--     This VHDL netlist is a verification model and uses simulation 
--     primitives which may not represent the true implementation of the 
--     device, however the netlist is functionally correct and should not 
--     be modified. This file cannot be synthesized and should only be used 
--     with supported simulation tools.
--             
-- Reference:  
--     Command Line Tools User Guide, Chapter 23
--     Synthesis and Simulation Design Guide, Chapter 6
--             
--------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library SIMPRIM;
use SIMPRIM.VCOMPONENTS.ALL;
use SIMPRIM.VPACKAGE.ALL;

entity mux is
  port (
    a27 : in STD_LOGIC := 'X'; 
    a26 : in STD_LOGIC := 'X'; 
    a21 : in STD_LOGIC := 'X'; 
    CLK : in STD_LOGIC := 'X'; 
    SIZ0 : in STD_LOGIC := 'X'; 
    a0 : in STD_LOGIC := 'X'; 
    SIZ1 : in STD_LOGIC := 'X'; 
    a19 : in STD_LOGIC := 'X'; 
    a20 : in STD_LOGIC := 'X'; 
    FC2 : in STD_LOGIC := 'X'; 
    FC1 : in STD_LOGIC := 'X'; 
    FC0 : in STD_LOGIC := 'X'; 
    AS : in STD_LOGIC := 'X'; 
    R_BTN : in STD_LOGIC := 'X'; 
    ECS : in STD_LOGIC := 'X'; 
    a1 : in STD_LOGIC := 'X'; 
    RW_N : in STD_LOGIC := 'X'; 
    DSACK0 : out STD_LOGIC; 
    BYTE0 : out STD_LOGIC; 
    BYTE1 : out STD_LOGIC; 
    BYTE2 : out STD_LOGIC; 
    BYTE3 : out STD_LOGIC; 
    DRAM_CS : out STD_LOGIC; 
    DSACK1 : out STD_LOGIC; 
    ETHRNT : out STD_LOGIC; 
    FLOPPY : out STD_LOGIC; 
    IDE : out STD_LOGIC; 
    IO_CS : out STD_LOGIC; 
    KBD_CS : out STD_LOGIC; 
    ROM : out STD_LOGIC; 
    RW0 : out STD_LOGIC; 
    RW1 : out STD_LOGIC; 
    VGA_CS : out STD_LOGIC; 
    nRW0 : out STD_LOGIC; 
    nRW1 : out STD_LOGIC; 
    RESET : out STD_LOGIC; 
    HALT : out STD_LOGIC; 
    READ_n : out STD_LOGIC; 
    I_DIR : out STD_LOGIC; 
    I_ALE : out STD_LOGIC; 
    CDIS : out STD_LOGIC; 
    STERM : out STD_LOGIC; 
    I_MEMW : out STD_LOGIC; 
    I_MEMR : out STD_LOGIC; 
    I_IOW : out STD_LOGIC; 
    I_IOR : out STD_LOGIC; 
    FPU : out STD_LOGIC 
  );
end mux;

architecture Structure of mux is
  signal a27_IBUF_1 : STD_LOGIC; 
  signal a26_IBUF_3 : STD_LOGIC; 
  signal a21_IBUF_5 : STD_LOGIC; 
  signal FCLKIO_0_7 : STD_LOGIC; 
  signal SIZ0_IBUF_9 : STD_LOGIC; 
  signal a0_IBUF_11 : STD_LOGIC; 
  signal SIZ1_IBUF_13 : STD_LOGIC; 
  signal a19_IBUF_15 : STD_LOGIC; 
  signal a20_IBUF_17 : STD_LOGIC; 
  signal FC2_IBUF_19 : STD_LOGIC; 
  signal FC1_IBUF_21 : STD_LOGIC; 
  signal FC0_IBUF_23 : STD_LOGIC; 
  signal AS_IBUF_25 : STD_LOGIC; 
  signal HALT_OBUF_27 : STD_LOGIC; 
  signal ECS_IBUF_29 : STD_LOGIC; 
  signal a1_IBUF_31 : STD_LOGIC; 
  signal RW_N_IBUF_33 : STD_LOGIC; 
  signal DSACK0_OBUF_Q_35 : STD_LOGIC; 
  signal BYTE0_OBUF_37 : STD_LOGIC; 
  signal BYTE1_OBUF_39 : STD_LOGIC; 
  signal BYTE2_OBUF_41 : STD_LOGIC; 
  signal BYTE3_OBUF_43 : STD_LOGIC; 
  signal DRAM_CS_OBUF_45 : STD_LOGIC; 
  signal DSACK1_OBUF_47 : STD_LOGIC; 
  signal ETHRNT_OBUF_49 : STD_LOGIC; 
  signal FLOPPY_OBUF_51 : STD_LOGIC; 
  signal IDE_OBUF_53 : STD_LOGIC; 
  signal IO_CS_OBUF_55 : STD_LOGIC; 
  signal KBD_CS_OBUF_57 : STD_LOGIC; 
  signal ROM_OBUF_59 : STD_LOGIC; 
  signal RW0_OBUF_61 : STD_LOGIC; 
  signal RW1_OBUF_63 : STD_LOGIC; 
  signal VGA_CS_OBUF_65 : STD_LOGIC; 
  signal nRW0_OBUF_67 : STD_LOGIC; 
  signal nRW1_OBUF_69 : STD_LOGIC; 
  signal HALT_OBUF_BUF0_71 : STD_LOGIC; 
  signal HALT_OBUF_BUF1_73 : STD_LOGIC; 
  signal READ_n_OBUF_75 : STD_LOGIC; 
  signal I_ALE_OBUF_Q_77 : STD_LOGIC; 
  signal I_ALE_OBUF_BUF0_79 : STD_LOGIC; 
  signal I_ALE_OBUF_BUF1_81 : STD_LOGIC; 
  signal I_MEMW_OBUF_Q_83 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF0_85 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF1_87 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF2_89 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF3_91 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF4_93 : STD_LOGIC; 
  signal DSACK0_OBUF_Q_94 : STD_LOGIC; 
  signal DSACK0_OBUF_D_95 : STD_LOGIC; 
  signal DSACK0_OBUF_D1_96 : STD_LOGIC; 
  signal DSACK0_OBUF_D2_97 : STD_LOGIC; 
  signal DSACK0_OBUF_D2_PT_0_101 : STD_LOGIC; 
  signal DSACK0_OBUF_D2_PT_1_103 : STD_LOGIC; 
  signal cntr_0_Q_104 : STD_LOGIC; 
  signal cntr_0_D_105 : STD_LOGIC; 
  signal cntr_0_tsimcreated_xor_Q_106 : STD_LOGIC; 
  signal cntr_0_RSTF_107 : STD_LOGIC; 
  signal Gnd_108 : STD_LOGIC; 
  signal cntr_0_tsimcreated_prld_Q_109 : STD_LOGIC; 
  signal Vcc_110 : STD_LOGIC; 
  signal cntr_0_D1_111 : STD_LOGIC; 
  signal cntr_0_D2_112 : STD_LOGIC; 
  signal cntr_2_cntr_2_RSTF_INT_UIM_113 : STD_LOGIC; 
  signal cntr_1_Q_114 : STD_LOGIC; 
  signal cntr_1_D_115 : STD_LOGIC; 
  signal cntr_1_tsimcreated_xor_Q_116 : STD_LOGIC; 
  signal cntr_1_RSTF_117 : STD_LOGIC; 
  signal cntr_1_tsimcreated_prld_Q_118 : STD_LOGIC; 
  signal cntr_1_D1_119 : STD_LOGIC; 
  signal cntr_1_D2_120 : STD_LOGIC; 
  signal cntr_2_Q_121 : STD_LOGIC; 
  signal cntr_2_D_122 : STD_LOGIC; 
  signal cntr_2_tsimcreated_xor_Q_123 : STD_LOGIC; 
  signal cntr_2_RSTF_124 : STD_LOGIC; 
  signal cntr_2_tsimcreated_prld_Q_125 : STD_LOGIC; 
  signal cntr_2_D1_126 : STD_LOGIC; 
  signal cntr_2_D2_127 : STD_LOGIC; 
  signal cntr_3_Q_128 : STD_LOGIC; 
  signal cntr_3_D_129 : STD_LOGIC; 
  signal cntr_3_tsimcreated_xor_Q_130 : STD_LOGIC; 
  signal cntr_3_RSTF_131 : STD_LOGIC; 
  signal cntr_3_tsimcreated_prld_Q_132 : STD_LOGIC; 
  signal cntr_3_D1_133 : STD_LOGIC; 
  signal cntr_3_D2_134 : STD_LOGIC; 
  signal BYTE0_OBUF_Q_135 : STD_LOGIC; 
  signal BYTE0_OBUF_D_136 : STD_LOGIC; 
  signal BYTE0_OBUF_D1_137 : STD_LOGIC; 
  signal BYTE0_OBUF_D2_138 : STD_LOGIC; 
  signal BYTE0_OBUF_D2_PT_0_139 : STD_LOGIC; 
  signal BYTE0_OBUF_D2_PT_1_140 : STD_LOGIC; 
  signal BYTE0_OBUF_D2_PT_2_141 : STD_LOGIC; 
  signal BYTE0_OBUF_D2_PT_3_142 : STD_LOGIC; 
  signal BYTE1_OBUF_Q_143 : STD_LOGIC; 
  signal BYTE1_OBUF_D_144 : STD_LOGIC; 
  signal BYTE1_OBUF_D1_145 : STD_LOGIC; 
  signal BYTE1_OBUF_D2_146 : STD_LOGIC; 
  signal BYTE1_OBUF_D2_PT_0_147 : STD_LOGIC; 
  signal BYTE1_OBUF_D2_PT_1_148 : STD_LOGIC; 
  signal BYTE1_OBUF_D2_PT_2_149 : STD_LOGIC; 
  signal BYTE2_OBUF_Q_150 : STD_LOGIC; 
  signal BYTE2_OBUF_D_151 : STD_LOGIC; 
  signal BYTE2_OBUF_D1_152 : STD_LOGIC; 
  signal BYTE2_OBUF_D2_153 : STD_LOGIC; 
  signal BYTE2_OBUF_D2_PT_0_154 : STD_LOGIC; 
  signal BYTE2_OBUF_D2_PT_1_155 : STD_LOGIC; 
  signal BYTE3_OBUF_Q_156 : STD_LOGIC; 
  signal BYTE3_OBUF_D_157 : STD_LOGIC; 
  signal BYTE3_OBUF_D1_158 : STD_LOGIC; 
  signal BYTE3_OBUF_D2_159 : STD_LOGIC; 
  signal DRAM_CS_OBUF_Q_160 : STD_LOGIC; 
  signal DRAM_CS_OBUF_D_161 : STD_LOGIC; 
  signal DRAM_CS_OBUF_D1_162 : STD_LOGIC; 
  signal DRAM_CS_OBUF_D2_163 : STD_LOGIC; 
  signal DRAM_CS_OBUF_D2_PT_0_164 : STD_LOGIC; 
  signal DRAM_CS_OBUF_D2_PT_1_165 : STD_LOGIC; 
  signal DRAM_CS_OBUF_D2_PT_2_166 : STD_LOGIC; 
  signal DSACK1_OBUF_Q_167 : STD_LOGIC; 
  signal DSACK1_OBUF_D_168 : STD_LOGIC; 
  signal DSACK1_OBUF_D1_169 : STD_LOGIC; 
  signal DSACK1_OBUF_D2_170 : STD_LOGIC; 
  signal DSACK1_OBUF_D2_PT_0_171 : STD_LOGIC; 
  signal DSACK1_OBUF_D2_PT_1_172 : STD_LOGIC; 
  signal DSACK1_OBUF_D2_PT_2_173 : STD_LOGIC; 
  signal ETHRNT_OBUF_Q_174 : STD_LOGIC; 
  signal ETHRNT_OBUF_D_175 : STD_LOGIC; 
  signal ETHRNT_OBUF_D1_176 : STD_LOGIC; 
  signal ETHRNT_OBUF_D2_177 : STD_LOGIC; 
  signal ETHRNT_OBUF_D2_PT_0_178 : STD_LOGIC; 
  signal ETHRNT_OBUF_D2_PT_1_179 : STD_LOGIC; 
  signal ETHRNT_OBUF_D2_PT_2_180 : STD_LOGIC; 
  signal FLOPPY_OBUF_Q_181 : STD_LOGIC; 
  signal FLOPPY_OBUF_D_182 : STD_LOGIC; 
  signal FLOPPY_OBUF_D1_183 : STD_LOGIC; 
  signal FLOPPY_OBUF_D2_184 : STD_LOGIC; 
  signal FLOPPY_OBUF_D2_PT_0_185 : STD_LOGIC; 
  signal FLOPPY_OBUF_D2_PT_1_186 : STD_LOGIC; 
  signal FLOPPY_OBUF_D2_PT_2_187 : STD_LOGIC; 
  signal IDE_OBUF_Q_188 : STD_LOGIC; 
  signal IDE_OBUF_D_189 : STD_LOGIC; 
  signal IDE_OBUF_D1_190 : STD_LOGIC; 
  signal IDE_OBUF_D2_191 : STD_LOGIC; 
  signal IDE_OBUF_D2_PT_0_192 : STD_LOGIC; 
  signal IDE_OBUF_D2_PT_1_193 : STD_LOGIC; 
  signal IDE_OBUF_D2_PT_2_194 : STD_LOGIC; 
  signal IO_CS_OBUF_Q_195 : STD_LOGIC; 
  signal IO_CS_OBUF_D_196 : STD_LOGIC; 
  signal IO_CS_OBUF_D1_197 : STD_LOGIC; 
  signal IO_CS_OBUF_D2_198 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF1_EXP_199 : STD_LOGIC; 
  signal IO_CS_OBUF_D2_PT_0_200 : STD_LOGIC; 
  signal IO_CS_OBUF_D2_PT_1_201 : STD_LOGIC; 
  signal IO_CS_OBUF_D2_PT_2_202 : STD_LOGIC; 
  signal IO_CS_OBUF_D2_PT_3_203 : STD_LOGIC; 
  signal IO_CS_OBUF_D2_PT_4_204 : STD_LOGIC; 
  signal IO_CS_OBUF_D2_PT_5_205 : STD_LOGIC; 
  signal KBD_CS_OBUF_Q_206 : STD_LOGIC; 
  signal KBD_CS_OBUF_D_207 : STD_LOGIC; 
  signal KBD_CS_OBUF_D1_208 : STD_LOGIC; 
  signal KBD_CS_OBUF_D2_209 : STD_LOGIC; 
  signal KBD_CS_OBUF_D2_PT_0_210 : STD_LOGIC; 
  signal KBD_CS_OBUF_D2_PT_1_211 : STD_LOGIC; 
  signal KBD_CS_OBUF_D2_PT_2_212 : STD_LOGIC; 
  signal ROM_OBUF_Q_213 : STD_LOGIC; 
  signal ROM_OBUF_D_214 : STD_LOGIC; 
  signal ROM_OBUF_D1_215 : STD_LOGIC; 
  signal ROM_OBUF_D2_216 : STD_LOGIC; 
  signal ROM_OBUF_D2_PT_0_217 : STD_LOGIC; 
  signal ROM_OBUF_D2_PT_1_218 : STD_LOGIC; 
  signal ROM_OBUF_D2_PT_2_219 : STD_LOGIC; 
  signal RW0_OBUF_Q_220 : STD_LOGIC; 
  signal RW0_OBUF_D_221 : STD_LOGIC; 
  signal RW0_OBUF_D1_222 : STD_LOGIC; 
  signal RW0_OBUF_D2_223 : STD_LOGIC; 
  signal RW0_OBUF_D2_PT_0_224 : STD_LOGIC; 
  signal RW0_OBUF_D2_PT_1_225 : STD_LOGIC; 
  signal RW0_OBUF_D2_PT_2_226 : STD_LOGIC; 
  signal RW0_OBUF_D2_PT_3_227 : STD_LOGIC; 
  signal RW0_OBUF_D2_PT_4_228 : STD_LOGIC; 
  signal RW1_OBUF_Q_229 : STD_LOGIC; 
  signal RW1_OBUF_D_230 : STD_LOGIC; 
  signal RW1_OBUF_D1_231 : STD_LOGIC; 
  signal RW1_OBUF_D2_232 : STD_LOGIC; 
  signal RW1_OBUF_D2_PT_0_233 : STD_LOGIC; 
  signal RW1_OBUF_D2_PT_1_234 : STD_LOGIC; 
  signal RW1_OBUF_D2_PT_2_235 : STD_LOGIC; 
  signal RW1_OBUF_D2_PT_3_236 : STD_LOGIC; 
  signal RW1_OBUF_D2_PT_4_237 : STD_LOGIC; 
  signal VGA_CS_OBUF_Q_238 : STD_LOGIC; 
  signal VGA_CS_OBUF_D_239 : STD_LOGIC; 
  signal VGA_CS_OBUF_D1_240 : STD_LOGIC; 
  signal VGA_CS_OBUF_D2_241 : STD_LOGIC; 
  signal VGA_CS_OBUF_D2_PT_0_242 : STD_LOGIC; 
  signal VGA_CS_OBUF_D2_PT_1_243 : STD_LOGIC; 
  signal VGA_CS_OBUF_D2_PT_2_244 : STD_LOGIC; 
  signal nRW0_OBUF_Q_245 : STD_LOGIC; 
  signal nRW0_OBUF_D_246 : STD_LOGIC; 
  signal nRW0_OBUF_D1_247 : STD_LOGIC; 
  signal nRW0_OBUF_D2_248 : STD_LOGIC; 
  signal nRW0_OBUF_D2_PT_0_249 : STD_LOGIC; 
  signal nRW0_OBUF_D2_PT_1_250 : STD_LOGIC; 
  signal nRW0_OBUF_D2_PT_2_251 : STD_LOGIC; 
  signal nRW0_OBUF_D2_PT_3_252 : STD_LOGIC; 
  signal nRW0_OBUF_D2_PT_4_253 : STD_LOGIC; 
  signal nRW1_OBUF_Q_254 : STD_LOGIC; 
  signal nRW1_OBUF_D_255 : STD_LOGIC; 
  signal nRW1_OBUF_D1_256 : STD_LOGIC; 
  signal nRW1_OBUF_D2_257 : STD_LOGIC; 
  signal nRW1_OBUF_D2_PT_0_258 : STD_LOGIC; 
  signal nRW1_OBUF_D2_PT_1_259 : STD_LOGIC; 
  signal nRW1_OBUF_D2_PT_2_260 : STD_LOGIC; 
  signal nRW1_OBUF_D2_PT_3_261 : STD_LOGIC; 
  signal nRW1_OBUF_D2_PT_4_262 : STD_LOGIC; 
  signal HALT_OBUF_BUF0_Q_263 : STD_LOGIC; 
  signal HALT_OBUF_BUF0_D_264 : STD_LOGIC; 
  signal HALT_OBUF_BUF0_D1_265 : STD_LOGIC; 
  signal HALT_OBUF_BUF0_D2_266 : STD_LOGIC; 
  signal HALT_OBUF_BUF1_Q_267 : STD_LOGIC; 
  signal HALT_OBUF_BUF1_D_268 : STD_LOGIC; 
  signal HALT_OBUF_BUF1_D1_269 : STD_LOGIC; 
  signal HALT_OBUF_BUF1_D2_270 : STD_LOGIC; 
  signal READ_n_OBUF_Q_271 : STD_LOGIC; 
  signal READ_n_OBUF_D_272 : STD_LOGIC; 
  signal READ_n_OBUF_D1_273 : STD_LOGIC; 
  signal READ_n_OBUF_D2_274 : STD_LOGIC; 
  signal I_ALE_OBUF_Q_275 : STD_LOGIC; 
  signal I_ALE_OBUF_D_276 : STD_LOGIC; 
  signal I_ALE_OBUF_D1_277 : STD_LOGIC; 
  signal I_ALE_OBUF_D2_278 : STD_LOGIC; 
  signal I_ALE_OBUF_BUF0_Q_279 : STD_LOGIC; 
  signal I_ALE_OBUF_BUF0_D_280 : STD_LOGIC; 
  signal I_ALE_OBUF_BUF0_D1_281 : STD_LOGIC; 
  signal I_ALE_OBUF_BUF0_D2_282 : STD_LOGIC; 
  signal I_ALE_OBUF_BUF1_Q_283 : STD_LOGIC; 
  signal I_ALE_OBUF_BUF1_D_284 : STD_LOGIC; 
  signal I_ALE_OBUF_BUF1_D1_285 : STD_LOGIC; 
  signal I_ALE_OBUF_BUF1_D2_286 : STD_LOGIC; 
  signal I_MEMW_OBUF_Q_287 : STD_LOGIC; 
  signal I_MEMW_OBUF_D_288 : STD_LOGIC; 
  signal I_MEMW_OBUF_D1_289 : STD_LOGIC; 
  signal I_MEMW_OBUF_D2_290 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF0_Q_291 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF0_D_292 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF0_D1_293 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF0_D2_294 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF1_Q_295 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_Q_296 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF1_D_297 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF1_D1_298 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF1_D2_299 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF2_Q_300 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF2_D_301 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF2_D1_302 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF2_D2_303 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF3_Q_304 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF3_D_305 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF3_D1_306 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF3_D2_307 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF4_Q_308 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF4_D_309 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF4_D1_310 : STD_LOGIC; 
  signal I_MEMW_OBUF_BUF4_D2_311 : STD_LOGIC; 
  signal cntr_2_cntr_2_RSTF_INT_Q_312 : STD_LOGIC; 
  signal cntr_2_cntr_2_RSTF_INT_D_313 : STD_LOGIC; 
  signal cntr_2_cntr_2_RSTF_INT_D1_314 : STD_LOGIC; 
  signal cntr_2_cntr_2_RSTF_INT_D2_315 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK0_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK0_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK0_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK0_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK0_OBUF_D2_PT_0_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK0_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK0_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK0_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK0_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK0_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_0_tsimcreated_xor_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_0_tsimcreated_xor_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_0_tsimcreated_prld_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_0_tsimcreated_prld_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_0_REG_IN : STD_LOGIC; 
  signal NlwBufferSignal_cntr_0_REG_CLK : STD_LOGIC; 
  signal NlwBufferSignal_cntr_0_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_0_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_0_RSTF_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_0_RSTF_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_1_tsimcreated_xor_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_1_tsimcreated_xor_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_1_tsimcreated_prld_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_1_tsimcreated_prld_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_1_REG_IN : STD_LOGIC; 
  signal NlwBufferSignal_cntr_1_REG_CLK : STD_LOGIC; 
  signal NlwBufferSignal_cntr_1_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_1_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_1_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_1_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_1_RSTF_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_1_RSTF_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_2_tsimcreated_xor_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_2_tsimcreated_xor_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_2_tsimcreated_prld_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_2_tsimcreated_prld_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_2_REG_IN : STD_LOGIC; 
  signal NlwBufferSignal_cntr_2_REG_CLK : STD_LOGIC; 
  signal NlwBufferSignal_cntr_2_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_2_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_2_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_2_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_2_RSTF_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_2_RSTF_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_3_tsimcreated_xor_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_3_tsimcreated_xor_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_3_tsimcreated_prld_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_3_tsimcreated_prld_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_3_REG_IN : STD_LOGIC; 
  signal NlwBufferSignal_cntr_3_REG_CLK : STD_LOGIC; 
  signal NlwBufferSignal_cntr_3_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_3_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_3_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_3_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_3_D2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_3_RSTF_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_3_RSTF_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE0_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE0_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE0_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE0_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE0_OBUF_D2_PT_0_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE0_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE0_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE0_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE0_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE0_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE0_OBUF_D2_PT_2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE0_OBUF_D2_PT_3_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE0_OBUF_D2_PT_3_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE0_OBUF_D2_PT_3_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE0_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE0_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE0_OBUF_D2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE0_OBUF_D2_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE1_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE1_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE1_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE1_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE1_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE1_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE1_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE1_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE1_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE1_OBUF_D2_PT_2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE1_OBUF_D2_PT_2_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE1_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE1_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE1_OBUF_D2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE2_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE2_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE2_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE2_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE2_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE2_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE2_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE2_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE2_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE3_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE3_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE3_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_BYTE3_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_DRAM_CS_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_DRAM_CS_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_DRAM_CS_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_DRAM_CS_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_DRAM_CS_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_DRAM_CS_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_DRAM_CS_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_DRAM_CS_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_DRAM_CS_OBUF_D2_PT_2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_DRAM_CS_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_DRAM_CS_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_DRAM_CS_OBUF_D2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK1_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK1_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK1_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK1_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK1_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK1_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK1_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK1_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK1_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK1_OBUF_D2_PT_2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK1_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK1_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_DSACK1_OBUF_D2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_ETHRNT_OBUF_D2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_FLOPPY_OBUF_D2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_0_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_0_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_0_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_0_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_0_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_1_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_1_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_1_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_1_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_2_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_2_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_2_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_PT_2_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_IDE_OBUF_D2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_IO_CS_OBUF_D2_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_KBD_CS_OBUF_D2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_0_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_0_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_0_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_0_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_0_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_1_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_1_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_1_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_1_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_2_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_2_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_2_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_PT_2_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_ROM_OBUF_D2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_RW0_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_RW0_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_RW0_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_RW0_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_RW0_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_RW0_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_RW0_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_RW0_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_RW0_OBUF_D2_PT_3_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_RW0_OBUF_D2_PT_3_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_RW0_OBUF_D2_PT_4_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_RW0_OBUF_D2_PT_4_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_RW0_OBUF_D2_PT_4_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_RW0_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_RW0_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_RW0_OBUF_D2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_RW0_OBUF_D2_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_RW0_OBUF_D2_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_RW1_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_RW1_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_RW1_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_RW1_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_RW1_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_RW1_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_RW1_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_RW1_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_RW1_OBUF_D2_PT_3_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_RW1_OBUF_D2_PT_3_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_RW1_OBUF_D2_PT_4_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_RW1_OBUF_D2_PT_4_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_RW1_OBUF_D2_PT_4_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_RW1_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_RW1_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_RW1_OBUF_D2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_RW1_OBUF_D2_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_RW1_OBUF_D2_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_VGA_CS_OBUF_D2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_nRW0_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_nRW0_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_nRW0_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_nRW0_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_nRW0_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_nRW0_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_nRW0_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_nRW0_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_nRW0_OBUF_D2_PT_3_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_nRW0_OBUF_D2_PT_3_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_nRW0_OBUF_D2_PT_4_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_nRW0_OBUF_D2_PT_4_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_nRW0_OBUF_D2_PT_4_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_nRW0_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_nRW0_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_nRW0_OBUF_D2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_nRW0_OBUF_D2_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_nRW0_OBUF_D2_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_nRW1_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_nRW1_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_nRW1_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_nRW1_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_nRW1_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_nRW1_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_nRW1_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_nRW1_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_nRW1_OBUF_D2_PT_3_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_nRW1_OBUF_D2_PT_3_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_nRW1_OBUF_D2_PT_4_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_nRW1_OBUF_D2_PT_4_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_nRW1_OBUF_D2_PT_4_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_nRW1_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_nRW1_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_nRW1_OBUF_D2_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_nRW1_OBUF_D2_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_nRW1_OBUF_D2_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_HALT_OBUF_BUF0_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_HALT_OBUF_BUF0_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_HALT_OBUF_BUF0_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_HALT_OBUF_BUF0_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_HALT_OBUF_BUF1_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_HALT_OBUF_BUF1_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_HALT_OBUF_BUF1_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_HALT_OBUF_BUF1_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_READ_n_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_READ_n_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_READ_n_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_READ_n_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_I_ALE_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_I_ALE_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_I_ALE_OBUF_BUF0_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_I_ALE_OBUF_BUF0_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_I_ALE_OBUF_BUF1_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_I_ALE_OBUF_BUF1_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_I_MEMW_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_I_MEMW_OBUF_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_I_MEMW_OBUF_BUF0_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_I_MEMW_OBUF_BUF0_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_I_MEMW_OBUF_BUF1_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_I_MEMW_OBUF_BUF1_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN2 : STD_LOGIC; 
  signal NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN3 : STD_LOGIC; 
  signal NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN4 : STD_LOGIC; 
  signal NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN5 : STD_LOGIC; 
  signal NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN6 : STD_LOGIC; 
  signal NlwBufferSignal_I_MEMW_OBUF_BUF2_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_I_MEMW_OBUF_BUF2_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_I_MEMW_OBUF_BUF3_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_I_MEMW_OBUF_BUF3_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_I_MEMW_OBUF_BUF4_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_I_MEMW_OBUF_BUF4_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_2_cntr_2_RSTF_INT_D_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_2_cntr_2_RSTF_INT_D_IN1 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_2_cntr_2_RSTF_INT_D2_IN0 : STD_LOGIC; 
  signal NlwBufferSignal_cntr_2_cntr_2_RSTF_INT_D2_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_DSACK0_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_DSACK0_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_DSACK0_OBUF_D2_PT_0_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_DSACK0_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_DSACK0_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_DSACK0_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_cntr_0_RSTF_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_cntr_0_RSTF_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_cntr_1_RSTF_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_cntr_1_RSTF_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_cntr_2_RSTF_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_cntr_2_RSTF_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_cntr_3_RSTF_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_cntr_3_RSTF_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_BYTE0_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_BYTE0_OBUF_D2_PT_0_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_BYTE0_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_BYTE0_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_BYTE0_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_BYTE0_OBUF_D2_PT_2_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_BYTE0_OBUF_D2_PT_3_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_BYTE0_OBUF_D2_PT_3_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_BYTE1_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_BYTE1_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_BYTE1_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_BYTE1_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_BYTE1_OBUF_D2_PT_2_IN3 : STD_LOGIC; 
  signal NlwInverterSignal_BYTE2_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_BYTE2_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_BYTE3_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_BYTE3_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_BYTE3_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_DRAM_CS_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_DRAM_CS_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_DSACK1_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_DSACK1_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_DSACK1_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_DSACK1_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_DSACK1_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_DSACK1_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_DSACK1_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_DSACK1_OBUF_D2_PT_2_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_ETHRNT_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_ETHRNT_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_ETHRNT_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_ETHRNT_OBUF_D2_PT_0_IN4 : STD_LOGIC; 
  signal NlwInverterSignal_ETHRNT_OBUF_D2_PT_0_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_ETHRNT_OBUF_D2_PT_0_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_ETHRNT_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_ETHRNT_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_ETHRNT_OBUF_D2_PT_1_IN4 : STD_LOGIC; 
  signal NlwInverterSignal_ETHRNT_OBUF_D2_PT_1_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_ETHRNT_OBUF_D2_PT_1_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_ETHRNT_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_ETHRNT_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_ETHRNT_OBUF_D2_PT_2_IN4 : STD_LOGIC; 
  signal NlwInverterSignal_ETHRNT_OBUF_D2_PT_2_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_ETHRNT_OBUF_D2_PT_2_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_FLOPPY_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_FLOPPY_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_FLOPPY_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_FLOPPY_OBUF_D2_PT_0_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_FLOPPY_OBUF_D2_PT_0_IN4 : STD_LOGIC; 
  signal NlwInverterSignal_FLOPPY_OBUF_D2_PT_0_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_FLOPPY_OBUF_D2_PT_0_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_FLOPPY_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_FLOPPY_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_FLOPPY_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_FLOPPY_OBUF_D2_PT_1_IN4 : STD_LOGIC; 
  signal NlwInverterSignal_FLOPPY_OBUF_D2_PT_1_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_FLOPPY_OBUF_D2_PT_1_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_FLOPPY_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_FLOPPY_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_FLOPPY_OBUF_D2_PT_2_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_FLOPPY_OBUF_D2_PT_2_IN4 : STD_LOGIC; 
  signal NlwInverterSignal_FLOPPY_OBUF_D2_PT_2_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_FLOPPY_OBUF_D2_PT_2_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_IDE_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_IDE_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_IDE_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_IDE_OBUF_D2_PT_0_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_IDE_OBUF_D2_PT_0_IN3 : STD_LOGIC; 
  signal NlwInverterSignal_IDE_OBUF_D2_PT_0_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_IDE_OBUF_D2_PT_0_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_IDE_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_IDE_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_IDE_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_IDE_OBUF_D2_PT_1_IN3 : STD_LOGIC; 
  signal NlwInverterSignal_IDE_OBUF_D2_PT_1_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_IDE_OBUF_D2_PT_1_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_IDE_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_IDE_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_IDE_OBUF_D2_PT_2_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_IDE_OBUF_D2_PT_2_IN3 : STD_LOGIC; 
  signal NlwInverterSignal_IDE_OBUF_D2_PT_2_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_IDE_OBUF_D2_PT_2_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_1_IN4 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_1_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_1_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_2_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_2_IN4 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_2_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_2_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_3_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_3_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_3_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_3_IN3 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_3_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_3_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_4_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_4_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_4_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_4_IN3 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_4_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_4_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_5_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_5_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_5_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_5_IN3 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_5_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_IO_CS_OBUF_D2_PT_5_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_KBD_CS_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_KBD_CS_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_KBD_CS_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_KBD_CS_OBUF_D2_PT_0_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_KBD_CS_OBUF_D2_PT_0_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_KBD_CS_OBUF_D2_PT_0_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_KBD_CS_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_KBD_CS_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_KBD_CS_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_KBD_CS_OBUF_D2_PT_1_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_KBD_CS_OBUF_D2_PT_1_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_KBD_CS_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_KBD_CS_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_KBD_CS_OBUF_D2_PT_2_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_KBD_CS_OBUF_D2_PT_2_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_KBD_CS_OBUF_D2_PT_2_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_0_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_0_IN3 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_0_IN4 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_0_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_0_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_1_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_1_IN3 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_1_IN4 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_1_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_1_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_2_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_2_IN3 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_2_IN4 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_2_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_ROM_OBUF_D2_PT_2_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_RW0_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_RW0_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_RW1_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_RW1_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_RW1_OBUF_D2_PT_3_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_VGA_CS_OBUF_D_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_VGA_CS_OBUF_D2_PT_0_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_VGA_CS_OBUF_D2_PT_0_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_VGA_CS_OBUF_D2_PT_0_IN3 : STD_LOGIC; 
  signal NlwInverterSignal_VGA_CS_OBUF_D2_PT_0_IN4 : STD_LOGIC; 
  signal NlwInverterSignal_VGA_CS_OBUF_D2_PT_0_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_VGA_CS_OBUF_D2_PT_0_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_VGA_CS_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_VGA_CS_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_VGA_CS_OBUF_D2_PT_1_IN3 : STD_LOGIC; 
  signal NlwInverterSignal_VGA_CS_OBUF_D2_PT_1_IN4 : STD_LOGIC; 
  signal NlwInverterSignal_VGA_CS_OBUF_D2_PT_1_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_VGA_CS_OBUF_D2_PT_1_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_VGA_CS_OBUF_D2_PT_2_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_VGA_CS_OBUF_D2_PT_2_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_VGA_CS_OBUF_D2_PT_2_IN3 : STD_LOGIC; 
  signal NlwInverterSignal_VGA_CS_OBUF_D2_PT_2_IN4 : STD_LOGIC; 
  signal NlwInverterSignal_VGA_CS_OBUF_D2_PT_2_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_VGA_CS_OBUF_D2_PT_2_IN6 : STD_LOGIC; 
  signal NlwInverterSignal_nRW0_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_nRW0_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_nRW0_OBUF_D2_PT_3_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_nRW1_OBUF_D2_PT_1_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_nRW1_OBUF_D2_PT_1_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_nRW1_OBUF_D2_PT_3_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_nRW1_OBUF_D2_PT_3_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_READ_n_OBUF_D2_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_READ_n_OBUF_D2_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN0 : STD_LOGIC; 
  signal NlwInverterSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN1 : STD_LOGIC; 
  signal NlwInverterSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN2 : STD_LOGIC; 
  signal NlwInverterSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN4 : STD_LOGIC; 
  signal NlwInverterSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN5 : STD_LOGIC; 
  signal NlwInverterSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN6 : STD_LOGIC; 
  signal cntr : STD_LOGIC_VECTOR ( 3 downto 0 ); 
begin
  a27_IBUF : X_BUF
    port map (
      I => a27,
      O => a27_IBUF_1
    );
  a26_IBUF : X_BUF
    port map (
      I => a26,
      O => a26_IBUF_3
    );
  a21_IBUF : X_BUF
    port map (
      I => a21,
      O => a21_IBUF_5
    );
  FCLKIO_0 : X_BUF
    port map (
      I => CLK,
      O => FCLKIO_0_7
    );
  SIZ0_IBUF : X_BUF
    port map (
      I => SIZ0,
      O => SIZ0_IBUF_9
    );
  a0_IBUF : X_BUF
    port map (
      I => a0,
      O => a0_IBUF_11
    );
  SIZ1_IBUF : X_BUF
    port map (
      I => SIZ1,
      O => SIZ1_IBUF_13
    );
  a19_IBUF : X_BUF
    port map (
      I => a19,
      O => a19_IBUF_15
    );
  a20_IBUF : X_BUF
    port map (
      I => a20,
      O => a20_IBUF_17
    );
  FC2_IBUF : X_BUF
    port map (
      I => FC2,
      O => FC2_IBUF_19
    );
  FC1_IBUF : X_BUF
    port map (
      I => FC1,
      O => FC1_IBUF_21
    );
  FC0_IBUF : X_BUF
    port map (
      I => FC0,
      O => FC0_IBUF_23
    );
  AS_IBUF : X_BUF
    port map (
      I => AS,
      O => AS_IBUF_25
    );
  HALT_OBUF : X_BUF
    port map (
      I => R_BTN,
      O => HALT_OBUF_27
    );
  ECS_IBUF : X_BUF
    port map (
      I => ECS,
      O => ECS_IBUF_29
    );
  a1_IBUF : X_BUF
    port map (
      I => a1,
      O => a1_IBUF_31
    );
  RW_N_IBUF : X_BUF
    port map (
      I => RW_N,
      O => RW_N_IBUF_33
    );
  DSACK0_36 : X_BUF
    port map (
      I => DSACK0_OBUF_Q_35,
      O => DSACK0
    );
  BYTE0_38 : X_BUF
    port map (
      I => BYTE0_OBUF_37,
      O => BYTE0
    );
  BYTE1_40 : X_BUF
    port map (
      I => BYTE1_OBUF_39,
      O => BYTE1
    );
  BYTE2_42 : X_BUF
    port map (
      I => BYTE2_OBUF_41,
      O => BYTE2
    );
  BYTE3_44 : X_BUF
    port map (
      I => BYTE3_OBUF_43,
      O => BYTE3
    );
  DRAM_CS_46 : X_BUF
    port map (
      I => DRAM_CS_OBUF_45,
      O => DRAM_CS
    );
  DSACK1_48 : X_BUF
    port map (
      I => DSACK1_OBUF_47,
      O => DSACK1
    );
  ETHRNT_50 : X_BUF
    port map (
      I => ETHRNT_OBUF_49,
      O => ETHRNT
    );
  FLOPPY_52 : X_BUF
    port map (
      I => FLOPPY_OBUF_51,
      O => FLOPPY
    );
  IDE_54 : X_BUF
    port map (
      I => IDE_OBUF_53,
      O => IDE
    );
  IO_CS_56 : X_BUF
    port map (
      I => IO_CS_OBUF_55,
      O => IO_CS
    );
  KBD_CS_58 : X_BUF
    port map (
      I => KBD_CS_OBUF_57,
      O => KBD_CS
    );
  ROM_60 : X_BUF
    port map (
      I => ROM_OBUF_59,
      O => ROM
    );
  RW0_62 : X_BUF
    port map (
      I => RW0_OBUF_61,
      O => RW0
    );
  RW1_64 : X_BUF
    port map (
      I => RW1_OBUF_63,
      O => RW1
    );
  VGA_CS_66 : X_BUF
    port map (
      I => VGA_CS_OBUF_65,
      O => VGA_CS
    );
  nRW0_68 : X_BUF
    port map (
      I => nRW0_OBUF_67,
      O => nRW0
    );
  nRW1_70 : X_BUF
    port map (
      I => nRW1_OBUF_69,
      O => nRW1
    );
  RESET_72 : X_BUF
    port map (
      I => HALT_OBUF_BUF0_71,
      O => RESET
    );
  HALT_74 : X_BUF
    port map (
      I => HALT_OBUF_BUF1_73,
      O => HALT
    );
  READ_n_76 : X_BUF
    port map (
      I => READ_n_OBUF_75,
      O => READ_n
    );
  I_DIR_78 : X_BUF
    port map (
      I => I_ALE_OBUF_Q_77,
      O => I_DIR
    );
  I_ALE_80 : X_BUF
    port map (
      I => I_ALE_OBUF_BUF0_79,
      O => I_ALE
    );
  CDIS_82 : X_BUF
    port map (
      I => I_ALE_OBUF_BUF1_81,
      O => CDIS
    );
  STERM_84 : X_BUF
    port map (
      I => I_MEMW_OBUF_Q_83,
      O => STERM
    );
  I_MEMW_86 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF0_85,
      O => I_MEMW
    );
  I_MEMR_88 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF1_87,
      O => I_MEMR
    );
  I_IOW_90 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF2_89,
      O => I_IOW
    );
  I_IOR_92 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF3_91,
      O => I_IOR
    );
  FPU_94 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF4_93,
      O => FPU
    );
  DSACK0_OBUF_Q : X_BUF
    port map (
      I => DSACK0_OBUF_Q_94,
      O => DSACK0_OBUF_Q_35
    );
  DSACK0_OBUF_Q_96 : X_BUF
    port map (
      I => DSACK0_OBUF_D_95,
      O => DSACK0_OBUF_Q_94
    );
  DSACK0_OBUF_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_DSACK0_OBUF_D_IN0,
      I1 => NlwBufferSignal_DSACK0_OBUF_D_IN1,
      O => DSACK0_OBUF_D_95
    );
  DSACK0_OBUF_D1 : X_ZERO
    port map (
      O => DSACK0_OBUF_D1_96
    );
  DSACK0_OBUF_D2_PT_0 : X_AND3
    port map (
      I0 => NlwInverterSignal_DSACK0_OBUF_D2_PT_0_IN0,
      I1 => NlwInverterSignal_DSACK0_OBUF_D2_PT_0_IN1,
      I2 => NlwInverterSignal_DSACK0_OBUF_D2_PT_0_IN2,
      O => DSACK0_OBUF_D2_PT_0_101
    );
  DSACK0_OBUF_D2_PT_1 : X_AND3
    port map (
      I0 => NlwInverterSignal_DSACK0_OBUF_D2_PT_1_IN0,
      I1 => NlwInverterSignal_DSACK0_OBUF_D2_PT_1_IN1,
      I2 => NlwInverterSignal_DSACK0_OBUF_D2_PT_1_IN2,
      O => DSACK0_OBUF_D2_PT_1_103
    );
  DSACK0_OBUF_D2 : X_OR2
    port map (
      I0 => NlwBufferSignal_DSACK0_OBUF_D2_IN0,
      I1 => NlwBufferSignal_DSACK0_OBUF_D2_IN1,
      O => DSACK0_OBUF_D2_97
    );
  cntr_0_Q : X_BUF
    port map (
      I => cntr_0_Q_104,
      O => cntr(0)
    );
  cntr_0_tsimcreated_xor_Q : X_XOR2
    port map (
      I0 => NlwBufferSignal_cntr_0_tsimcreated_xor_IN0,
      I1 => NlwBufferSignal_cntr_0_tsimcreated_xor_IN1,
      O => cntr_0_tsimcreated_xor_Q_106
    );
  cntr_0_tsimcreated_prld_Q : X_OR2
    port map (
      I0 => NlwBufferSignal_cntr_0_tsimcreated_prld_IN0,
      I1 => NlwBufferSignal_cntr_0_tsimcreated_prld_IN1,
      O => cntr_0_tsimcreated_prld_Q_109
    );
  Gnd : X_ZERO
    port map (
      O => Gnd_108
    );
  cntr_0_REG : X_FF
    generic map(
      INIT => '0'
    )
    port map (
      I => NlwBufferSignal_cntr_0_REG_IN,
      CE => Vcc_110,
      CLK => NlwBufferSignal_cntr_0_REG_CLK,
      SET => Gnd_108,
      RST => cntr_0_tsimcreated_prld_Q_109,
      O => cntr_0_Q_104
    );
  Vcc : X_ONE
    port map (
      O => Vcc_110
    );
  cntr_0_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_cntr_0_D_IN0,
      I1 => NlwBufferSignal_cntr_0_D_IN1,
      O => cntr_0_D_105
    );
  cntr_0_D1 : X_ZERO
    port map (
      O => cntr_0_D1_111
    );
  cntr_0_D2 : X_ONE
    port map (
      O => cntr_0_D2_112
    );
  cntr_0_RSTF : X_AND2
    port map (
      I0 => NlwInverterSignal_cntr_0_RSTF_IN0,
      I1 => NlwInverterSignal_cntr_0_RSTF_IN1,
      O => cntr_0_RSTF_107
    );
  cntr_1_Q : X_BUF
    port map (
      I => cntr_1_Q_114,
      O => cntr(1)
    );
  cntr_1_tsimcreated_xor_Q : X_XOR2
    port map (
      I0 => NlwBufferSignal_cntr_1_tsimcreated_xor_IN0,
      I1 => NlwBufferSignal_cntr_1_tsimcreated_xor_IN1,
      O => cntr_1_tsimcreated_xor_Q_116
    );
  cntr_1_tsimcreated_prld_Q : X_OR2
    port map (
      I0 => NlwBufferSignal_cntr_1_tsimcreated_prld_IN0,
      I1 => NlwBufferSignal_cntr_1_tsimcreated_prld_IN1,
      O => cntr_1_tsimcreated_prld_Q_118
    );
  cntr_1_REG : X_FF
    generic map(
      INIT => '0'
    )
    port map (
      I => NlwBufferSignal_cntr_1_REG_IN,
      CE => Vcc_110,
      CLK => NlwBufferSignal_cntr_1_REG_CLK,
      SET => Gnd_108,
      RST => cntr_1_tsimcreated_prld_Q_118,
      O => cntr_1_Q_114
    );
  cntr_1_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_cntr_1_D_IN0,
      I1 => NlwBufferSignal_cntr_1_D_IN1,
      O => cntr_1_D_115
    );
  cntr_1_D1 : X_ZERO
    port map (
      O => cntr_1_D1_119
    );
  cntr_1_D2 : X_AND2
    port map (
      I0 => NlwBufferSignal_cntr_1_D2_IN0,
      I1 => NlwBufferSignal_cntr_1_D2_IN1,
      O => cntr_1_D2_120
    );
  cntr_1_RSTF : X_AND2
    port map (
      I0 => NlwInverterSignal_cntr_1_RSTF_IN0,
      I1 => NlwInverterSignal_cntr_1_RSTF_IN1,
      O => cntr_1_RSTF_117
    );
  cntr_2_Q : X_BUF
    port map (
      I => cntr_2_Q_121,
      O => cntr(2)
    );
  cntr_2_tsimcreated_xor_Q : X_XOR2
    port map (
      I0 => NlwBufferSignal_cntr_2_tsimcreated_xor_IN0,
      I1 => NlwBufferSignal_cntr_2_tsimcreated_xor_IN1,
      O => cntr_2_tsimcreated_xor_Q_123
    );
  cntr_2_tsimcreated_prld_Q : X_OR2
    port map (
      I0 => NlwBufferSignal_cntr_2_tsimcreated_prld_IN0,
      I1 => NlwBufferSignal_cntr_2_tsimcreated_prld_IN1,
      O => cntr_2_tsimcreated_prld_Q_125
    );
  cntr_2_REG : X_FF
    generic map(
      INIT => '0'
    )
    port map (
      I => NlwBufferSignal_cntr_2_REG_IN,
      CE => Vcc_110,
      CLK => NlwBufferSignal_cntr_2_REG_CLK,
      SET => Gnd_108,
      RST => cntr_2_tsimcreated_prld_Q_125,
      O => cntr_2_Q_121
    );
  cntr_2_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_cntr_2_D_IN0,
      I1 => NlwBufferSignal_cntr_2_D_IN1,
      O => cntr_2_D_122
    );
  cntr_2_D1 : X_ZERO
    port map (
      O => cntr_2_D1_126
    );
  cntr_2_D2 : X_AND2
    port map (
      I0 => NlwBufferSignal_cntr_2_D2_IN0,
      I1 => NlwBufferSignal_cntr_2_D2_IN1,
      O => cntr_2_D2_127
    );
  cntr_2_RSTF : X_AND2
    port map (
      I0 => NlwInverterSignal_cntr_2_RSTF_IN0,
      I1 => NlwInverterSignal_cntr_2_RSTF_IN1,
      O => cntr_2_RSTF_124
    );
  cntr_3_Q : X_BUF
    port map (
      I => cntr_3_Q_128,
      O => cntr(3)
    );
  cntr_3_tsimcreated_xor_Q : X_XOR2
    port map (
      I0 => NlwBufferSignal_cntr_3_tsimcreated_xor_IN0,
      I1 => NlwBufferSignal_cntr_3_tsimcreated_xor_IN1,
      O => cntr_3_tsimcreated_xor_Q_130
    );
  cntr_3_tsimcreated_prld_Q : X_OR2
    port map (
      I0 => NlwBufferSignal_cntr_3_tsimcreated_prld_IN0,
      I1 => NlwBufferSignal_cntr_3_tsimcreated_prld_IN1,
      O => cntr_3_tsimcreated_prld_Q_132
    );
  cntr_3_REG : X_FF
    generic map(
      INIT => '0'
    )
    port map (
      I => NlwBufferSignal_cntr_3_REG_IN,
      CE => Vcc_110,
      CLK => NlwBufferSignal_cntr_3_REG_CLK,
      SET => Gnd_108,
      RST => cntr_3_tsimcreated_prld_Q_132,
      O => cntr_3_Q_128
    );
  cntr_3_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_cntr_3_D_IN0,
      I1 => NlwBufferSignal_cntr_3_D_IN1,
      O => cntr_3_D_129
    );
  cntr_3_D1 : X_ZERO
    port map (
      O => cntr_3_D1_133
    );
  cntr_3_D2 : X_AND3
    port map (
      I0 => NlwBufferSignal_cntr_3_D2_IN0,
      I1 => NlwBufferSignal_cntr_3_D2_IN1,
      I2 => NlwBufferSignal_cntr_3_D2_IN2,
      O => cntr_3_D2_134
    );
  cntr_3_RSTF : X_AND2
    port map (
      I0 => NlwInverterSignal_cntr_3_RSTF_IN0,
      I1 => NlwInverterSignal_cntr_3_RSTF_IN1,
      O => cntr_3_RSTF_131
    );
  BYTE0_OBUF : X_BUF
    port map (
      I => BYTE0_OBUF_Q_135,
      O => BYTE0_OBUF_37
    );
  BYTE0_OBUF_Q : X_BUF
    port map (
      I => BYTE0_OBUF_D_136,
      O => BYTE0_OBUF_Q_135
    );
  BYTE0_OBUF_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_BYTE0_OBUF_D_IN0,
      I1 => NlwBufferSignal_BYTE0_OBUF_D_IN1,
      O => BYTE0_OBUF_D_136
    );
  BYTE0_OBUF_D1 : X_ZERO
    port map (
      O => BYTE0_OBUF_D1_137
    );
  BYTE0_OBUF_D2_PT_0 : X_AND3
    port map (
      I0 => NlwBufferSignal_BYTE0_OBUF_D2_PT_0_IN0,
      I1 => NlwInverterSignal_BYTE0_OBUF_D2_PT_0_IN1,
      I2 => NlwInverterSignal_BYTE0_OBUF_D2_PT_0_IN2,
      O => BYTE0_OBUF_D2_PT_0_139
    );
  BYTE0_OBUF_D2_PT_1 : X_AND3
    port map (
      I0 => NlwBufferSignal_BYTE0_OBUF_D2_PT_1_IN0,
      I1 => NlwInverterSignal_BYTE0_OBUF_D2_PT_1_IN1,
      I2 => NlwInverterSignal_BYTE0_OBUF_D2_PT_1_IN2,
      O => BYTE0_OBUF_D2_PT_1_140
    );
  BYTE0_OBUF_D2_PT_2 : X_AND3
    port map (
      I0 => NlwBufferSignal_BYTE0_OBUF_D2_PT_2_IN0,
      I1 => NlwInverterSignal_BYTE0_OBUF_D2_PT_2_IN1,
      I2 => NlwInverterSignal_BYTE0_OBUF_D2_PT_2_IN2,
      O => BYTE0_OBUF_D2_PT_2_141
    );
  BYTE0_OBUF_D2_PT_3 : X_AND3
    port map (
      I0 => NlwInverterSignal_BYTE0_OBUF_D2_PT_3_IN0,
      I1 => NlwBufferSignal_BYTE0_OBUF_D2_PT_3_IN1,
      I2 => NlwInverterSignal_BYTE0_OBUF_D2_PT_3_IN2,
      O => BYTE0_OBUF_D2_PT_3_142
    );
  BYTE0_OBUF_D2 : X_OR4
    port map (
      I0 => NlwBufferSignal_BYTE0_OBUF_D2_IN0,
      I1 => NlwBufferSignal_BYTE0_OBUF_D2_IN1,
      I2 => NlwBufferSignal_BYTE0_OBUF_D2_IN2,
      I3 => NlwBufferSignal_BYTE0_OBUF_D2_IN3,
      O => BYTE0_OBUF_D2_138
    );
  BYTE1_OBUF : X_BUF
    port map (
      I => BYTE1_OBUF_Q_143,
      O => BYTE1_OBUF_39
    );
  BYTE1_OBUF_Q : X_BUF
    port map (
      I => BYTE1_OBUF_D_144,
      O => BYTE1_OBUF_Q_143
    );
  BYTE1_OBUF_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_BYTE1_OBUF_D_IN0,
      I1 => NlwBufferSignal_BYTE1_OBUF_D_IN1,
      O => BYTE1_OBUF_D_144
    );
  BYTE1_OBUF_D1 : X_ZERO
    port map (
      O => BYTE1_OBUF_D1_145
    );
  BYTE1_OBUF_D2_PT_0 : X_AND2
    port map (
      I0 => NlwBufferSignal_BYTE1_OBUF_D2_PT_0_IN0,
      I1 => NlwBufferSignal_BYTE1_OBUF_D2_PT_0_IN1,
      O => BYTE1_OBUF_D2_PT_0_147
    );
  BYTE1_OBUF_D2_PT_1 : X_AND3
    port map (
      I0 => NlwBufferSignal_BYTE1_OBUF_D2_PT_1_IN0,
      I1 => NlwInverterSignal_BYTE1_OBUF_D2_PT_1_IN1,
      I2 => NlwInverterSignal_BYTE1_OBUF_D2_PT_1_IN2,
      O => BYTE1_OBUF_D2_PT_1_148
    );
  BYTE1_OBUF_D2_PT_2 : X_AND4
    port map (
      I0 => NlwInverterSignal_BYTE1_OBUF_D2_PT_2_IN0,
      I1 => NlwInverterSignal_BYTE1_OBUF_D2_PT_2_IN1,
      I2 => NlwBufferSignal_BYTE1_OBUF_D2_PT_2_IN2,
      I3 => NlwInverterSignal_BYTE1_OBUF_D2_PT_2_IN3,
      O => BYTE1_OBUF_D2_PT_2_149
    );
  BYTE1_OBUF_D2 : X_OR3
    port map (
      I0 => NlwBufferSignal_BYTE1_OBUF_D2_IN0,
      I1 => NlwBufferSignal_BYTE1_OBUF_D2_IN1,
      I2 => NlwBufferSignal_BYTE1_OBUF_D2_IN2,
      O => BYTE1_OBUF_D2_146
    );
  BYTE2_OBUF : X_BUF
    port map (
      I => BYTE2_OBUF_Q_150,
      O => BYTE2_OBUF_41
    );
  BYTE2_OBUF_Q : X_BUF
    port map (
      I => BYTE2_OBUF_D_151,
      O => BYTE2_OBUF_Q_150
    );
  BYTE2_OBUF_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_BYTE2_OBUF_D_IN0,
      I1 => NlwBufferSignal_BYTE2_OBUF_D_IN1,
      O => BYTE2_OBUF_D_151
    );
  BYTE2_OBUF_D1 : X_ZERO
    port map (
      O => BYTE2_OBUF_D1_152
    );
  BYTE2_OBUF_D2_PT_0 : X_AND2
    port map (
      I0 => NlwBufferSignal_BYTE2_OBUF_D2_PT_0_IN0,
      I1 => NlwBufferSignal_BYTE2_OBUF_D2_PT_0_IN1,
      O => BYTE2_OBUF_D2_PT_0_154
    );
  BYTE2_OBUF_D2_PT_1 : X_AND3
    port map (
      I0 => NlwBufferSignal_BYTE2_OBUF_D2_PT_1_IN0,
      I1 => NlwInverterSignal_BYTE2_OBUF_D2_PT_1_IN1,
      I2 => NlwInverterSignal_BYTE2_OBUF_D2_PT_1_IN2,
      O => BYTE2_OBUF_D2_PT_1_155
    );
  BYTE2_OBUF_D2 : X_OR2
    port map (
      I0 => NlwBufferSignal_BYTE2_OBUF_D2_IN0,
      I1 => NlwBufferSignal_BYTE2_OBUF_D2_IN1,
      O => BYTE2_OBUF_D2_153
    );
  BYTE3_OBUF : X_BUF
    port map (
      I => BYTE3_OBUF_Q_156,
      O => BYTE3_OBUF_43
    );
  BYTE3_OBUF_Q : X_BUF
    port map (
      I => BYTE3_OBUF_D_157,
      O => BYTE3_OBUF_Q_156
    );
  BYTE3_OBUF_D : X_XOR2
    port map (
      I0 => NlwInverterSignal_BYTE3_OBUF_D_IN0,
      I1 => NlwBufferSignal_BYTE3_OBUF_D_IN1,
      O => BYTE3_OBUF_D_157
    );
  BYTE3_OBUF_D1 : X_ZERO
    port map (
      O => BYTE3_OBUF_D1_158
    );
  BYTE3_OBUF_D2 : X_AND2
    port map (
      I0 => NlwInverterSignal_BYTE3_OBUF_D2_IN0,
      I1 => NlwInverterSignal_BYTE3_OBUF_D2_IN1,
      O => BYTE3_OBUF_D2_159
    );
  DRAM_CS_OBUF : X_BUF
    port map (
      I => DRAM_CS_OBUF_Q_160,
      O => DRAM_CS_OBUF_45
    );
  DRAM_CS_OBUF_Q : X_BUF
    port map (
      I => DRAM_CS_OBUF_D_161,
      O => DRAM_CS_OBUF_Q_160
    );
  DRAM_CS_OBUF_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_DRAM_CS_OBUF_D_IN0,
      I1 => NlwBufferSignal_DRAM_CS_OBUF_D_IN1,
      O => DRAM_CS_OBUF_D_161
    );
  DRAM_CS_OBUF_D1 : X_ZERO
    port map (
      O => DRAM_CS_OBUF_D1_162
    );
  DRAM_CS_OBUF_D2_PT_0 : X_AND2
    port map (
      I0 => NlwInverterSignal_DRAM_CS_OBUF_D2_PT_0_IN0,
      I1 => NlwInverterSignal_DRAM_CS_OBUF_D2_PT_0_IN1,
      O => DRAM_CS_OBUF_D2_PT_0_164
    );
  DRAM_CS_OBUF_D2_PT_1 : X_AND2
    port map (
      I0 => NlwBufferSignal_DRAM_CS_OBUF_D2_PT_1_IN0,
      I1 => NlwBufferSignal_DRAM_CS_OBUF_D2_PT_1_IN1,
      O => DRAM_CS_OBUF_D2_PT_1_165
    );
  DRAM_CS_OBUF_D2_PT_2 : X_AND3
    port map (
      I0 => NlwBufferSignal_DRAM_CS_OBUF_D2_PT_2_IN0,
      I1 => NlwBufferSignal_DRAM_CS_OBUF_D2_PT_2_IN1,
      I2 => NlwBufferSignal_DRAM_CS_OBUF_D2_PT_2_IN2,
      O => DRAM_CS_OBUF_D2_PT_2_166
    );
  DRAM_CS_OBUF_D2 : X_OR3
    port map (
      I0 => NlwBufferSignal_DRAM_CS_OBUF_D2_IN0,
      I1 => NlwBufferSignal_DRAM_CS_OBUF_D2_IN1,
      I2 => NlwBufferSignal_DRAM_CS_OBUF_D2_IN2,
      O => DRAM_CS_OBUF_D2_163
    );
  DSACK1_OBUF : X_BUF
    port map (
      I => DSACK1_OBUF_Q_167,
      O => DSACK1_OBUF_47
    );
  DSACK1_OBUF_Q : X_BUF
    port map (
      I => DSACK1_OBUF_D_168,
      O => DSACK1_OBUF_Q_167
    );
  DSACK1_OBUF_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_DSACK1_OBUF_D_IN0,
      I1 => NlwBufferSignal_DSACK1_OBUF_D_IN1,
      O => DSACK1_OBUF_D_168
    );
  DSACK1_OBUF_D1 : X_ZERO
    port map (
      O => DSACK1_OBUF_D1_169
    );
  DSACK1_OBUF_D2_PT_0 : X_AND2
    port map (
      I0 => NlwInverterSignal_DSACK1_OBUF_D2_PT_0_IN0,
      I1 => NlwInverterSignal_DSACK1_OBUF_D2_PT_0_IN1,
      O => DSACK1_OBUF_D2_PT_0_171
    );
  DSACK1_OBUF_D2_PT_1 : X_AND3
    port map (
      I0 => NlwInverterSignal_DSACK1_OBUF_D2_PT_1_IN0,
      I1 => NlwInverterSignal_DSACK1_OBUF_D2_PT_1_IN1,
      I2 => NlwInverterSignal_DSACK1_OBUF_D2_PT_1_IN2,
      O => DSACK1_OBUF_D2_PT_1_172
    );
  DSACK1_OBUF_D2_PT_2 : X_AND3
    port map (
      I0 => NlwInverterSignal_DSACK1_OBUF_D2_PT_2_IN0,
      I1 => NlwInverterSignal_DSACK1_OBUF_D2_PT_2_IN1,
      I2 => NlwInverterSignal_DSACK1_OBUF_D2_PT_2_IN2,
      O => DSACK1_OBUF_D2_PT_2_173
    );
  DSACK1_OBUF_D2 : X_OR3
    port map (
      I0 => NlwBufferSignal_DSACK1_OBUF_D2_IN0,
      I1 => NlwBufferSignal_DSACK1_OBUF_D2_IN1,
      I2 => NlwBufferSignal_DSACK1_OBUF_D2_IN2,
      O => DSACK1_OBUF_D2_170
    );
  ETHRNT_OBUF : X_BUF
    port map (
      I => ETHRNT_OBUF_Q_174,
      O => ETHRNT_OBUF_49
    );
  ETHRNT_OBUF_Q : X_BUF
    port map (
      I => ETHRNT_OBUF_D_175,
      O => ETHRNT_OBUF_Q_174
    );
  ETHRNT_OBUF_D : X_XOR2
    port map (
      I0 => NlwInverterSignal_ETHRNT_OBUF_D_IN0,
      I1 => NlwBufferSignal_ETHRNT_OBUF_D_IN1,
      O => ETHRNT_OBUF_D_175
    );
  ETHRNT_OBUF_D1 : X_ZERO
    port map (
      O => ETHRNT_OBUF_D1_176
    );
  ETHRNT_OBUF_D2_PT_0 : X_AND7
    port map (
      I0 => NlwInverterSignal_ETHRNT_OBUF_D2_PT_0_IN0,
      I1 => NlwInverterSignal_ETHRNT_OBUF_D2_PT_0_IN1,
      I2 => NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN2,
      I3 => NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN3,
      I4 => NlwInverterSignal_ETHRNT_OBUF_D2_PT_0_IN4,
      I5 => NlwInverterSignal_ETHRNT_OBUF_D2_PT_0_IN5,
      I6 => NlwInverterSignal_ETHRNT_OBUF_D2_PT_0_IN6,
      O => ETHRNT_OBUF_D2_PT_0_178
    );
  ETHRNT_OBUF_D2_PT_1 : X_AND7
    port map (
      I0 => NlwInverterSignal_ETHRNT_OBUF_D2_PT_1_IN0,
      I1 => NlwInverterSignal_ETHRNT_OBUF_D2_PT_1_IN1,
      I2 => NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN2,
      I3 => NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN3,
      I4 => NlwInverterSignal_ETHRNT_OBUF_D2_PT_1_IN4,
      I5 => NlwInverterSignal_ETHRNT_OBUF_D2_PT_1_IN5,
      I6 => NlwInverterSignal_ETHRNT_OBUF_D2_PT_1_IN6,
      O => ETHRNT_OBUF_D2_PT_1_179
    );
  ETHRNT_OBUF_D2_PT_2 : X_AND7
    port map (
      I0 => NlwInverterSignal_ETHRNT_OBUF_D2_PT_2_IN0,
      I1 => NlwInverterSignal_ETHRNT_OBUF_D2_PT_2_IN1,
      I2 => NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN2,
      I3 => NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN3,
      I4 => NlwInverterSignal_ETHRNT_OBUF_D2_PT_2_IN4,
      I5 => NlwInverterSignal_ETHRNT_OBUF_D2_PT_2_IN5,
      I6 => NlwInverterSignal_ETHRNT_OBUF_D2_PT_2_IN6,
      O => ETHRNT_OBUF_D2_PT_2_180
    );
  ETHRNT_OBUF_D2 : X_OR3
    port map (
      I0 => NlwBufferSignal_ETHRNT_OBUF_D2_IN0,
      I1 => NlwBufferSignal_ETHRNT_OBUF_D2_IN1,
      I2 => NlwBufferSignal_ETHRNT_OBUF_D2_IN2,
      O => ETHRNT_OBUF_D2_177
    );
  FLOPPY_OBUF : X_BUF
    port map (
      I => FLOPPY_OBUF_Q_181,
      O => FLOPPY_OBUF_51
    );
  FLOPPY_OBUF_Q : X_BUF
    port map (
      I => FLOPPY_OBUF_D_182,
      O => FLOPPY_OBUF_Q_181
    );
  FLOPPY_OBUF_D : X_XOR2
    port map (
      I0 => NlwInverterSignal_FLOPPY_OBUF_D_IN0,
      I1 => NlwBufferSignal_FLOPPY_OBUF_D_IN1,
      O => FLOPPY_OBUF_D_182
    );
  FLOPPY_OBUF_D1 : X_ZERO
    port map (
      O => FLOPPY_OBUF_D1_183
    );
  FLOPPY_OBUF_D2_PT_0 : X_AND7
    port map (
      I0 => NlwInverterSignal_FLOPPY_OBUF_D2_PT_0_IN0,
      I1 => NlwInverterSignal_FLOPPY_OBUF_D2_PT_0_IN1,
      I2 => NlwInverterSignal_FLOPPY_OBUF_D2_PT_0_IN2,
      I3 => NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN3,
      I4 => NlwInverterSignal_FLOPPY_OBUF_D2_PT_0_IN4,
      I5 => NlwInverterSignal_FLOPPY_OBUF_D2_PT_0_IN5,
      I6 => NlwInverterSignal_FLOPPY_OBUF_D2_PT_0_IN6,
      O => FLOPPY_OBUF_D2_PT_0_185
    );
  FLOPPY_OBUF_D2_PT_1 : X_AND7
    port map (
      I0 => NlwInverterSignal_FLOPPY_OBUF_D2_PT_1_IN0,
      I1 => NlwInverterSignal_FLOPPY_OBUF_D2_PT_1_IN1,
      I2 => NlwInverterSignal_FLOPPY_OBUF_D2_PT_1_IN2,
      I3 => NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN3,
      I4 => NlwInverterSignal_FLOPPY_OBUF_D2_PT_1_IN4,
      I5 => NlwInverterSignal_FLOPPY_OBUF_D2_PT_1_IN5,
      I6 => NlwInverterSignal_FLOPPY_OBUF_D2_PT_1_IN6,
      O => FLOPPY_OBUF_D2_PT_1_186
    );
  FLOPPY_OBUF_D2_PT_2 : X_AND7
    port map (
      I0 => NlwInverterSignal_FLOPPY_OBUF_D2_PT_2_IN0,
      I1 => NlwInverterSignal_FLOPPY_OBUF_D2_PT_2_IN1,
      I2 => NlwInverterSignal_FLOPPY_OBUF_D2_PT_2_IN2,
      I3 => NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN3,
      I4 => NlwInverterSignal_FLOPPY_OBUF_D2_PT_2_IN4,
      I5 => NlwInverterSignal_FLOPPY_OBUF_D2_PT_2_IN5,
      I6 => NlwInverterSignal_FLOPPY_OBUF_D2_PT_2_IN6,
      O => FLOPPY_OBUF_D2_PT_2_187
    );
  FLOPPY_OBUF_D2 : X_OR3
    port map (
      I0 => NlwBufferSignal_FLOPPY_OBUF_D2_IN0,
      I1 => NlwBufferSignal_FLOPPY_OBUF_D2_IN1,
      I2 => NlwBufferSignal_FLOPPY_OBUF_D2_IN2,
      O => FLOPPY_OBUF_D2_184
    );
  IDE_OBUF : X_BUF
    port map (
      I => IDE_OBUF_Q_188,
      O => IDE_OBUF_53
    );
  IDE_OBUF_Q : X_BUF
    port map (
      I => IDE_OBUF_D_189,
      O => IDE_OBUF_Q_188
    );
  IDE_OBUF_D : X_XOR2
    port map (
      I0 => NlwInverterSignal_IDE_OBUF_D_IN0,
      I1 => NlwBufferSignal_IDE_OBUF_D_IN1,
      O => IDE_OBUF_D_189
    );
  IDE_OBUF_D1 : X_ZERO
    port map (
      O => IDE_OBUF_D1_190
    );
  IDE_OBUF_D2_PT_0 : X_AND7
    port map (
      I0 => NlwInverterSignal_IDE_OBUF_D2_PT_0_IN0,
      I1 => NlwInverterSignal_IDE_OBUF_D2_PT_0_IN1,
      I2 => NlwInverterSignal_IDE_OBUF_D2_PT_0_IN2,
      I3 => NlwInverterSignal_IDE_OBUF_D2_PT_0_IN3,
      I4 => NlwBufferSignal_IDE_OBUF_D2_PT_0_IN4,
      I5 => NlwInverterSignal_IDE_OBUF_D2_PT_0_IN5,
      I6 => NlwInverterSignal_IDE_OBUF_D2_PT_0_IN6,
      O => IDE_OBUF_D2_PT_0_192
    );
  IDE_OBUF_D2_PT_1 : X_AND7
    port map (
      I0 => NlwInverterSignal_IDE_OBUF_D2_PT_1_IN0,
      I1 => NlwInverterSignal_IDE_OBUF_D2_PT_1_IN1,
      I2 => NlwInverterSignal_IDE_OBUF_D2_PT_1_IN2,
      I3 => NlwInverterSignal_IDE_OBUF_D2_PT_1_IN3,
      I4 => NlwBufferSignal_IDE_OBUF_D2_PT_1_IN4,
      I5 => NlwInverterSignal_IDE_OBUF_D2_PT_1_IN5,
      I6 => NlwInverterSignal_IDE_OBUF_D2_PT_1_IN6,
      O => IDE_OBUF_D2_PT_1_193
    );
  IDE_OBUF_D2_PT_2 : X_AND7
    port map (
      I0 => NlwInverterSignal_IDE_OBUF_D2_PT_2_IN0,
      I1 => NlwInverterSignal_IDE_OBUF_D2_PT_2_IN1,
      I2 => NlwInverterSignal_IDE_OBUF_D2_PT_2_IN2,
      I3 => NlwInverterSignal_IDE_OBUF_D2_PT_2_IN3,
      I4 => NlwBufferSignal_IDE_OBUF_D2_PT_2_IN4,
      I5 => NlwInverterSignal_IDE_OBUF_D2_PT_2_IN5,
      I6 => NlwInverterSignal_IDE_OBUF_D2_PT_2_IN6,
      O => IDE_OBUF_D2_PT_2_194
    );
  IDE_OBUF_D2 : X_OR3
    port map (
      I0 => NlwBufferSignal_IDE_OBUF_D2_IN0,
      I1 => NlwBufferSignal_IDE_OBUF_D2_IN1,
      I2 => NlwBufferSignal_IDE_OBUF_D2_IN2,
      O => IDE_OBUF_D2_191
    );
  IO_CS_OBUF : X_BUF
    port map (
      I => IO_CS_OBUF_Q_195,
      O => IO_CS_OBUF_55
    );
  IO_CS_OBUF_Q : X_BUF
    port map (
      I => IO_CS_OBUF_D_196,
      O => IO_CS_OBUF_Q_195
    );
  IO_CS_OBUF_D : X_XOR2
    port map (
      I0 => NlwInverterSignal_IO_CS_OBUF_D_IN0,
      I1 => NlwBufferSignal_IO_CS_OBUF_D_IN1,
      O => IO_CS_OBUF_D_196
    );
  IO_CS_OBUF_D1 : X_ZERO
    port map (
      O => IO_CS_OBUF_D1_197
    );
  IO_CS_OBUF_D2_PT_0 : X_AND2
    port map (
      I0 => NlwBufferSignal_IO_CS_OBUF_D2_PT_0_IN0,
      I1 => NlwBufferSignal_IO_CS_OBUF_D2_PT_0_IN1,
      O => IO_CS_OBUF_D2_PT_0_200
    );
  IO_CS_OBUF_D2_PT_1 : X_AND7
    port map (
      I0 => NlwInverterSignal_IO_CS_OBUF_D2_PT_1_IN0,
      I1 => NlwInverterSignal_IO_CS_OBUF_D2_PT_1_IN1,
      I2 => NlwInverterSignal_IO_CS_OBUF_D2_PT_1_IN2,
      I3 => NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN3,
      I4 => NlwInverterSignal_IO_CS_OBUF_D2_PT_1_IN4,
      I5 => NlwInverterSignal_IO_CS_OBUF_D2_PT_1_IN5,
      I6 => NlwInverterSignal_IO_CS_OBUF_D2_PT_1_IN6,
      O => IO_CS_OBUF_D2_PT_1_201
    );
  IO_CS_OBUF_D2_PT_2 : X_AND7
    port map (
      I0 => NlwInverterSignal_IO_CS_OBUF_D2_PT_2_IN0,
      I1 => NlwInverterSignal_IO_CS_OBUF_D2_PT_2_IN1,
      I2 => NlwInverterSignal_IO_CS_OBUF_D2_PT_2_IN2,
      I3 => NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN3,
      I4 => NlwInverterSignal_IO_CS_OBUF_D2_PT_2_IN4,
      I5 => NlwInverterSignal_IO_CS_OBUF_D2_PT_2_IN5,
      I6 => NlwInverterSignal_IO_CS_OBUF_D2_PT_2_IN6,
      O => IO_CS_OBUF_D2_PT_2_202
    );
  IO_CS_OBUF_D2_PT_3 : X_AND7
    port map (
      I0 => NlwInverterSignal_IO_CS_OBUF_D2_PT_3_IN0,
      I1 => NlwInverterSignal_IO_CS_OBUF_D2_PT_3_IN1,
      I2 => NlwInverterSignal_IO_CS_OBUF_D2_PT_3_IN2,
      I3 => NlwInverterSignal_IO_CS_OBUF_D2_PT_3_IN3,
      I4 => NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN4,
      I5 => NlwInverterSignal_IO_CS_OBUF_D2_PT_3_IN5,
      I6 => NlwInverterSignal_IO_CS_OBUF_D2_PT_3_IN6,
      O => IO_CS_OBUF_D2_PT_3_203
    );
  IO_CS_OBUF_D2_PT_4 : X_AND7
    port map (
      I0 => NlwInverterSignal_IO_CS_OBUF_D2_PT_4_IN0,
      I1 => NlwInverterSignal_IO_CS_OBUF_D2_PT_4_IN1,
      I2 => NlwInverterSignal_IO_CS_OBUF_D2_PT_4_IN2,
      I3 => NlwInverterSignal_IO_CS_OBUF_D2_PT_4_IN3,
      I4 => NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN4,
      I5 => NlwInverterSignal_IO_CS_OBUF_D2_PT_4_IN5,
      I6 => NlwInverterSignal_IO_CS_OBUF_D2_PT_4_IN6,
      O => IO_CS_OBUF_D2_PT_4_204
    );
  IO_CS_OBUF_D2_PT_5 : X_AND7
    port map (
      I0 => NlwInverterSignal_IO_CS_OBUF_D2_PT_5_IN0,
      I1 => NlwInverterSignal_IO_CS_OBUF_D2_PT_5_IN1,
      I2 => NlwInverterSignal_IO_CS_OBUF_D2_PT_5_IN2,
      I3 => NlwInverterSignal_IO_CS_OBUF_D2_PT_5_IN3,
      I4 => NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN4,
      I5 => NlwInverterSignal_IO_CS_OBUF_D2_PT_5_IN5,
      I6 => NlwInverterSignal_IO_CS_OBUF_D2_PT_5_IN6,
      O => IO_CS_OBUF_D2_PT_5_205
    );
  IO_CS_OBUF_D2 : X_OR6
    port map (
      I0 => NlwBufferSignal_IO_CS_OBUF_D2_IN0,
      I1 => NlwBufferSignal_IO_CS_OBUF_D2_IN1,
      I2 => NlwBufferSignal_IO_CS_OBUF_D2_IN2,
      I3 => NlwBufferSignal_IO_CS_OBUF_D2_IN3,
      I4 => NlwBufferSignal_IO_CS_OBUF_D2_IN4,
      I5 => NlwBufferSignal_IO_CS_OBUF_D2_IN5,
      O => IO_CS_OBUF_D2_198
    );
  KBD_CS_OBUF : X_BUF
    port map (
      I => KBD_CS_OBUF_Q_206,
      O => KBD_CS_OBUF_57
    );
  KBD_CS_OBUF_Q : X_BUF
    port map (
      I => KBD_CS_OBUF_D_207,
      O => KBD_CS_OBUF_Q_206
    );
  KBD_CS_OBUF_D : X_XOR2
    port map (
      I0 => NlwInverterSignal_KBD_CS_OBUF_D_IN0,
      I1 => NlwBufferSignal_KBD_CS_OBUF_D_IN1,
      O => KBD_CS_OBUF_D_207
    );
  KBD_CS_OBUF_D1 : X_ZERO
    port map (
      O => KBD_CS_OBUF_D1_208
    );
  KBD_CS_OBUF_D2_PT_0 : X_AND7
    port map (
      I0 => NlwInverterSignal_KBD_CS_OBUF_D2_PT_0_IN0,
      I1 => NlwInverterSignal_KBD_CS_OBUF_D2_PT_0_IN1,
      I2 => NlwInverterSignal_KBD_CS_OBUF_D2_PT_0_IN2,
      I3 => NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN3,
      I4 => NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN4,
      I5 => NlwInverterSignal_KBD_CS_OBUF_D2_PT_0_IN5,
      I6 => NlwInverterSignal_KBD_CS_OBUF_D2_PT_0_IN6,
      O => KBD_CS_OBUF_D2_PT_0_210
    );
  KBD_CS_OBUF_D2_PT_1 : X_AND7
    port map (
      I0 => NlwInverterSignal_KBD_CS_OBUF_D2_PT_1_IN0,
      I1 => NlwInverterSignal_KBD_CS_OBUF_D2_PT_1_IN1,
      I2 => NlwInverterSignal_KBD_CS_OBUF_D2_PT_1_IN2,
      I3 => NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN3,
      I4 => NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN4,
      I5 => NlwInverterSignal_KBD_CS_OBUF_D2_PT_1_IN5,
      I6 => NlwInverterSignal_KBD_CS_OBUF_D2_PT_1_IN6,
      O => KBD_CS_OBUF_D2_PT_1_211
    );
  KBD_CS_OBUF_D2_PT_2 : X_AND7
    port map (
      I0 => NlwInverterSignal_KBD_CS_OBUF_D2_PT_2_IN0,
      I1 => NlwInverterSignal_KBD_CS_OBUF_D2_PT_2_IN1,
      I2 => NlwInverterSignal_KBD_CS_OBUF_D2_PT_2_IN2,
      I3 => NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN3,
      I4 => NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN4,
      I5 => NlwInverterSignal_KBD_CS_OBUF_D2_PT_2_IN5,
      I6 => NlwInverterSignal_KBD_CS_OBUF_D2_PT_2_IN6,
      O => KBD_CS_OBUF_D2_PT_2_212
    );
  KBD_CS_OBUF_D2 : X_OR3
    port map (
      I0 => NlwBufferSignal_KBD_CS_OBUF_D2_IN0,
      I1 => NlwBufferSignal_KBD_CS_OBUF_D2_IN1,
      I2 => NlwBufferSignal_KBD_CS_OBUF_D2_IN2,
      O => KBD_CS_OBUF_D2_209
    );
  ROM_OBUF : X_BUF
    port map (
      I => ROM_OBUF_Q_213,
      O => ROM_OBUF_59
    );
  ROM_OBUF_Q : X_BUF
    port map (
      I => ROM_OBUF_D_214,
      O => ROM_OBUF_Q_213
    );
  ROM_OBUF_D : X_XOR2
    port map (
      I0 => NlwInverterSignal_ROM_OBUF_D_IN0,
      I1 => NlwBufferSignal_ROM_OBUF_D_IN1,
      O => ROM_OBUF_D_214
    );
  ROM_OBUF_D1 : X_ZERO
    port map (
      O => ROM_OBUF_D1_215
    );
  ROM_OBUF_D2_PT_0 : X_AND7
    port map (
      I0 => NlwInverterSignal_ROM_OBUF_D2_PT_0_IN0,
      I1 => NlwInverterSignal_ROM_OBUF_D2_PT_0_IN1,
      I2 => NlwInverterSignal_ROM_OBUF_D2_PT_0_IN2,
      I3 => NlwInverterSignal_ROM_OBUF_D2_PT_0_IN3,
      I4 => NlwInverterSignal_ROM_OBUF_D2_PT_0_IN4,
      I5 => NlwInverterSignal_ROM_OBUF_D2_PT_0_IN5,
      I6 => NlwInverterSignal_ROM_OBUF_D2_PT_0_IN6,
      O => ROM_OBUF_D2_PT_0_217
    );
  ROM_OBUF_D2_PT_1 : X_AND7
    port map (
      I0 => NlwInverterSignal_ROM_OBUF_D2_PT_1_IN0,
      I1 => NlwInverterSignal_ROM_OBUF_D2_PT_1_IN1,
      I2 => NlwInverterSignal_ROM_OBUF_D2_PT_1_IN2,
      I3 => NlwInverterSignal_ROM_OBUF_D2_PT_1_IN3,
      I4 => NlwInverterSignal_ROM_OBUF_D2_PT_1_IN4,
      I5 => NlwInverterSignal_ROM_OBUF_D2_PT_1_IN5,
      I6 => NlwInverterSignal_ROM_OBUF_D2_PT_1_IN6,
      O => ROM_OBUF_D2_PT_1_218
    );
  ROM_OBUF_D2_PT_2 : X_AND7
    port map (
      I0 => NlwInverterSignal_ROM_OBUF_D2_PT_2_IN0,
      I1 => NlwInverterSignal_ROM_OBUF_D2_PT_2_IN1,
      I2 => NlwInverterSignal_ROM_OBUF_D2_PT_2_IN2,
      I3 => NlwInverterSignal_ROM_OBUF_D2_PT_2_IN3,
      I4 => NlwInverterSignal_ROM_OBUF_D2_PT_2_IN4,
      I5 => NlwInverterSignal_ROM_OBUF_D2_PT_2_IN5,
      I6 => NlwInverterSignal_ROM_OBUF_D2_PT_2_IN6,
      O => ROM_OBUF_D2_PT_2_219
    );
  ROM_OBUF_D2 : X_OR3
    port map (
      I0 => NlwBufferSignal_ROM_OBUF_D2_IN0,
      I1 => NlwBufferSignal_ROM_OBUF_D2_IN1,
      I2 => NlwBufferSignal_ROM_OBUF_D2_IN2,
      O => ROM_OBUF_D2_216
    );
  RW0_OBUF : X_BUF
    port map (
      I => RW0_OBUF_Q_220,
      O => RW0_OBUF_61
    );
  RW0_OBUF_Q : X_BUF
    port map (
      I => RW0_OBUF_D_221,
      O => RW0_OBUF_Q_220
    );
  RW0_OBUF_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_RW0_OBUF_D_IN0,
      I1 => NlwBufferSignal_RW0_OBUF_D_IN1,
      O => RW0_OBUF_D_221
    );
  RW0_OBUF_D1 : X_ZERO
    port map (
      O => RW0_OBUF_D1_222
    );
  RW0_OBUF_D2_PT_0 : X_AND2
    port map (
      I0 => NlwBufferSignal_RW0_OBUF_D2_PT_0_IN0,
      I1 => NlwBufferSignal_RW0_OBUF_D2_PT_0_IN1,
      O => RW0_OBUF_D2_PT_0_224
    );
  RW0_OBUF_D2_PT_1 : X_AND2
    port map (
      I0 => NlwInverterSignal_RW0_OBUF_D2_PT_1_IN0,
      I1 => NlwInverterSignal_RW0_OBUF_D2_PT_1_IN1,
      O => RW0_OBUF_D2_PT_1_225
    );
  RW0_OBUF_D2_PT_2 : X_AND2
    port map (
      I0 => NlwBufferSignal_RW0_OBUF_D2_PT_2_IN0,
      I1 => NlwBufferSignal_RW0_OBUF_D2_PT_2_IN1,
      O => RW0_OBUF_D2_PT_2_226
    );
  RW0_OBUF_D2_PT_3 : X_AND2
    port map (
      I0 => NlwBufferSignal_RW0_OBUF_D2_PT_3_IN0,
      I1 => NlwBufferSignal_RW0_OBUF_D2_PT_3_IN1,
      O => RW0_OBUF_D2_PT_3_227
    );
  RW0_OBUF_D2_PT_4 : X_AND3
    port map (
      I0 => NlwBufferSignal_RW0_OBUF_D2_PT_4_IN0,
      I1 => NlwBufferSignal_RW0_OBUF_D2_PT_4_IN1,
      I2 => NlwBufferSignal_RW0_OBUF_D2_PT_4_IN2,
      O => RW0_OBUF_D2_PT_4_228
    );
  RW0_OBUF_D2 : X_OR5
    port map (
      I0 => NlwBufferSignal_RW0_OBUF_D2_IN0,
      I1 => NlwBufferSignal_RW0_OBUF_D2_IN1,
      I2 => NlwBufferSignal_RW0_OBUF_D2_IN2,
      I3 => NlwBufferSignal_RW0_OBUF_D2_IN3,
      I4 => NlwBufferSignal_RW0_OBUF_D2_IN4,
      O => RW0_OBUF_D2_223
    );
  RW1_OBUF : X_BUF
    port map (
      I => RW1_OBUF_Q_229,
      O => RW1_OBUF_63
    );
  RW1_OBUF_Q : X_BUF
    port map (
      I => RW1_OBUF_D_230,
      O => RW1_OBUF_Q_229
    );
  RW1_OBUF_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_RW1_OBUF_D_IN0,
      I1 => NlwBufferSignal_RW1_OBUF_D_IN1,
      O => RW1_OBUF_D_230
    );
  RW1_OBUF_D1 : X_ZERO
    port map (
      O => RW1_OBUF_D1_231
    );
  RW1_OBUF_D2_PT_0 : X_AND2
    port map (
      I0 => NlwBufferSignal_RW1_OBUF_D2_PT_0_IN0,
      I1 => NlwBufferSignal_RW1_OBUF_D2_PT_0_IN1,
      O => RW1_OBUF_D2_PT_0_233
    );
  RW1_OBUF_D2_PT_1 : X_AND2
    port map (
      I0 => NlwInverterSignal_RW1_OBUF_D2_PT_1_IN0,
      I1 => NlwInverterSignal_RW1_OBUF_D2_PT_1_IN1,
      O => RW1_OBUF_D2_PT_1_234
    );
  RW1_OBUF_D2_PT_2 : X_AND2
    port map (
      I0 => NlwBufferSignal_RW1_OBUF_D2_PT_2_IN0,
      I1 => NlwBufferSignal_RW1_OBUF_D2_PT_2_IN1,
      O => RW1_OBUF_D2_PT_2_235
    );
  RW1_OBUF_D2_PT_3 : X_AND2
    port map (
      I0 => NlwInverterSignal_RW1_OBUF_D2_PT_3_IN0,
      I1 => NlwBufferSignal_RW1_OBUF_D2_PT_3_IN1,
      O => RW1_OBUF_D2_PT_3_236
    );
  RW1_OBUF_D2_PT_4 : X_AND3
    port map (
      I0 => NlwBufferSignal_RW1_OBUF_D2_PT_4_IN0,
      I1 => NlwBufferSignal_RW1_OBUF_D2_PT_4_IN1,
      I2 => NlwBufferSignal_RW1_OBUF_D2_PT_4_IN2,
      O => RW1_OBUF_D2_PT_4_237
    );
  RW1_OBUF_D2 : X_OR5
    port map (
      I0 => NlwBufferSignal_RW1_OBUF_D2_IN0,
      I1 => NlwBufferSignal_RW1_OBUF_D2_IN1,
      I2 => NlwBufferSignal_RW1_OBUF_D2_IN2,
      I3 => NlwBufferSignal_RW1_OBUF_D2_IN3,
      I4 => NlwBufferSignal_RW1_OBUF_D2_IN4,
      O => RW1_OBUF_D2_232
    );
  VGA_CS_OBUF : X_BUF
    port map (
      I => VGA_CS_OBUF_Q_238,
      O => VGA_CS_OBUF_65
    );
  VGA_CS_OBUF_Q : X_BUF
    port map (
      I => VGA_CS_OBUF_D_239,
      O => VGA_CS_OBUF_Q_238
    );
  VGA_CS_OBUF_D : X_XOR2
    port map (
      I0 => NlwInverterSignal_VGA_CS_OBUF_D_IN0,
      I1 => NlwBufferSignal_VGA_CS_OBUF_D_IN1,
      O => VGA_CS_OBUF_D_239
    );
  VGA_CS_OBUF_D1 : X_ZERO
    port map (
      O => VGA_CS_OBUF_D1_240
    );
  VGA_CS_OBUF_D2_PT_0 : X_AND7
    port map (
      I0 => NlwInverterSignal_VGA_CS_OBUF_D2_PT_0_IN0,
      I1 => NlwInverterSignal_VGA_CS_OBUF_D2_PT_0_IN1,
      I2 => NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN2,
      I3 => NlwInverterSignal_VGA_CS_OBUF_D2_PT_0_IN3,
      I4 => NlwInverterSignal_VGA_CS_OBUF_D2_PT_0_IN4,
      I5 => NlwInverterSignal_VGA_CS_OBUF_D2_PT_0_IN5,
      I6 => NlwInverterSignal_VGA_CS_OBUF_D2_PT_0_IN6,
      O => VGA_CS_OBUF_D2_PT_0_242
    );
  VGA_CS_OBUF_D2_PT_1 : X_AND7
    port map (
      I0 => NlwInverterSignal_VGA_CS_OBUF_D2_PT_1_IN0,
      I1 => NlwInverterSignal_VGA_CS_OBUF_D2_PT_1_IN1,
      I2 => NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN2,
      I3 => NlwInverterSignal_VGA_CS_OBUF_D2_PT_1_IN3,
      I4 => NlwInverterSignal_VGA_CS_OBUF_D2_PT_1_IN4,
      I5 => NlwInverterSignal_VGA_CS_OBUF_D2_PT_1_IN5,
      I6 => NlwInverterSignal_VGA_CS_OBUF_D2_PT_1_IN6,
      O => VGA_CS_OBUF_D2_PT_1_243
    );
  VGA_CS_OBUF_D2_PT_2 : X_AND7
    port map (
      I0 => NlwInverterSignal_VGA_CS_OBUF_D2_PT_2_IN0,
      I1 => NlwInverterSignal_VGA_CS_OBUF_D2_PT_2_IN1,
      I2 => NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN2,
      I3 => NlwInverterSignal_VGA_CS_OBUF_D2_PT_2_IN3,
      I4 => NlwInverterSignal_VGA_CS_OBUF_D2_PT_2_IN4,
      I5 => NlwInverterSignal_VGA_CS_OBUF_D2_PT_2_IN5,
      I6 => NlwInverterSignal_VGA_CS_OBUF_D2_PT_2_IN6,
      O => VGA_CS_OBUF_D2_PT_2_244
    );
  VGA_CS_OBUF_D2 : X_OR3
    port map (
      I0 => NlwBufferSignal_VGA_CS_OBUF_D2_IN0,
      I1 => NlwBufferSignal_VGA_CS_OBUF_D2_IN1,
      I2 => NlwBufferSignal_VGA_CS_OBUF_D2_IN2,
      O => VGA_CS_OBUF_D2_241
    );
  nRW0_OBUF : X_BUF
    port map (
      I => nRW0_OBUF_Q_245,
      O => nRW0_OBUF_67
    );
  nRW0_OBUF_Q : X_BUF
    port map (
      I => nRW0_OBUF_D_246,
      O => nRW0_OBUF_Q_245
    );
  nRW0_OBUF_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_nRW0_OBUF_D_IN0,
      I1 => NlwBufferSignal_nRW0_OBUF_D_IN1,
      O => nRW0_OBUF_D_246
    );
  nRW0_OBUF_D1 : X_ZERO
    port map (
      O => nRW0_OBUF_D1_247
    );
  nRW0_OBUF_D2_PT_0 : X_AND2
    port map (
      I0 => NlwBufferSignal_nRW0_OBUF_D2_PT_0_IN0,
      I1 => NlwBufferSignal_nRW0_OBUF_D2_PT_0_IN1,
      O => nRW0_OBUF_D2_PT_0_249
    );
  nRW0_OBUF_D2_PT_1 : X_AND2
    port map (
      I0 => NlwInverterSignal_nRW0_OBUF_D2_PT_1_IN0,
      I1 => NlwInverterSignal_nRW0_OBUF_D2_PT_1_IN1,
      O => nRW0_OBUF_D2_PT_1_250
    );
  nRW0_OBUF_D2_PT_2 : X_AND2
    port map (
      I0 => NlwBufferSignal_nRW0_OBUF_D2_PT_2_IN0,
      I1 => NlwBufferSignal_nRW0_OBUF_D2_PT_2_IN1,
      O => nRW0_OBUF_D2_PT_2_251
    );
  nRW0_OBUF_D2_PT_3 : X_AND2
    port map (
      I0 => NlwBufferSignal_nRW0_OBUF_D2_PT_3_IN0,
      I1 => NlwInverterSignal_nRW0_OBUF_D2_PT_3_IN1,
      O => nRW0_OBUF_D2_PT_3_252
    );
  nRW0_OBUF_D2_PT_4 : X_AND3
    port map (
      I0 => NlwBufferSignal_nRW0_OBUF_D2_PT_4_IN0,
      I1 => NlwBufferSignal_nRW0_OBUF_D2_PT_4_IN1,
      I2 => NlwBufferSignal_nRW0_OBUF_D2_PT_4_IN2,
      O => nRW0_OBUF_D2_PT_4_253
    );
  nRW0_OBUF_D2 : X_OR5
    port map (
      I0 => NlwBufferSignal_nRW0_OBUF_D2_IN0,
      I1 => NlwBufferSignal_nRW0_OBUF_D2_IN1,
      I2 => NlwBufferSignal_nRW0_OBUF_D2_IN2,
      I3 => NlwBufferSignal_nRW0_OBUF_D2_IN3,
      I4 => NlwBufferSignal_nRW0_OBUF_D2_IN4,
      O => nRW0_OBUF_D2_248
    );
  nRW1_OBUF : X_BUF
    port map (
      I => nRW1_OBUF_Q_254,
      O => nRW1_OBUF_69
    );
  nRW1_OBUF_Q : X_BUF
    port map (
      I => nRW1_OBUF_D_255,
      O => nRW1_OBUF_Q_254
    );
  nRW1_OBUF_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_nRW1_OBUF_D_IN0,
      I1 => NlwBufferSignal_nRW1_OBUF_D_IN1,
      O => nRW1_OBUF_D_255
    );
  nRW1_OBUF_D1 : X_ZERO
    port map (
      O => nRW1_OBUF_D1_256
    );
  nRW1_OBUF_D2_PT_0 : X_AND2
    port map (
      I0 => NlwBufferSignal_nRW1_OBUF_D2_PT_0_IN0,
      I1 => NlwBufferSignal_nRW1_OBUF_D2_PT_0_IN1,
      O => nRW1_OBUF_D2_PT_0_258
    );
  nRW1_OBUF_D2_PT_1 : X_AND2
    port map (
      I0 => NlwInverterSignal_nRW1_OBUF_D2_PT_1_IN0,
      I1 => NlwInverterSignal_nRW1_OBUF_D2_PT_1_IN1,
      O => nRW1_OBUF_D2_PT_1_259
    );
  nRW1_OBUF_D2_PT_2 : X_AND2
    port map (
      I0 => NlwBufferSignal_nRW1_OBUF_D2_PT_2_IN0,
      I1 => NlwBufferSignal_nRW1_OBUF_D2_PT_2_IN1,
      O => nRW1_OBUF_D2_PT_2_260
    );
  nRW1_OBUF_D2_PT_3 : X_AND2
    port map (
      I0 => NlwInverterSignal_nRW1_OBUF_D2_PT_3_IN0,
      I1 => NlwInverterSignal_nRW1_OBUF_D2_PT_3_IN1,
      O => nRW1_OBUF_D2_PT_3_261
    );
  nRW1_OBUF_D2_PT_4 : X_AND3
    port map (
      I0 => NlwBufferSignal_nRW1_OBUF_D2_PT_4_IN0,
      I1 => NlwBufferSignal_nRW1_OBUF_D2_PT_4_IN1,
      I2 => NlwBufferSignal_nRW1_OBUF_D2_PT_4_IN2,
      O => nRW1_OBUF_D2_PT_4_262
    );
  nRW1_OBUF_D2 : X_OR5
    port map (
      I0 => NlwBufferSignal_nRW1_OBUF_D2_IN0,
      I1 => NlwBufferSignal_nRW1_OBUF_D2_IN1,
      I2 => NlwBufferSignal_nRW1_OBUF_D2_IN2,
      I3 => NlwBufferSignal_nRW1_OBUF_D2_IN3,
      I4 => NlwBufferSignal_nRW1_OBUF_D2_IN4,
      O => nRW1_OBUF_D2_257
    );
  HALT_OBUF_BUF0 : X_BUF
    port map (
      I => HALT_OBUF_BUF0_Q_263,
      O => HALT_OBUF_BUF0_71
    );
  HALT_OBUF_BUF0_Q : X_BUF
    port map (
      I => HALT_OBUF_BUF0_D_264,
      O => HALT_OBUF_BUF0_Q_263
    );
  HALT_OBUF_BUF0_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_HALT_OBUF_BUF0_D_IN0,
      I1 => NlwBufferSignal_HALT_OBUF_BUF0_D_IN1,
      O => HALT_OBUF_BUF0_D_264
    );
  HALT_OBUF_BUF0_D1 : X_ZERO
    port map (
      O => HALT_OBUF_BUF0_D1_265
    );
  HALT_OBUF_BUF0_D2 : X_AND2
    port map (
      I0 => NlwBufferSignal_HALT_OBUF_BUF0_D2_IN0,
      I1 => NlwBufferSignal_HALT_OBUF_BUF0_D2_IN1,
      O => HALT_OBUF_BUF0_D2_266
    );
  HALT_OBUF_BUF1 : X_BUF
    port map (
      I => HALT_OBUF_BUF1_Q_267,
      O => HALT_OBUF_BUF1_73
    );
  HALT_OBUF_BUF1_Q : X_BUF
    port map (
      I => HALT_OBUF_BUF1_D_268,
      O => HALT_OBUF_BUF1_Q_267
    );
  HALT_OBUF_BUF1_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_HALT_OBUF_BUF1_D_IN0,
      I1 => NlwBufferSignal_HALT_OBUF_BUF1_D_IN1,
      O => HALT_OBUF_BUF1_D_268
    );
  HALT_OBUF_BUF1_D1 : X_ZERO
    port map (
      O => HALT_OBUF_BUF1_D1_269
    );
  HALT_OBUF_BUF1_D2 : X_AND2
    port map (
      I0 => NlwBufferSignal_HALT_OBUF_BUF1_D2_IN0,
      I1 => NlwBufferSignal_HALT_OBUF_BUF1_D2_IN1,
      O => HALT_OBUF_BUF1_D2_270
    );
  READ_n_OBUF : X_BUF
    port map (
      I => READ_n_OBUF_Q_271,
      O => READ_n_OBUF_75
    );
  READ_n_OBUF_Q : X_BUF
    port map (
      I => READ_n_OBUF_D_272,
      O => READ_n_OBUF_Q_271
    );
  READ_n_OBUF_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_READ_n_OBUF_D_IN0,
      I1 => NlwBufferSignal_READ_n_OBUF_D_IN1,
      O => READ_n_OBUF_D_272
    );
  READ_n_OBUF_D1 : X_ZERO
    port map (
      O => READ_n_OBUF_D1_273
    );
  READ_n_OBUF_D2 : X_AND2
    port map (
      I0 => NlwInverterSignal_READ_n_OBUF_D2_IN0,
      I1 => NlwInverterSignal_READ_n_OBUF_D2_IN1,
      O => READ_n_OBUF_D2_274
    );
  I_ALE_OBUF_Q : X_BUF
    port map (
      I => I_ALE_OBUF_Q_275,
      O => I_ALE_OBUF_Q_77
    );
  I_ALE_OBUF_Q_296 : X_BUF
    port map (
      I => I_ALE_OBUF_D_276,
      O => I_ALE_OBUF_Q_275
    );
  I_ALE_OBUF_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_I_ALE_OBUF_D_IN0,
      I1 => NlwBufferSignal_I_ALE_OBUF_D_IN1,
      O => I_ALE_OBUF_D_276
    );
  I_ALE_OBUF_D1 : X_ZERO
    port map (
      O => I_ALE_OBUF_D1_277
    );
  I_ALE_OBUF_D2 : X_ZERO
    port map (
      O => I_ALE_OBUF_D2_278
    );
  I_ALE_OBUF_BUF0 : X_BUF
    port map (
      I => I_ALE_OBUF_BUF0_Q_279,
      O => I_ALE_OBUF_BUF0_79
    );
  I_ALE_OBUF_BUF0_Q : X_BUF
    port map (
      I => I_ALE_OBUF_BUF0_D_280,
      O => I_ALE_OBUF_BUF0_Q_279
    );
  I_ALE_OBUF_BUF0_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_I_ALE_OBUF_BUF0_D_IN0,
      I1 => NlwBufferSignal_I_ALE_OBUF_BUF0_D_IN1,
      O => I_ALE_OBUF_BUF0_D_280
    );
  I_ALE_OBUF_BUF0_D1 : X_ZERO
    port map (
      O => I_ALE_OBUF_BUF0_D1_281
    );
  I_ALE_OBUF_BUF0_D2 : X_ZERO
    port map (
      O => I_ALE_OBUF_BUF0_D2_282
    );
  I_ALE_OBUF_BUF1 : X_BUF
    port map (
      I => I_ALE_OBUF_BUF1_Q_283,
      O => I_ALE_OBUF_BUF1_81
    );
  I_ALE_OBUF_BUF1_Q : X_BUF
    port map (
      I => I_ALE_OBUF_BUF1_D_284,
      O => I_ALE_OBUF_BUF1_Q_283
    );
  I_ALE_OBUF_BUF1_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_I_ALE_OBUF_BUF1_D_IN0,
      I1 => NlwBufferSignal_I_ALE_OBUF_BUF1_D_IN1,
      O => I_ALE_OBUF_BUF1_D_284
    );
  I_ALE_OBUF_BUF1_D1 : X_ZERO
    port map (
      O => I_ALE_OBUF_BUF1_D1_285
    );
  I_ALE_OBUF_BUF1_D2 : X_ZERO
    port map (
      O => I_ALE_OBUF_BUF1_D2_286
    );
  I_MEMW_OBUF_Q : X_BUF
    port map (
      I => I_MEMW_OBUF_Q_287,
      O => I_MEMW_OBUF_Q_83
    );
  I_MEMW_OBUF_Q_311 : X_BUF
    port map (
      I => I_MEMW_OBUF_D_288,
      O => I_MEMW_OBUF_Q_287
    );
  I_MEMW_OBUF_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_I_MEMW_OBUF_D_IN0,
      I1 => NlwBufferSignal_I_MEMW_OBUF_D_IN1,
      O => I_MEMW_OBUF_D_288
    );
  I_MEMW_OBUF_D1 : X_ZERO
    port map (
      O => I_MEMW_OBUF_D1_289
    );
  I_MEMW_OBUF_D2 : X_ONE
    port map (
      O => I_MEMW_OBUF_D2_290
    );
  I_MEMW_OBUF_BUF0 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF0_Q_291,
      O => I_MEMW_OBUF_BUF0_85
    );
  I_MEMW_OBUF_BUF0_Q : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF0_D_292,
      O => I_MEMW_OBUF_BUF0_Q_291
    );
  I_MEMW_OBUF_BUF0_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_I_MEMW_OBUF_BUF0_D_IN0,
      I1 => NlwBufferSignal_I_MEMW_OBUF_BUF0_D_IN1,
      O => I_MEMW_OBUF_BUF0_D_292
    );
  I_MEMW_OBUF_BUF0_D1 : X_ZERO
    port map (
      O => I_MEMW_OBUF_BUF0_D1_293
    );
  I_MEMW_OBUF_BUF0_D2 : X_ONE
    port map (
      O => I_MEMW_OBUF_BUF0_D2_294
    );
  I_MEMW_OBUF_BUF1 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF1_Q_295,
      O => I_MEMW_OBUF_BUF1_87
    );
  I_MEMW_OBUF_BUF1_EXP : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_Q_296,
      O => I_MEMW_OBUF_BUF1_EXP_199
    );
  I_MEMW_OBUF_BUF1_Q : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF1_D_297,
      O => I_MEMW_OBUF_BUF1_Q_295
    );
  I_MEMW_OBUF_BUF1_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_I_MEMW_OBUF_BUF1_D_IN0,
      I1 => NlwBufferSignal_I_MEMW_OBUF_BUF1_D_IN1,
      O => I_MEMW_OBUF_BUF1_D_297
    );
  I_MEMW_OBUF_BUF1_D1 : X_ZERO
    port map (
      O => I_MEMW_OBUF_BUF1_D1_298
    );
  I_MEMW_OBUF_BUF1_D2 : X_ONE
    port map (
      O => I_MEMW_OBUF_BUF1_D2_299
    );
  I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_Q : X_AND7
    port map (
      I0 => NlwInverterSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN0,
      I1 => NlwInverterSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN1,
      I2 => NlwInverterSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN2,
      I3 => NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN3,
      I4 => NlwInverterSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN4,
      I5 => NlwInverterSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN5,
      I6 => NlwInverterSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN6,
      O => I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_Q_296
    );
  I_MEMW_OBUF_BUF2 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF2_Q_300,
      O => I_MEMW_OBUF_BUF2_89
    );
  I_MEMW_OBUF_BUF2_Q : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF2_D_301,
      O => I_MEMW_OBUF_BUF2_Q_300
    );
  I_MEMW_OBUF_BUF2_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_I_MEMW_OBUF_BUF2_D_IN0,
      I1 => NlwBufferSignal_I_MEMW_OBUF_BUF2_D_IN1,
      O => I_MEMW_OBUF_BUF2_D_301
    );
  I_MEMW_OBUF_BUF2_D1 : X_ZERO
    port map (
      O => I_MEMW_OBUF_BUF2_D1_302
    );
  I_MEMW_OBUF_BUF2_D2 : X_ONE
    port map (
      O => I_MEMW_OBUF_BUF2_D2_303
    );
  I_MEMW_OBUF_BUF3 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF3_Q_304,
      O => I_MEMW_OBUF_BUF3_91
    );
  I_MEMW_OBUF_BUF3_Q : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF3_D_305,
      O => I_MEMW_OBUF_BUF3_Q_304
    );
  I_MEMW_OBUF_BUF3_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_I_MEMW_OBUF_BUF3_D_IN0,
      I1 => NlwBufferSignal_I_MEMW_OBUF_BUF3_D_IN1,
      O => I_MEMW_OBUF_BUF3_D_305
    );
  I_MEMW_OBUF_BUF3_D1 : X_ZERO
    port map (
      O => I_MEMW_OBUF_BUF3_D1_306
    );
  I_MEMW_OBUF_BUF3_D2 : X_ONE
    port map (
      O => I_MEMW_OBUF_BUF3_D2_307
    );
  I_MEMW_OBUF_BUF4 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF4_Q_308,
      O => I_MEMW_OBUF_BUF4_93
    );
  I_MEMW_OBUF_BUF4_Q : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF4_D_309,
      O => I_MEMW_OBUF_BUF4_Q_308
    );
  I_MEMW_OBUF_BUF4_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_I_MEMW_OBUF_BUF4_D_IN0,
      I1 => NlwBufferSignal_I_MEMW_OBUF_BUF4_D_IN1,
      O => I_MEMW_OBUF_BUF4_D_309
    );
  I_MEMW_OBUF_BUF4_D1 : X_ZERO
    port map (
      O => I_MEMW_OBUF_BUF4_D1_310
    );
  I_MEMW_OBUF_BUF4_D2 : X_ONE
    port map (
      O => I_MEMW_OBUF_BUF4_D2_311
    );
  cntr_2_cntr_2_RSTF_INT_UIM : X_BUF
    port map (
      I => cntr_2_cntr_2_RSTF_INT_Q_312,
      O => cntr_2_cntr_2_RSTF_INT_UIM_113
    );
  cntr_2_cntr_2_RSTF_INT_Q : X_BUF
    port map (
      I => cntr_2_cntr_2_RSTF_INT_D_313,
      O => cntr_2_cntr_2_RSTF_INT_Q_312
    );
  cntr_2_cntr_2_RSTF_INT_D : X_XOR2
    port map (
      I0 => NlwBufferSignal_cntr_2_cntr_2_RSTF_INT_D_IN0,
      I1 => NlwBufferSignal_cntr_2_cntr_2_RSTF_INT_D_IN1,
      O => cntr_2_cntr_2_RSTF_INT_D_313
    );
  cntr_2_cntr_2_RSTF_INT_D1 : X_ZERO
    port map (
      O => cntr_2_cntr_2_RSTF_INT_D1_314
    );
  cntr_2_cntr_2_RSTF_INT_D2 : X_AND2
    port map (
      I0 => NlwBufferSignal_cntr_2_cntr_2_RSTF_INT_D2_IN0,
      I1 => NlwBufferSignal_cntr_2_cntr_2_RSTF_INT_D2_IN1,
      O => cntr_2_cntr_2_RSTF_INT_D2_315
    );
  NlwBufferBlock_DSACK0_OBUF_D_IN0 : X_BUF
    port map (
      I => DSACK0_OBUF_D1_96,
      O => NlwBufferSignal_DSACK0_OBUF_D_IN0
    );
  NlwBufferBlock_DSACK0_OBUF_D_IN1 : X_BUF
    port map (
      I => DSACK0_OBUF_D2_97,
      O => NlwBufferSignal_DSACK0_OBUF_D_IN1
    );
  NlwBufferBlock_DSACK0_OBUF_D2_PT_0_IN0 : X_BUF
    port map (
      I => cntr(0),
      O => NlwBufferSignal_DSACK0_OBUF_D2_PT_0_IN0
    );
  NlwBufferBlock_DSACK0_OBUF_D2_PT_0_IN1 : X_BUF
    port map (
      I => cntr(2),
      O => NlwBufferSignal_DSACK0_OBUF_D2_PT_0_IN1
    );
  NlwBufferBlock_DSACK0_OBUF_D2_PT_0_IN2 : X_BUF
    port map (
      I => cntr(3),
      O => NlwBufferSignal_DSACK0_OBUF_D2_PT_0_IN2
    );
  NlwBufferBlock_DSACK0_OBUF_D2_PT_1_IN0 : X_BUF
    port map (
      I => cntr(2),
      O => NlwBufferSignal_DSACK0_OBUF_D2_PT_1_IN0
    );
  NlwBufferBlock_DSACK0_OBUF_D2_PT_1_IN1 : X_BUF
    port map (
      I => cntr(3),
      O => NlwBufferSignal_DSACK0_OBUF_D2_PT_1_IN1
    );
  NlwBufferBlock_DSACK0_OBUF_D2_PT_1_IN2 : X_BUF
    port map (
      I => cntr(1),
      O => NlwBufferSignal_DSACK0_OBUF_D2_PT_1_IN2
    );
  NlwBufferBlock_DSACK0_OBUF_D2_IN0 : X_BUF
    port map (
      I => DSACK0_OBUF_D2_PT_0_101,
      O => NlwBufferSignal_DSACK0_OBUF_D2_IN0
    );
  NlwBufferBlock_DSACK0_OBUF_D2_IN1 : X_BUF
    port map (
      I => DSACK0_OBUF_D2_PT_1_103,
      O => NlwBufferSignal_DSACK0_OBUF_D2_IN1
    );
  NlwBufferBlock_cntr_0_tsimcreated_xor_IN0 : X_BUF
    port map (
      I => cntr_0_D_105,
      O => NlwBufferSignal_cntr_0_tsimcreated_xor_IN0
    );
  NlwBufferBlock_cntr_0_tsimcreated_xor_IN1 : X_BUF
    port map (
      I => cntr_0_Q_104,
      O => NlwBufferSignal_cntr_0_tsimcreated_xor_IN1
    );
  NlwBufferBlock_cntr_0_tsimcreated_prld_IN0 : X_BUF
    port map (
      I => cntr_0_RSTF_107,
      O => NlwBufferSignal_cntr_0_tsimcreated_prld_IN0
    );
  NlwBufferBlock_cntr_0_tsimcreated_prld_IN1 : X_BUF
    port map (
      I => Gnd_108,
      O => NlwBufferSignal_cntr_0_tsimcreated_prld_IN1
    );
  NlwBufferBlock_cntr_0_REG_IN : X_BUF
    port map (
      I => cntr_0_tsimcreated_xor_Q_106,
      O => NlwBufferSignal_cntr_0_REG_IN
    );
  NlwBufferBlock_cntr_0_REG_CLK : X_BUF
    port map (
      I => FCLKIO_0_7,
      O => NlwBufferSignal_cntr_0_REG_CLK
    );
  NlwBufferBlock_cntr_0_D_IN0 : X_BUF
    port map (
      I => cntr_0_D1_111,
      O => NlwBufferSignal_cntr_0_D_IN0
    );
  NlwBufferBlock_cntr_0_D_IN1 : X_BUF
    port map (
      I => cntr_0_D2_112,
      O => NlwBufferSignal_cntr_0_D_IN1
    );
  NlwBufferBlock_cntr_0_RSTF_IN0 : X_BUF
    port map (
      I => cntr_2_cntr_2_RSTF_INT_UIM_113,
      O => NlwBufferSignal_cntr_0_RSTF_IN0
    );
  NlwBufferBlock_cntr_0_RSTF_IN1 : X_BUF
    port map (
      I => cntr_2_cntr_2_RSTF_INT_UIM_113,
      O => NlwBufferSignal_cntr_0_RSTF_IN1
    );
  NlwBufferBlock_cntr_1_tsimcreated_xor_IN0 : X_BUF
    port map (
      I => cntr_1_D_115,
      O => NlwBufferSignal_cntr_1_tsimcreated_xor_IN0
    );
  NlwBufferBlock_cntr_1_tsimcreated_xor_IN1 : X_BUF
    port map (
      I => cntr_1_Q_114,
      O => NlwBufferSignal_cntr_1_tsimcreated_xor_IN1
    );
  NlwBufferBlock_cntr_1_tsimcreated_prld_IN0 : X_BUF
    port map (
      I => cntr_1_RSTF_117,
      O => NlwBufferSignal_cntr_1_tsimcreated_prld_IN0
    );
  NlwBufferBlock_cntr_1_tsimcreated_prld_IN1 : X_BUF
    port map (
      I => Gnd_108,
      O => NlwBufferSignal_cntr_1_tsimcreated_prld_IN1
    );
  NlwBufferBlock_cntr_1_REG_IN : X_BUF
    port map (
      I => cntr_1_tsimcreated_xor_Q_116,
      O => NlwBufferSignal_cntr_1_REG_IN
    );
  NlwBufferBlock_cntr_1_REG_CLK : X_BUF
    port map (
      I => FCLKIO_0_7,
      O => NlwBufferSignal_cntr_1_REG_CLK
    );
  NlwBufferBlock_cntr_1_D_IN0 : X_BUF
    port map (
      I => cntr_1_D1_119,
      O => NlwBufferSignal_cntr_1_D_IN0
    );
  NlwBufferBlock_cntr_1_D_IN1 : X_BUF
    port map (
      I => cntr_1_D2_120,
      O => NlwBufferSignal_cntr_1_D_IN1
    );
  NlwBufferBlock_cntr_1_D2_IN0 : X_BUF
    port map (
      I => cntr(0),
      O => NlwBufferSignal_cntr_1_D2_IN0
    );
  NlwBufferBlock_cntr_1_D2_IN1 : X_BUF
    port map (
      I => cntr(0),
      O => NlwBufferSignal_cntr_1_D2_IN1
    );
  NlwBufferBlock_cntr_1_RSTF_IN0 : X_BUF
    port map (
      I => cntr_2_cntr_2_RSTF_INT_UIM_113,
      O => NlwBufferSignal_cntr_1_RSTF_IN0
    );
  NlwBufferBlock_cntr_1_RSTF_IN1 : X_BUF
    port map (
      I => cntr_2_cntr_2_RSTF_INT_UIM_113,
      O => NlwBufferSignal_cntr_1_RSTF_IN1
    );
  NlwBufferBlock_cntr_2_tsimcreated_xor_IN0 : X_BUF
    port map (
      I => cntr_2_D_122,
      O => NlwBufferSignal_cntr_2_tsimcreated_xor_IN0
    );
  NlwBufferBlock_cntr_2_tsimcreated_xor_IN1 : X_BUF
    port map (
      I => cntr_2_Q_121,
      O => NlwBufferSignal_cntr_2_tsimcreated_xor_IN1
    );
  NlwBufferBlock_cntr_2_tsimcreated_prld_IN0 : X_BUF
    port map (
      I => cntr_2_RSTF_124,
      O => NlwBufferSignal_cntr_2_tsimcreated_prld_IN0
    );
  NlwBufferBlock_cntr_2_tsimcreated_prld_IN1 : X_BUF
    port map (
      I => Gnd_108,
      O => NlwBufferSignal_cntr_2_tsimcreated_prld_IN1
    );
  NlwBufferBlock_cntr_2_REG_IN : X_BUF
    port map (
      I => cntr_2_tsimcreated_xor_Q_123,
      O => NlwBufferSignal_cntr_2_REG_IN
    );
  NlwBufferBlock_cntr_2_REG_CLK : X_BUF
    port map (
      I => FCLKIO_0_7,
      O => NlwBufferSignal_cntr_2_REG_CLK
    );
  NlwBufferBlock_cntr_2_D_IN0 : X_BUF
    port map (
      I => cntr_2_D1_126,
      O => NlwBufferSignal_cntr_2_D_IN0
    );
  NlwBufferBlock_cntr_2_D_IN1 : X_BUF
    port map (
      I => cntr_2_D2_127,
      O => NlwBufferSignal_cntr_2_D_IN1
    );
  NlwBufferBlock_cntr_2_D2_IN0 : X_BUF
    port map (
      I => cntr(0),
      O => NlwBufferSignal_cntr_2_D2_IN0
    );
  NlwBufferBlock_cntr_2_D2_IN1 : X_BUF
    port map (
      I => cntr(1),
      O => NlwBufferSignal_cntr_2_D2_IN1
    );
  NlwBufferBlock_cntr_2_RSTF_IN0 : X_BUF
    port map (
      I => cntr_2_cntr_2_RSTF_INT_UIM_113,
      O => NlwBufferSignal_cntr_2_RSTF_IN0
    );
  NlwBufferBlock_cntr_2_RSTF_IN1 : X_BUF
    port map (
      I => cntr_2_cntr_2_RSTF_INT_UIM_113,
      O => NlwBufferSignal_cntr_2_RSTF_IN1
    );
  NlwBufferBlock_cntr_3_tsimcreated_xor_IN0 : X_BUF
    port map (
      I => cntr_3_D_129,
      O => NlwBufferSignal_cntr_3_tsimcreated_xor_IN0
    );
  NlwBufferBlock_cntr_3_tsimcreated_xor_IN1 : X_BUF
    port map (
      I => cntr_3_Q_128,
      O => NlwBufferSignal_cntr_3_tsimcreated_xor_IN1
    );
  NlwBufferBlock_cntr_3_tsimcreated_prld_IN0 : X_BUF
    port map (
      I => cntr_3_RSTF_131,
      O => NlwBufferSignal_cntr_3_tsimcreated_prld_IN0
    );
  NlwBufferBlock_cntr_3_tsimcreated_prld_IN1 : X_BUF
    port map (
      I => Gnd_108,
      O => NlwBufferSignal_cntr_3_tsimcreated_prld_IN1
    );
  NlwBufferBlock_cntr_3_REG_IN : X_BUF
    port map (
      I => cntr_3_tsimcreated_xor_Q_130,
      O => NlwBufferSignal_cntr_3_REG_IN
    );
  NlwBufferBlock_cntr_3_REG_CLK : X_BUF
    port map (
      I => FCLKIO_0_7,
      O => NlwBufferSignal_cntr_3_REG_CLK
    );
  NlwBufferBlock_cntr_3_D_IN0 : X_BUF
    port map (
      I => cntr_3_D1_133,
      O => NlwBufferSignal_cntr_3_D_IN0
    );
  NlwBufferBlock_cntr_3_D_IN1 : X_BUF
    port map (
      I => cntr_3_D2_134,
      O => NlwBufferSignal_cntr_3_D_IN1
    );
  NlwBufferBlock_cntr_3_D2_IN0 : X_BUF
    port map (
      I => cntr(0),
      O => NlwBufferSignal_cntr_3_D2_IN0
    );
  NlwBufferBlock_cntr_3_D2_IN1 : X_BUF
    port map (
      I => cntr(2),
      O => NlwBufferSignal_cntr_3_D2_IN1
    );
  NlwBufferBlock_cntr_3_D2_IN2 : X_BUF
    port map (
      I => cntr(1),
      O => NlwBufferSignal_cntr_3_D2_IN2
    );
  NlwBufferBlock_cntr_3_RSTF_IN0 : X_BUF
    port map (
      I => cntr_2_cntr_2_RSTF_INT_UIM_113,
      O => NlwBufferSignal_cntr_3_RSTF_IN0
    );
  NlwBufferBlock_cntr_3_RSTF_IN1 : X_BUF
    port map (
      I => cntr_2_cntr_2_RSTF_INT_UIM_113,
      O => NlwBufferSignal_cntr_3_RSTF_IN1
    );
  NlwBufferBlock_BYTE0_OBUF_D_IN0 : X_BUF
    port map (
      I => BYTE0_OBUF_D1_137,
      O => NlwBufferSignal_BYTE0_OBUF_D_IN0
    );
  NlwBufferBlock_BYTE0_OBUF_D_IN1 : X_BUF
    port map (
      I => BYTE0_OBUF_D2_138,
      O => NlwBufferSignal_BYTE0_OBUF_D_IN1
    );
  NlwBufferBlock_BYTE0_OBUF_D2_PT_0_IN0 : X_BUF
    port map (
      I => SIZ0_IBUF_9,
      O => NlwBufferSignal_BYTE0_OBUF_D2_PT_0_IN0
    );
  NlwBufferBlock_BYTE0_OBUF_D2_PT_0_IN1 : X_BUF
    port map (
      I => a0_IBUF_11,
      O => NlwBufferSignal_BYTE0_OBUF_D2_PT_0_IN1
    );
  NlwBufferBlock_BYTE0_OBUF_D2_PT_0_IN2 : X_BUF
    port map (
      I => SIZ1_IBUF_13,
      O => NlwBufferSignal_BYTE0_OBUF_D2_PT_0_IN2
    );
  NlwBufferBlock_BYTE0_OBUF_D2_PT_1_IN0 : X_BUF
    port map (
      I => SIZ0_IBUF_9,
      O => NlwBufferSignal_BYTE0_OBUF_D2_PT_1_IN0
    );
  NlwBufferBlock_BYTE0_OBUF_D2_PT_1_IN1 : X_BUF
    port map (
      I => a0_IBUF_11,
      O => NlwBufferSignal_BYTE0_OBUF_D2_PT_1_IN1
    );
  NlwBufferBlock_BYTE0_OBUF_D2_PT_1_IN2 : X_BUF
    port map (
      I => a1_IBUF_31,
      O => NlwBufferSignal_BYTE0_OBUF_D2_PT_1_IN2
    );
  NlwBufferBlock_BYTE0_OBUF_D2_PT_2_IN0 : X_BUF
    port map (
      I => SIZ0_IBUF_9,
      O => NlwBufferSignal_BYTE0_OBUF_D2_PT_2_IN0
    );
  NlwBufferBlock_BYTE0_OBUF_D2_PT_2_IN1 : X_BUF
    port map (
      I => SIZ1_IBUF_13,
      O => NlwBufferSignal_BYTE0_OBUF_D2_PT_2_IN1
    );
  NlwBufferBlock_BYTE0_OBUF_D2_PT_2_IN2 : X_BUF
    port map (
      I => a1_IBUF_31,
      O => NlwBufferSignal_BYTE0_OBUF_D2_PT_2_IN2
    );
  NlwBufferBlock_BYTE0_OBUF_D2_PT_3_IN0 : X_BUF
    port map (
      I => SIZ0_IBUF_9,
      O => NlwBufferSignal_BYTE0_OBUF_D2_PT_3_IN0
    );
  NlwBufferBlock_BYTE0_OBUF_D2_PT_3_IN1 : X_BUF
    port map (
      I => SIZ1_IBUF_13,
      O => NlwBufferSignal_BYTE0_OBUF_D2_PT_3_IN1
    );
  NlwBufferBlock_BYTE0_OBUF_D2_PT_3_IN2 : X_BUF
    port map (
      I => a1_IBUF_31,
      O => NlwBufferSignal_BYTE0_OBUF_D2_PT_3_IN2
    );
  NlwBufferBlock_BYTE0_OBUF_D2_IN0 : X_BUF
    port map (
      I => BYTE0_OBUF_D2_PT_0_139,
      O => NlwBufferSignal_BYTE0_OBUF_D2_IN0
    );
  NlwBufferBlock_BYTE0_OBUF_D2_IN1 : X_BUF
    port map (
      I => BYTE0_OBUF_D2_PT_1_140,
      O => NlwBufferSignal_BYTE0_OBUF_D2_IN1
    );
  NlwBufferBlock_BYTE0_OBUF_D2_IN2 : X_BUF
    port map (
      I => BYTE0_OBUF_D2_PT_2_141,
      O => NlwBufferSignal_BYTE0_OBUF_D2_IN2
    );
  NlwBufferBlock_BYTE0_OBUF_D2_IN3 : X_BUF
    port map (
      I => BYTE0_OBUF_D2_PT_3_142,
      O => NlwBufferSignal_BYTE0_OBUF_D2_IN3
    );
  NlwBufferBlock_BYTE1_OBUF_D_IN0 : X_BUF
    port map (
      I => BYTE1_OBUF_D1_145,
      O => NlwBufferSignal_BYTE1_OBUF_D_IN0
    );
  NlwBufferBlock_BYTE1_OBUF_D_IN1 : X_BUF
    port map (
      I => BYTE1_OBUF_D2_146,
      O => NlwBufferSignal_BYTE1_OBUF_D_IN1
    );
  NlwBufferBlock_BYTE1_OBUF_D2_PT_0_IN0 : X_BUF
    port map (
      I => a0_IBUF_11,
      O => NlwBufferSignal_BYTE1_OBUF_D2_PT_0_IN0
    );
  NlwBufferBlock_BYTE1_OBUF_D2_PT_0_IN1 : X_BUF
    port map (
      I => a1_IBUF_31,
      O => NlwBufferSignal_BYTE1_OBUF_D2_PT_0_IN1
    );
  NlwBufferBlock_BYTE1_OBUF_D2_PT_1_IN0 : X_BUF
    port map (
      I => SIZ0_IBUF_9,
      O => NlwBufferSignal_BYTE1_OBUF_D2_PT_1_IN0
    );
  NlwBufferBlock_BYTE1_OBUF_D2_PT_1_IN1 : X_BUF
    port map (
      I => SIZ1_IBUF_13,
      O => NlwBufferSignal_BYTE1_OBUF_D2_PT_1_IN1
    );
  NlwBufferBlock_BYTE1_OBUF_D2_PT_1_IN2 : X_BUF
    port map (
      I => a1_IBUF_31,
      O => NlwBufferSignal_BYTE1_OBUF_D2_PT_1_IN2
    );
  NlwBufferBlock_BYTE1_OBUF_D2_PT_2_IN0 : X_BUF
    port map (
      I => SIZ0_IBUF_9,
      O => NlwBufferSignal_BYTE1_OBUF_D2_PT_2_IN0
    );
  NlwBufferBlock_BYTE1_OBUF_D2_PT_2_IN1 : X_BUF
    port map (
      I => a0_IBUF_11,
      O => NlwBufferSignal_BYTE1_OBUF_D2_PT_2_IN1
    );
  NlwBufferBlock_BYTE1_OBUF_D2_PT_2_IN2 : X_BUF
    port map (
      I => SIZ1_IBUF_13,
      O => NlwBufferSignal_BYTE1_OBUF_D2_PT_2_IN2
    );
  NlwBufferBlock_BYTE1_OBUF_D2_PT_2_IN3 : X_BUF
    port map (
      I => a1_IBUF_31,
      O => NlwBufferSignal_BYTE1_OBUF_D2_PT_2_IN3
    );
  NlwBufferBlock_BYTE1_OBUF_D2_IN0 : X_BUF
    port map (
      I => BYTE1_OBUF_D2_PT_0_147,
      O => NlwBufferSignal_BYTE1_OBUF_D2_IN0
    );
  NlwBufferBlock_BYTE1_OBUF_D2_IN1 : X_BUF
    port map (
      I => BYTE1_OBUF_D2_PT_1_148,
      O => NlwBufferSignal_BYTE1_OBUF_D2_IN1
    );
  NlwBufferBlock_BYTE1_OBUF_D2_IN2 : X_BUF
    port map (
      I => BYTE1_OBUF_D2_PT_2_149,
      O => NlwBufferSignal_BYTE1_OBUF_D2_IN2
    );
  NlwBufferBlock_BYTE2_OBUF_D_IN0 : X_BUF
    port map (
      I => BYTE2_OBUF_D1_152,
      O => NlwBufferSignal_BYTE2_OBUF_D_IN0
    );
  NlwBufferBlock_BYTE2_OBUF_D_IN1 : X_BUF
    port map (
      I => BYTE2_OBUF_D2_153,
      O => NlwBufferSignal_BYTE2_OBUF_D_IN1
    );
  NlwBufferBlock_BYTE2_OBUF_D2_PT_0_IN0 : X_BUF
    port map (
      I => a1_IBUF_31,
      O => NlwBufferSignal_BYTE2_OBUF_D2_PT_0_IN0
    );
  NlwBufferBlock_BYTE2_OBUF_D2_PT_0_IN1 : X_BUF
    port map (
      I => a1_IBUF_31,
      O => NlwBufferSignal_BYTE2_OBUF_D2_PT_0_IN1
    );
  NlwBufferBlock_BYTE2_OBUF_D2_PT_1_IN0 : X_BUF
    port map (
      I => SIZ0_IBUF_9,
      O => NlwBufferSignal_BYTE2_OBUF_D2_PT_1_IN0
    );
  NlwBufferBlock_BYTE2_OBUF_D2_PT_1_IN1 : X_BUF
    port map (
      I => a0_IBUF_11,
      O => NlwBufferSignal_BYTE2_OBUF_D2_PT_1_IN1
    );
  NlwBufferBlock_BYTE2_OBUF_D2_PT_1_IN2 : X_BUF
    port map (
      I => SIZ1_IBUF_13,
      O => NlwBufferSignal_BYTE2_OBUF_D2_PT_1_IN2
    );
  NlwBufferBlock_BYTE2_OBUF_D2_IN0 : X_BUF
    port map (
      I => BYTE2_OBUF_D2_PT_0_154,
      O => NlwBufferSignal_BYTE2_OBUF_D2_IN0
    );
  NlwBufferBlock_BYTE2_OBUF_D2_IN1 : X_BUF
    port map (
      I => BYTE2_OBUF_D2_PT_1_155,
      O => NlwBufferSignal_BYTE2_OBUF_D2_IN1
    );
  NlwBufferBlock_BYTE3_OBUF_D_IN0 : X_BUF
    port map (
      I => BYTE3_OBUF_D1_158,
      O => NlwBufferSignal_BYTE3_OBUF_D_IN0
    );
  NlwBufferBlock_BYTE3_OBUF_D_IN1 : X_BUF
    port map (
      I => BYTE3_OBUF_D2_159,
      O => NlwBufferSignal_BYTE3_OBUF_D_IN1
    );
  NlwBufferBlock_BYTE3_OBUF_D2_IN0 : X_BUF
    port map (
      I => a0_IBUF_11,
      O => NlwBufferSignal_BYTE3_OBUF_D2_IN0
    );
  NlwBufferBlock_BYTE3_OBUF_D2_IN1 : X_BUF
    port map (
      I => a1_IBUF_31,
      O => NlwBufferSignal_BYTE3_OBUF_D2_IN1
    );
  NlwBufferBlock_DRAM_CS_OBUF_D_IN0 : X_BUF
    port map (
      I => DRAM_CS_OBUF_D1_162,
      O => NlwBufferSignal_DRAM_CS_OBUF_D_IN0
    );
  NlwBufferBlock_DRAM_CS_OBUF_D_IN1 : X_BUF
    port map (
      I => DRAM_CS_OBUF_D2_163,
      O => NlwBufferSignal_DRAM_CS_OBUF_D_IN1
    );
  NlwBufferBlock_DRAM_CS_OBUF_D2_PT_0_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_DRAM_CS_OBUF_D2_PT_0_IN0
    );
  NlwBufferBlock_DRAM_CS_OBUF_D2_PT_0_IN1 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_DRAM_CS_OBUF_D2_PT_0_IN1
    );
  NlwBufferBlock_DRAM_CS_OBUF_D2_PT_1_IN0 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_DRAM_CS_OBUF_D2_PT_1_IN0
    );
  NlwBufferBlock_DRAM_CS_OBUF_D2_PT_1_IN1 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_DRAM_CS_OBUF_D2_PT_1_IN1
    );
  NlwBufferBlock_DRAM_CS_OBUF_D2_PT_2_IN0 : X_BUF
    port map (
      I => FC2_IBUF_19,
      O => NlwBufferSignal_DRAM_CS_OBUF_D2_PT_2_IN0
    );
  NlwBufferBlock_DRAM_CS_OBUF_D2_PT_2_IN1 : X_BUF
    port map (
      I => FC1_IBUF_21,
      O => NlwBufferSignal_DRAM_CS_OBUF_D2_PT_2_IN1
    );
  NlwBufferBlock_DRAM_CS_OBUF_D2_PT_2_IN2 : X_BUF
    port map (
      I => FC0_IBUF_23,
      O => NlwBufferSignal_DRAM_CS_OBUF_D2_PT_2_IN2
    );
  NlwBufferBlock_DRAM_CS_OBUF_D2_IN0 : X_BUF
    port map (
      I => DRAM_CS_OBUF_D2_PT_0_164,
      O => NlwBufferSignal_DRAM_CS_OBUF_D2_IN0
    );
  NlwBufferBlock_DRAM_CS_OBUF_D2_IN1 : X_BUF
    port map (
      I => DRAM_CS_OBUF_D2_PT_1_165,
      O => NlwBufferSignal_DRAM_CS_OBUF_D2_IN1
    );
  NlwBufferBlock_DRAM_CS_OBUF_D2_IN2 : X_BUF
    port map (
      I => DRAM_CS_OBUF_D2_PT_2_166,
      O => NlwBufferSignal_DRAM_CS_OBUF_D2_IN2
    );
  NlwBufferBlock_DSACK1_OBUF_D_IN0 : X_BUF
    port map (
      I => DSACK1_OBUF_D1_169,
      O => NlwBufferSignal_DSACK1_OBUF_D_IN0
    );
  NlwBufferBlock_DSACK1_OBUF_D_IN1 : X_BUF
    port map (
      I => DSACK1_OBUF_D2_170,
      O => NlwBufferSignal_DSACK1_OBUF_D_IN1
    );
  NlwBufferBlock_DSACK1_OBUF_D2_PT_0_IN0 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_DSACK1_OBUF_D2_PT_0_IN0
    );
  NlwBufferBlock_DSACK1_OBUF_D2_PT_0_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_DSACK1_OBUF_D2_PT_0_IN1
    );
  NlwBufferBlock_DSACK1_OBUF_D2_PT_1_IN0 : X_BUF
    port map (
      I => cntr(0),
      O => NlwBufferSignal_DSACK1_OBUF_D2_PT_1_IN0
    );
  NlwBufferBlock_DSACK1_OBUF_D2_PT_1_IN1 : X_BUF
    port map (
      I => cntr(2),
      O => NlwBufferSignal_DSACK1_OBUF_D2_PT_1_IN1
    );
  NlwBufferBlock_DSACK1_OBUF_D2_PT_1_IN2 : X_BUF
    port map (
      I => cntr(3),
      O => NlwBufferSignal_DSACK1_OBUF_D2_PT_1_IN2
    );
  NlwBufferBlock_DSACK1_OBUF_D2_PT_2_IN0 : X_BUF
    port map (
      I => cntr(2),
      O => NlwBufferSignal_DSACK1_OBUF_D2_PT_2_IN0
    );
  NlwBufferBlock_DSACK1_OBUF_D2_PT_2_IN1 : X_BUF
    port map (
      I => cntr(3),
      O => NlwBufferSignal_DSACK1_OBUF_D2_PT_2_IN1
    );
  NlwBufferBlock_DSACK1_OBUF_D2_PT_2_IN2 : X_BUF
    port map (
      I => cntr(1),
      O => NlwBufferSignal_DSACK1_OBUF_D2_PT_2_IN2
    );
  NlwBufferBlock_DSACK1_OBUF_D2_IN0 : X_BUF
    port map (
      I => DSACK1_OBUF_D2_PT_0_171,
      O => NlwBufferSignal_DSACK1_OBUF_D2_IN0
    );
  NlwBufferBlock_DSACK1_OBUF_D2_IN1 : X_BUF
    port map (
      I => DSACK1_OBUF_D2_PT_1_172,
      O => NlwBufferSignal_DSACK1_OBUF_D2_IN1
    );
  NlwBufferBlock_DSACK1_OBUF_D2_IN2 : X_BUF
    port map (
      I => DSACK1_OBUF_D2_PT_2_173,
      O => NlwBufferSignal_DSACK1_OBUF_D2_IN2
    );
  NlwBufferBlock_ETHRNT_OBUF_D_IN0 : X_BUF
    port map (
      I => ETHRNT_OBUF_D1_176,
      O => NlwBufferSignal_ETHRNT_OBUF_D_IN0
    );
  NlwBufferBlock_ETHRNT_OBUF_D_IN1 : X_BUF
    port map (
      I => ETHRNT_OBUF_D2_177,
      O => NlwBufferSignal_ETHRNT_OBUF_D_IN1
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_0_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN0
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_0_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN1
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_0_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN2
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_0_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN3
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_0_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN4
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_0_IN5 : X_BUF
    port map (
      I => FC2_IBUF_19,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN5
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_0_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN6
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_1_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN0
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_1_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN1
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_1_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN2
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_1_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN3
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_1_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN4
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_1_IN5 : X_BUF
    port map (
      I => FC1_IBUF_21,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN5
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_1_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN6
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_2_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN0
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_2_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN1
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_2_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN2
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_2_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN3
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_2_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN4
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_2_IN5 : X_BUF
    port map (
      I => FC0_IBUF_23,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN5
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_PT_2_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN6
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_IN0 : X_BUF
    port map (
      I => ETHRNT_OBUF_D2_PT_0_178,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_IN0
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_IN1 : X_BUF
    port map (
      I => ETHRNT_OBUF_D2_PT_1_179,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_IN1
    );
  NlwBufferBlock_ETHRNT_OBUF_D2_IN2 : X_BUF
    port map (
      I => ETHRNT_OBUF_D2_PT_2_180,
      O => NlwBufferSignal_ETHRNT_OBUF_D2_IN2
    );
  NlwBufferBlock_FLOPPY_OBUF_D_IN0 : X_BUF
    port map (
      I => FLOPPY_OBUF_D1_183,
      O => NlwBufferSignal_FLOPPY_OBUF_D_IN0
    );
  NlwBufferBlock_FLOPPY_OBUF_D_IN1 : X_BUF
    port map (
      I => FLOPPY_OBUF_D2_184,
      O => NlwBufferSignal_FLOPPY_OBUF_D_IN1
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_0_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN0
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_0_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN1
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_0_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN2
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_0_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN3
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_0_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN4
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_0_IN5 : X_BUF
    port map (
      I => FC2_IBUF_19,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN5
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_0_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN6
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_1_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN0
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_1_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN1
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_1_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN2
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_1_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN3
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_1_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN4
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_1_IN5 : X_BUF
    port map (
      I => FC1_IBUF_21,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN5
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_1_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN6
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_2_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN0
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_2_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN1
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_2_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN2
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_2_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN3
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_2_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN4
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_2_IN5 : X_BUF
    port map (
      I => FC0_IBUF_23,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN5
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_PT_2_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN6
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_IN0 : X_BUF
    port map (
      I => FLOPPY_OBUF_D2_PT_0_185,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_IN0
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_IN1 : X_BUF
    port map (
      I => FLOPPY_OBUF_D2_PT_1_186,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_IN1
    );
  NlwBufferBlock_FLOPPY_OBUF_D2_IN2 : X_BUF
    port map (
      I => FLOPPY_OBUF_D2_PT_2_187,
      O => NlwBufferSignal_FLOPPY_OBUF_D2_IN2
    );
  NlwBufferBlock_IDE_OBUF_D_IN0 : X_BUF
    port map (
      I => IDE_OBUF_D1_190,
      O => NlwBufferSignal_IDE_OBUF_D_IN0
    );
  NlwBufferBlock_IDE_OBUF_D_IN1 : X_BUF
    port map (
      I => IDE_OBUF_D2_191,
      O => NlwBufferSignal_IDE_OBUF_D_IN1
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_0_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_0_IN0
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_0_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_0_IN1
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_0_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_0_IN2
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_0_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_0_IN3
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_0_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_0_IN4
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_0_IN5 : X_BUF
    port map (
      I => FC2_IBUF_19,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_0_IN5
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_0_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_0_IN6
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_1_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_1_IN0
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_1_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_1_IN1
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_1_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_1_IN2
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_1_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_1_IN3
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_1_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_1_IN4
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_1_IN5 : X_BUF
    port map (
      I => FC1_IBUF_21,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_1_IN5
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_1_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_1_IN6
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_2_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_2_IN0
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_2_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_2_IN1
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_2_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_2_IN2
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_2_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_2_IN3
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_2_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_2_IN4
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_2_IN5 : X_BUF
    port map (
      I => FC0_IBUF_23,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_2_IN5
    );
  NlwBufferBlock_IDE_OBUF_D2_PT_2_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_IDE_OBUF_D2_PT_2_IN6
    );
  NlwBufferBlock_IDE_OBUF_D2_IN0 : X_BUF
    port map (
      I => IDE_OBUF_D2_PT_0_192,
      O => NlwBufferSignal_IDE_OBUF_D2_IN0
    );
  NlwBufferBlock_IDE_OBUF_D2_IN1 : X_BUF
    port map (
      I => IDE_OBUF_D2_PT_1_193,
      O => NlwBufferSignal_IDE_OBUF_D2_IN1
    );
  NlwBufferBlock_IDE_OBUF_D2_IN2 : X_BUF
    port map (
      I => IDE_OBUF_D2_PT_2_194,
      O => NlwBufferSignal_IDE_OBUF_D2_IN2
    );
  NlwBufferBlock_IO_CS_OBUF_D_IN0 : X_BUF
    port map (
      I => IO_CS_OBUF_D1_197,
      O => NlwBufferSignal_IO_CS_OBUF_D_IN0
    );
  NlwBufferBlock_IO_CS_OBUF_D_IN1 : X_BUF
    port map (
      I => IO_CS_OBUF_D2_198,
      O => NlwBufferSignal_IO_CS_OBUF_D_IN1
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_0_IN0 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF1_EXP_199,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_0_IN0
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_0_IN1 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF1_EXP_199,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_0_IN1
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_1_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN0
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_1_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN1
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_1_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN2
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_1_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN3
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_1_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN4
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_1_IN5 : X_BUF
    port map (
      I => FC1_IBUF_21,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN5
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_1_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN6
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_2_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN0
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_2_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN1
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_2_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN2
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_2_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN3
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_2_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN4
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_2_IN5 : X_BUF
    port map (
      I => FC0_IBUF_23,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN5
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_2_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN6
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_3_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN0
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_3_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN1
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_3_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN2
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_3_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN3
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_3_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN4
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_3_IN5 : X_BUF
    port map (
      I => FC2_IBUF_19,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN5
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_3_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN6
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_4_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN0
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_4_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN1
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_4_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN2
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_4_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN3
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_4_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN4
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_4_IN5 : X_BUF
    port map (
      I => FC1_IBUF_21,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN5
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_4_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN6
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_5_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN0
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_5_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN1
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_5_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN2
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_5_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN3
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_5_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN4
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_5_IN5 : X_BUF
    port map (
      I => FC0_IBUF_23,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN5
    );
  NlwBufferBlock_IO_CS_OBUF_D2_PT_5_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN6
    );
  NlwBufferBlock_IO_CS_OBUF_D2_IN0 : X_BUF
    port map (
      I => IO_CS_OBUF_D2_PT_0_200,
      O => NlwBufferSignal_IO_CS_OBUF_D2_IN0
    );
  NlwBufferBlock_IO_CS_OBUF_D2_IN1 : X_BUF
    port map (
      I => IO_CS_OBUF_D2_PT_1_201,
      O => NlwBufferSignal_IO_CS_OBUF_D2_IN1
    );
  NlwBufferBlock_IO_CS_OBUF_D2_IN2 : X_BUF
    port map (
      I => IO_CS_OBUF_D2_PT_2_202,
      O => NlwBufferSignal_IO_CS_OBUF_D2_IN2
    );
  NlwBufferBlock_IO_CS_OBUF_D2_IN3 : X_BUF
    port map (
      I => IO_CS_OBUF_D2_PT_3_203,
      O => NlwBufferSignal_IO_CS_OBUF_D2_IN3
    );
  NlwBufferBlock_IO_CS_OBUF_D2_IN4 : X_BUF
    port map (
      I => IO_CS_OBUF_D2_PT_4_204,
      O => NlwBufferSignal_IO_CS_OBUF_D2_IN4
    );
  NlwBufferBlock_IO_CS_OBUF_D2_IN5 : X_BUF
    port map (
      I => IO_CS_OBUF_D2_PT_5_205,
      O => NlwBufferSignal_IO_CS_OBUF_D2_IN5
    );
  NlwBufferBlock_KBD_CS_OBUF_D_IN0 : X_BUF
    port map (
      I => KBD_CS_OBUF_D1_208,
      O => NlwBufferSignal_KBD_CS_OBUF_D_IN0
    );
  NlwBufferBlock_KBD_CS_OBUF_D_IN1 : X_BUF
    port map (
      I => KBD_CS_OBUF_D2_209,
      O => NlwBufferSignal_KBD_CS_OBUF_D_IN1
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_0_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN0
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_0_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN1
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_0_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN2
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_0_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN3
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_0_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN4
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_0_IN5 : X_BUF
    port map (
      I => FC2_IBUF_19,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN5
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_0_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN6
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_1_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN0
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_1_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN1
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_1_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN2
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_1_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN3
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_1_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN4
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_1_IN5 : X_BUF
    port map (
      I => FC1_IBUF_21,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN5
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_1_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN6
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_2_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN0
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_2_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN1
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_2_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN2
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_2_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN3
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_2_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN4
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_2_IN5 : X_BUF
    port map (
      I => FC0_IBUF_23,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN5
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_PT_2_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN6
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_IN0 : X_BUF
    port map (
      I => KBD_CS_OBUF_D2_PT_0_210,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_IN0
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_IN1 : X_BUF
    port map (
      I => KBD_CS_OBUF_D2_PT_1_211,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_IN1
    );
  NlwBufferBlock_KBD_CS_OBUF_D2_IN2 : X_BUF
    port map (
      I => KBD_CS_OBUF_D2_PT_2_212,
      O => NlwBufferSignal_KBD_CS_OBUF_D2_IN2
    );
  NlwBufferBlock_ROM_OBUF_D_IN0 : X_BUF
    port map (
      I => ROM_OBUF_D1_215,
      O => NlwBufferSignal_ROM_OBUF_D_IN0
    );
  NlwBufferBlock_ROM_OBUF_D_IN1 : X_BUF
    port map (
      I => ROM_OBUF_D2_216,
      O => NlwBufferSignal_ROM_OBUF_D_IN1
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_0_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_0_IN0
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_0_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_0_IN1
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_0_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_0_IN2
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_0_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_0_IN3
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_0_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_0_IN4
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_0_IN5 : X_BUF
    port map (
      I => FC2_IBUF_19,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_0_IN5
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_0_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_0_IN6
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_1_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_1_IN0
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_1_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_1_IN1
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_1_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_1_IN2
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_1_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_1_IN3
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_1_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_1_IN4
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_1_IN5 : X_BUF
    port map (
      I => FC1_IBUF_21,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_1_IN5
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_1_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_1_IN6
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_2_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_2_IN0
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_2_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_2_IN1
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_2_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_2_IN2
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_2_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_2_IN3
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_2_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_2_IN4
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_2_IN5 : X_BUF
    port map (
      I => FC0_IBUF_23,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_2_IN5
    );
  NlwBufferBlock_ROM_OBUF_D2_PT_2_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_ROM_OBUF_D2_PT_2_IN6
    );
  NlwBufferBlock_ROM_OBUF_D2_IN0 : X_BUF
    port map (
      I => ROM_OBUF_D2_PT_0_217,
      O => NlwBufferSignal_ROM_OBUF_D2_IN0
    );
  NlwBufferBlock_ROM_OBUF_D2_IN1 : X_BUF
    port map (
      I => ROM_OBUF_D2_PT_1_218,
      O => NlwBufferSignal_ROM_OBUF_D2_IN1
    );
  NlwBufferBlock_ROM_OBUF_D2_IN2 : X_BUF
    port map (
      I => ROM_OBUF_D2_PT_2_219,
      O => NlwBufferSignal_ROM_OBUF_D2_IN2
    );
  NlwBufferBlock_RW0_OBUF_D_IN0 : X_BUF
    port map (
      I => RW0_OBUF_D1_222,
      O => NlwBufferSignal_RW0_OBUF_D_IN0
    );
  NlwBufferBlock_RW0_OBUF_D_IN1 : X_BUF
    port map (
      I => RW0_OBUF_D2_223,
      O => NlwBufferSignal_RW0_OBUF_D_IN1
    );
  NlwBufferBlock_RW0_OBUF_D2_PT_0_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_RW0_OBUF_D2_PT_0_IN0
    );
  NlwBufferBlock_RW0_OBUF_D2_PT_0_IN1 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_RW0_OBUF_D2_PT_0_IN1
    );
  NlwBufferBlock_RW0_OBUF_D2_PT_1_IN0 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_RW0_OBUF_D2_PT_1_IN0
    );
  NlwBufferBlock_RW0_OBUF_D2_PT_1_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_RW0_OBUF_D2_PT_1_IN1
    );
  NlwBufferBlock_RW0_OBUF_D2_PT_2_IN0 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_RW0_OBUF_D2_PT_2_IN0
    );
  NlwBufferBlock_RW0_OBUF_D2_PT_2_IN1 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_RW0_OBUF_D2_PT_2_IN1
    );
  NlwBufferBlock_RW0_OBUF_D2_PT_3_IN0 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_RW0_OBUF_D2_PT_3_IN0
    );
  NlwBufferBlock_RW0_OBUF_D2_PT_3_IN1 : X_BUF
    port map (
      I => RW_N_IBUF_33,
      O => NlwBufferSignal_RW0_OBUF_D2_PT_3_IN1
    );
  NlwBufferBlock_RW0_OBUF_D2_PT_4_IN0 : X_BUF
    port map (
      I => FC2_IBUF_19,
      O => NlwBufferSignal_RW0_OBUF_D2_PT_4_IN0
    );
  NlwBufferBlock_RW0_OBUF_D2_PT_4_IN1 : X_BUF
    port map (
      I => FC1_IBUF_21,
      O => NlwBufferSignal_RW0_OBUF_D2_PT_4_IN1
    );
  NlwBufferBlock_RW0_OBUF_D2_PT_4_IN2 : X_BUF
    port map (
      I => FC0_IBUF_23,
      O => NlwBufferSignal_RW0_OBUF_D2_PT_4_IN2
    );
  NlwBufferBlock_RW0_OBUF_D2_IN0 : X_BUF
    port map (
      I => RW0_OBUF_D2_PT_0_224,
      O => NlwBufferSignal_RW0_OBUF_D2_IN0
    );
  NlwBufferBlock_RW0_OBUF_D2_IN1 : X_BUF
    port map (
      I => RW0_OBUF_D2_PT_1_225,
      O => NlwBufferSignal_RW0_OBUF_D2_IN1
    );
  NlwBufferBlock_RW0_OBUF_D2_IN2 : X_BUF
    port map (
      I => RW0_OBUF_D2_PT_2_226,
      O => NlwBufferSignal_RW0_OBUF_D2_IN2
    );
  NlwBufferBlock_RW0_OBUF_D2_IN3 : X_BUF
    port map (
      I => RW0_OBUF_D2_PT_3_227,
      O => NlwBufferSignal_RW0_OBUF_D2_IN3
    );
  NlwBufferBlock_RW0_OBUF_D2_IN4 : X_BUF
    port map (
      I => RW0_OBUF_D2_PT_4_228,
      O => NlwBufferSignal_RW0_OBUF_D2_IN4
    );
  NlwBufferBlock_RW1_OBUF_D_IN0 : X_BUF
    port map (
      I => RW1_OBUF_D1_231,
      O => NlwBufferSignal_RW1_OBUF_D_IN0
    );
  NlwBufferBlock_RW1_OBUF_D_IN1 : X_BUF
    port map (
      I => RW1_OBUF_D2_232,
      O => NlwBufferSignal_RW1_OBUF_D_IN1
    );
  NlwBufferBlock_RW1_OBUF_D2_PT_0_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_RW1_OBUF_D2_PT_0_IN0
    );
  NlwBufferBlock_RW1_OBUF_D2_PT_0_IN1 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_RW1_OBUF_D2_PT_0_IN1
    );
  NlwBufferBlock_RW1_OBUF_D2_PT_1_IN0 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_RW1_OBUF_D2_PT_1_IN0
    );
  NlwBufferBlock_RW1_OBUF_D2_PT_1_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_RW1_OBUF_D2_PT_1_IN1
    );
  NlwBufferBlock_RW1_OBUF_D2_PT_2_IN0 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_RW1_OBUF_D2_PT_2_IN0
    );
  NlwBufferBlock_RW1_OBUF_D2_PT_2_IN1 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_RW1_OBUF_D2_PT_2_IN1
    );
  NlwBufferBlock_RW1_OBUF_D2_PT_3_IN0 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_RW1_OBUF_D2_PT_3_IN0
    );
  NlwBufferBlock_RW1_OBUF_D2_PT_3_IN1 : X_BUF
    port map (
      I => RW_N_IBUF_33,
      O => NlwBufferSignal_RW1_OBUF_D2_PT_3_IN1
    );
  NlwBufferBlock_RW1_OBUF_D2_PT_4_IN0 : X_BUF
    port map (
      I => FC2_IBUF_19,
      O => NlwBufferSignal_RW1_OBUF_D2_PT_4_IN0
    );
  NlwBufferBlock_RW1_OBUF_D2_PT_4_IN1 : X_BUF
    port map (
      I => FC1_IBUF_21,
      O => NlwBufferSignal_RW1_OBUF_D2_PT_4_IN1
    );
  NlwBufferBlock_RW1_OBUF_D2_PT_4_IN2 : X_BUF
    port map (
      I => FC0_IBUF_23,
      O => NlwBufferSignal_RW1_OBUF_D2_PT_4_IN2
    );
  NlwBufferBlock_RW1_OBUF_D2_IN0 : X_BUF
    port map (
      I => RW1_OBUF_D2_PT_0_233,
      O => NlwBufferSignal_RW1_OBUF_D2_IN0
    );
  NlwBufferBlock_RW1_OBUF_D2_IN1 : X_BUF
    port map (
      I => RW1_OBUF_D2_PT_1_234,
      O => NlwBufferSignal_RW1_OBUF_D2_IN1
    );
  NlwBufferBlock_RW1_OBUF_D2_IN2 : X_BUF
    port map (
      I => RW1_OBUF_D2_PT_2_235,
      O => NlwBufferSignal_RW1_OBUF_D2_IN2
    );
  NlwBufferBlock_RW1_OBUF_D2_IN3 : X_BUF
    port map (
      I => RW1_OBUF_D2_PT_3_236,
      O => NlwBufferSignal_RW1_OBUF_D2_IN3
    );
  NlwBufferBlock_RW1_OBUF_D2_IN4 : X_BUF
    port map (
      I => RW1_OBUF_D2_PT_4_237,
      O => NlwBufferSignal_RW1_OBUF_D2_IN4
    );
  NlwBufferBlock_VGA_CS_OBUF_D_IN0 : X_BUF
    port map (
      I => VGA_CS_OBUF_D1_240,
      O => NlwBufferSignal_VGA_CS_OBUF_D_IN0
    );
  NlwBufferBlock_VGA_CS_OBUF_D_IN1 : X_BUF
    port map (
      I => VGA_CS_OBUF_D2_241,
      O => NlwBufferSignal_VGA_CS_OBUF_D_IN1
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_0_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN0
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_0_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN1
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_0_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN2
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_0_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN3
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_0_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN4
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_0_IN5 : X_BUF
    port map (
      I => FC2_IBUF_19,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN5
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_0_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN6
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_1_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN0
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_1_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN1
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_1_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN2
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_1_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN3
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_1_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN4
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_1_IN5 : X_BUF
    port map (
      I => FC1_IBUF_21,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN5
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_1_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN6
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_2_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN0
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_2_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN1
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_2_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN2
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_2_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN3
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_2_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN4
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_2_IN5 : X_BUF
    port map (
      I => FC0_IBUF_23,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN5
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_PT_2_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN6
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_IN0 : X_BUF
    port map (
      I => VGA_CS_OBUF_D2_PT_0_242,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_IN0
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_IN1 : X_BUF
    port map (
      I => VGA_CS_OBUF_D2_PT_1_243,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_IN1
    );
  NlwBufferBlock_VGA_CS_OBUF_D2_IN2 : X_BUF
    port map (
      I => VGA_CS_OBUF_D2_PT_2_244,
      O => NlwBufferSignal_VGA_CS_OBUF_D2_IN2
    );
  NlwBufferBlock_nRW0_OBUF_D_IN0 : X_BUF
    port map (
      I => nRW0_OBUF_D1_247,
      O => NlwBufferSignal_nRW0_OBUF_D_IN0
    );
  NlwBufferBlock_nRW0_OBUF_D_IN1 : X_BUF
    port map (
      I => nRW0_OBUF_D2_248,
      O => NlwBufferSignal_nRW0_OBUF_D_IN1
    );
  NlwBufferBlock_nRW0_OBUF_D2_PT_0_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_nRW0_OBUF_D2_PT_0_IN0
    );
  NlwBufferBlock_nRW0_OBUF_D2_PT_0_IN1 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_nRW0_OBUF_D2_PT_0_IN1
    );
  NlwBufferBlock_nRW0_OBUF_D2_PT_1_IN0 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_nRW0_OBUF_D2_PT_1_IN0
    );
  NlwBufferBlock_nRW0_OBUF_D2_PT_1_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_nRW0_OBUF_D2_PT_1_IN1
    );
  NlwBufferBlock_nRW0_OBUF_D2_PT_2_IN0 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_nRW0_OBUF_D2_PT_2_IN0
    );
  NlwBufferBlock_nRW0_OBUF_D2_PT_2_IN1 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_nRW0_OBUF_D2_PT_2_IN1
    );
  NlwBufferBlock_nRW0_OBUF_D2_PT_3_IN0 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_nRW0_OBUF_D2_PT_3_IN0
    );
  NlwBufferBlock_nRW0_OBUF_D2_PT_3_IN1 : X_BUF
    port map (
      I => RW_N_IBUF_33,
      O => NlwBufferSignal_nRW0_OBUF_D2_PT_3_IN1
    );
  NlwBufferBlock_nRW0_OBUF_D2_PT_4_IN0 : X_BUF
    port map (
      I => FC2_IBUF_19,
      O => NlwBufferSignal_nRW0_OBUF_D2_PT_4_IN0
    );
  NlwBufferBlock_nRW0_OBUF_D2_PT_4_IN1 : X_BUF
    port map (
      I => FC1_IBUF_21,
      O => NlwBufferSignal_nRW0_OBUF_D2_PT_4_IN1
    );
  NlwBufferBlock_nRW0_OBUF_D2_PT_4_IN2 : X_BUF
    port map (
      I => FC0_IBUF_23,
      O => NlwBufferSignal_nRW0_OBUF_D2_PT_4_IN2
    );
  NlwBufferBlock_nRW0_OBUF_D2_IN0 : X_BUF
    port map (
      I => nRW0_OBUF_D2_PT_0_249,
      O => NlwBufferSignal_nRW0_OBUF_D2_IN0
    );
  NlwBufferBlock_nRW0_OBUF_D2_IN1 : X_BUF
    port map (
      I => nRW0_OBUF_D2_PT_1_250,
      O => NlwBufferSignal_nRW0_OBUF_D2_IN1
    );
  NlwBufferBlock_nRW0_OBUF_D2_IN2 : X_BUF
    port map (
      I => nRW0_OBUF_D2_PT_2_251,
      O => NlwBufferSignal_nRW0_OBUF_D2_IN2
    );
  NlwBufferBlock_nRW0_OBUF_D2_IN3 : X_BUF
    port map (
      I => nRW0_OBUF_D2_PT_3_252,
      O => NlwBufferSignal_nRW0_OBUF_D2_IN3
    );
  NlwBufferBlock_nRW0_OBUF_D2_IN4 : X_BUF
    port map (
      I => nRW0_OBUF_D2_PT_4_253,
      O => NlwBufferSignal_nRW0_OBUF_D2_IN4
    );
  NlwBufferBlock_nRW1_OBUF_D_IN0 : X_BUF
    port map (
      I => nRW1_OBUF_D1_256,
      O => NlwBufferSignal_nRW1_OBUF_D_IN0
    );
  NlwBufferBlock_nRW1_OBUF_D_IN1 : X_BUF
    port map (
      I => nRW1_OBUF_D2_257,
      O => NlwBufferSignal_nRW1_OBUF_D_IN1
    );
  NlwBufferBlock_nRW1_OBUF_D2_PT_0_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_nRW1_OBUF_D2_PT_0_IN0
    );
  NlwBufferBlock_nRW1_OBUF_D2_PT_0_IN1 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_nRW1_OBUF_D2_PT_0_IN1
    );
  NlwBufferBlock_nRW1_OBUF_D2_PT_1_IN0 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_nRW1_OBUF_D2_PT_1_IN0
    );
  NlwBufferBlock_nRW1_OBUF_D2_PT_1_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_nRW1_OBUF_D2_PT_1_IN1
    );
  NlwBufferBlock_nRW1_OBUF_D2_PT_2_IN0 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_nRW1_OBUF_D2_PT_2_IN0
    );
  NlwBufferBlock_nRW1_OBUF_D2_PT_2_IN1 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_nRW1_OBUF_D2_PT_2_IN1
    );
  NlwBufferBlock_nRW1_OBUF_D2_PT_3_IN0 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_nRW1_OBUF_D2_PT_3_IN0
    );
  NlwBufferBlock_nRW1_OBUF_D2_PT_3_IN1 : X_BUF
    port map (
      I => RW_N_IBUF_33,
      O => NlwBufferSignal_nRW1_OBUF_D2_PT_3_IN1
    );
  NlwBufferBlock_nRW1_OBUF_D2_PT_4_IN0 : X_BUF
    port map (
      I => FC2_IBUF_19,
      O => NlwBufferSignal_nRW1_OBUF_D2_PT_4_IN0
    );
  NlwBufferBlock_nRW1_OBUF_D2_PT_4_IN1 : X_BUF
    port map (
      I => FC1_IBUF_21,
      O => NlwBufferSignal_nRW1_OBUF_D2_PT_4_IN1
    );
  NlwBufferBlock_nRW1_OBUF_D2_PT_4_IN2 : X_BUF
    port map (
      I => FC0_IBUF_23,
      O => NlwBufferSignal_nRW1_OBUF_D2_PT_4_IN2
    );
  NlwBufferBlock_nRW1_OBUF_D2_IN0 : X_BUF
    port map (
      I => nRW1_OBUF_D2_PT_0_258,
      O => NlwBufferSignal_nRW1_OBUF_D2_IN0
    );
  NlwBufferBlock_nRW1_OBUF_D2_IN1 : X_BUF
    port map (
      I => nRW1_OBUF_D2_PT_1_259,
      O => NlwBufferSignal_nRW1_OBUF_D2_IN1
    );
  NlwBufferBlock_nRW1_OBUF_D2_IN2 : X_BUF
    port map (
      I => nRW1_OBUF_D2_PT_2_260,
      O => NlwBufferSignal_nRW1_OBUF_D2_IN2
    );
  NlwBufferBlock_nRW1_OBUF_D2_IN3 : X_BUF
    port map (
      I => nRW1_OBUF_D2_PT_3_261,
      O => NlwBufferSignal_nRW1_OBUF_D2_IN3
    );
  NlwBufferBlock_nRW1_OBUF_D2_IN4 : X_BUF
    port map (
      I => nRW1_OBUF_D2_PT_4_262,
      O => NlwBufferSignal_nRW1_OBUF_D2_IN4
    );
  NlwBufferBlock_HALT_OBUF_BUF0_D_IN0 : X_BUF
    port map (
      I => HALT_OBUF_BUF0_D1_265,
      O => NlwBufferSignal_HALT_OBUF_BUF0_D_IN0
    );
  NlwBufferBlock_HALT_OBUF_BUF0_D_IN1 : X_BUF
    port map (
      I => HALT_OBUF_BUF0_D2_266,
      O => NlwBufferSignal_HALT_OBUF_BUF0_D_IN1
    );
  NlwBufferBlock_HALT_OBUF_BUF0_D2_IN0 : X_BUF
    port map (
      I => HALT_OBUF_27,
      O => NlwBufferSignal_HALT_OBUF_BUF0_D2_IN0
    );
  NlwBufferBlock_HALT_OBUF_BUF0_D2_IN1 : X_BUF
    port map (
      I => HALT_OBUF_27,
      O => NlwBufferSignal_HALT_OBUF_BUF0_D2_IN1
    );
  NlwBufferBlock_HALT_OBUF_BUF1_D_IN0 : X_BUF
    port map (
      I => HALT_OBUF_BUF1_D1_269,
      O => NlwBufferSignal_HALT_OBUF_BUF1_D_IN0
    );
  NlwBufferBlock_HALT_OBUF_BUF1_D_IN1 : X_BUF
    port map (
      I => HALT_OBUF_BUF1_D2_270,
      O => NlwBufferSignal_HALT_OBUF_BUF1_D_IN1
    );
  NlwBufferBlock_HALT_OBUF_BUF1_D2_IN0 : X_BUF
    port map (
      I => HALT_OBUF_27,
      O => NlwBufferSignal_HALT_OBUF_BUF1_D2_IN0
    );
  NlwBufferBlock_HALT_OBUF_BUF1_D2_IN1 : X_BUF
    port map (
      I => HALT_OBUF_27,
      O => NlwBufferSignal_HALT_OBUF_BUF1_D2_IN1
    );
  NlwBufferBlock_READ_n_OBUF_D_IN0 : X_BUF
    port map (
      I => READ_n_OBUF_D1_273,
      O => NlwBufferSignal_READ_n_OBUF_D_IN0
    );
  NlwBufferBlock_READ_n_OBUF_D_IN1 : X_BUF
    port map (
      I => READ_n_OBUF_D2_274,
      O => NlwBufferSignal_READ_n_OBUF_D_IN1
    );
  NlwBufferBlock_READ_n_OBUF_D2_IN0 : X_BUF
    port map (
      I => RW_N_IBUF_33,
      O => NlwBufferSignal_READ_n_OBUF_D2_IN0
    );
  NlwBufferBlock_READ_n_OBUF_D2_IN1 : X_BUF
    port map (
      I => RW_N_IBUF_33,
      O => NlwBufferSignal_READ_n_OBUF_D2_IN1
    );
  NlwBufferBlock_I_ALE_OBUF_D_IN0 : X_BUF
    port map (
      I => I_ALE_OBUF_D1_277,
      O => NlwBufferSignal_I_ALE_OBUF_D_IN0
    );
  NlwBufferBlock_I_ALE_OBUF_D_IN1 : X_BUF
    port map (
      I => I_ALE_OBUF_D2_278,
      O => NlwBufferSignal_I_ALE_OBUF_D_IN1
    );
  NlwBufferBlock_I_ALE_OBUF_BUF0_D_IN0 : X_BUF
    port map (
      I => I_ALE_OBUF_BUF0_D1_281,
      O => NlwBufferSignal_I_ALE_OBUF_BUF0_D_IN0
    );
  NlwBufferBlock_I_ALE_OBUF_BUF0_D_IN1 : X_BUF
    port map (
      I => I_ALE_OBUF_BUF0_D2_282,
      O => NlwBufferSignal_I_ALE_OBUF_BUF0_D_IN1
    );
  NlwBufferBlock_I_ALE_OBUF_BUF1_D_IN0 : X_BUF
    port map (
      I => I_ALE_OBUF_BUF1_D1_285,
      O => NlwBufferSignal_I_ALE_OBUF_BUF1_D_IN0
    );
  NlwBufferBlock_I_ALE_OBUF_BUF1_D_IN1 : X_BUF
    port map (
      I => I_ALE_OBUF_BUF1_D2_286,
      O => NlwBufferSignal_I_ALE_OBUF_BUF1_D_IN1
    );
  NlwBufferBlock_I_MEMW_OBUF_D_IN0 : X_BUF
    port map (
      I => I_MEMW_OBUF_D1_289,
      O => NlwBufferSignal_I_MEMW_OBUF_D_IN0
    );
  NlwBufferBlock_I_MEMW_OBUF_D_IN1 : X_BUF
    port map (
      I => I_MEMW_OBUF_D2_290,
      O => NlwBufferSignal_I_MEMW_OBUF_D_IN1
    );
  NlwBufferBlock_I_MEMW_OBUF_BUF0_D_IN0 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF0_D1_293,
      O => NlwBufferSignal_I_MEMW_OBUF_BUF0_D_IN0
    );
  NlwBufferBlock_I_MEMW_OBUF_BUF0_D_IN1 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF0_D2_294,
      O => NlwBufferSignal_I_MEMW_OBUF_BUF0_D_IN1
    );
  NlwBufferBlock_I_MEMW_OBUF_BUF1_D_IN0 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF1_D1_298,
      O => NlwBufferSignal_I_MEMW_OBUF_BUF1_D_IN0
    );
  NlwBufferBlock_I_MEMW_OBUF_BUF1_D_IN1 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF1_D2_299,
      O => NlwBufferSignal_I_MEMW_OBUF_BUF1_D_IN1
    );
  NlwBufferBlock_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN0 : X_BUF
    port map (
      I => a27_IBUF_1,
      O => NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN0
    );
  NlwBufferBlock_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN1 : X_BUF
    port map (
      I => a26_IBUF_3,
      O => NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN1
    );
  NlwBufferBlock_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN2 : X_BUF
    port map (
      I => a21_IBUF_5,
      O => NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN2
    );
  NlwBufferBlock_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN3 : X_BUF
    port map (
      I => a19_IBUF_15,
      O => NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN3
    );
  NlwBufferBlock_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN4 : X_BUF
    port map (
      I => a20_IBUF_17,
      O => NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN4
    );
  NlwBufferBlock_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN5 : X_BUF
    port map (
      I => FC2_IBUF_19,
      O => NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN5
    );
  NlwBufferBlock_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN6 : X_BUF
    port map (
      I => AS_IBUF_25,
      O => NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN6
    );
  NlwBufferBlock_I_MEMW_OBUF_BUF2_D_IN0 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF2_D1_302,
      O => NlwBufferSignal_I_MEMW_OBUF_BUF2_D_IN0
    );
  NlwBufferBlock_I_MEMW_OBUF_BUF2_D_IN1 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF2_D2_303,
      O => NlwBufferSignal_I_MEMW_OBUF_BUF2_D_IN1
    );
  NlwBufferBlock_I_MEMW_OBUF_BUF3_D_IN0 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF3_D1_306,
      O => NlwBufferSignal_I_MEMW_OBUF_BUF3_D_IN0
    );
  NlwBufferBlock_I_MEMW_OBUF_BUF3_D_IN1 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF3_D2_307,
      O => NlwBufferSignal_I_MEMW_OBUF_BUF3_D_IN1
    );
  NlwBufferBlock_I_MEMW_OBUF_BUF4_D_IN0 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF4_D1_310,
      O => NlwBufferSignal_I_MEMW_OBUF_BUF4_D_IN0
    );
  NlwBufferBlock_I_MEMW_OBUF_BUF4_D_IN1 : X_BUF
    port map (
      I => I_MEMW_OBUF_BUF4_D2_311,
      O => NlwBufferSignal_I_MEMW_OBUF_BUF4_D_IN1
    );
  NlwBufferBlock_cntr_2_cntr_2_RSTF_INT_D_IN0 : X_BUF
    port map (
      I => cntr_2_cntr_2_RSTF_INT_D1_314,
      O => NlwBufferSignal_cntr_2_cntr_2_RSTF_INT_D_IN0
    );
  NlwBufferBlock_cntr_2_cntr_2_RSTF_INT_D_IN1 : X_BUF
    port map (
      I => cntr_2_cntr_2_RSTF_INT_D2_315,
      O => NlwBufferSignal_cntr_2_cntr_2_RSTF_INT_D_IN1
    );
  NlwBufferBlock_cntr_2_cntr_2_RSTF_INT_D2_IN0 : X_BUF
    port map (
      I => HALT_OBUF_27,
      O => NlwBufferSignal_cntr_2_cntr_2_RSTF_INT_D2_IN0
    );
  NlwBufferBlock_cntr_2_cntr_2_RSTF_INT_D2_IN1 : X_BUF
    port map (
      I => ECS_IBUF_29,
      O => NlwBufferSignal_cntr_2_cntr_2_RSTF_INT_D2_IN1
    );
  NlwInverterBlock_DSACK0_OBUF_D2_PT_0_IN0 : X_INV
    port map (
      I => NlwBufferSignal_DSACK0_OBUF_D2_PT_0_IN0,
      O => NlwInverterSignal_DSACK0_OBUF_D2_PT_0_IN0
    );
  NlwInverterBlock_DSACK0_OBUF_D2_PT_0_IN1 : X_INV
    port map (
      I => NlwBufferSignal_DSACK0_OBUF_D2_PT_0_IN1,
      O => NlwInverterSignal_DSACK0_OBUF_D2_PT_0_IN1
    );
  NlwInverterBlock_DSACK0_OBUF_D2_PT_0_IN2 : X_INV
    port map (
      I => NlwBufferSignal_DSACK0_OBUF_D2_PT_0_IN2,
      O => NlwInverterSignal_DSACK0_OBUF_D2_PT_0_IN2
    );
  NlwInverterBlock_DSACK0_OBUF_D2_PT_1_IN0 : X_INV
    port map (
      I => NlwBufferSignal_DSACK0_OBUF_D2_PT_1_IN0,
      O => NlwInverterSignal_DSACK0_OBUF_D2_PT_1_IN0
    );
  NlwInverterBlock_DSACK0_OBUF_D2_PT_1_IN1 : X_INV
    port map (
      I => NlwBufferSignal_DSACK0_OBUF_D2_PT_1_IN1,
      O => NlwInverterSignal_DSACK0_OBUF_D2_PT_1_IN1
    );
  NlwInverterBlock_DSACK0_OBUF_D2_PT_1_IN2 : X_INV
    port map (
      I => NlwBufferSignal_DSACK0_OBUF_D2_PT_1_IN2,
      O => NlwInverterSignal_DSACK0_OBUF_D2_PT_1_IN2
    );
  NlwInverterBlock_cntr_0_RSTF_IN0 : X_INV
    port map (
      I => NlwBufferSignal_cntr_0_RSTF_IN0,
      O => NlwInverterSignal_cntr_0_RSTF_IN0
    );
  NlwInverterBlock_cntr_0_RSTF_IN1 : X_INV
    port map (
      I => NlwBufferSignal_cntr_0_RSTF_IN1,
      O => NlwInverterSignal_cntr_0_RSTF_IN1
    );
  NlwInverterBlock_cntr_1_RSTF_IN0 : X_INV
    port map (
      I => NlwBufferSignal_cntr_1_RSTF_IN0,
      O => NlwInverterSignal_cntr_1_RSTF_IN0
    );
  NlwInverterBlock_cntr_1_RSTF_IN1 : X_INV
    port map (
      I => NlwBufferSignal_cntr_1_RSTF_IN1,
      O => NlwInverterSignal_cntr_1_RSTF_IN1
    );
  NlwInverterBlock_cntr_2_RSTF_IN0 : X_INV
    port map (
      I => NlwBufferSignal_cntr_2_RSTF_IN0,
      O => NlwInverterSignal_cntr_2_RSTF_IN0
    );
  NlwInverterBlock_cntr_2_RSTF_IN1 : X_INV
    port map (
      I => NlwBufferSignal_cntr_2_RSTF_IN1,
      O => NlwInverterSignal_cntr_2_RSTF_IN1
    );
  NlwInverterBlock_cntr_3_RSTF_IN0 : X_INV
    port map (
      I => NlwBufferSignal_cntr_3_RSTF_IN0,
      O => NlwInverterSignal_cntr_3_RSTF_IN0
    );
  NlwInverterBlock_cntr_3_RSTF_IN1 : X_INV
    port map (
      I => NlwBufferSignal_cntr_3_RSTF_IN1,
      O => NlwInverterSignal_cntr_3_RSTF_IN1
    );
  NlwInverterBlock_BYTE0_OBUF_D2_PT_0_IN1 : X_INV
    port map (
      I => NlwBufferSignal_BYTE0_OBUF_D2_PT_0_IN1,
      O => NlwInverterSignal_BYTE0_OBUF_D2_PT_0_IN1
    );
  NlwInverterBlock_BYTE0_OBUF_D2_PT_0_IN2 : X_INV
    port map (
      I => NlwBufferSignal_BYTE0_OBUF_D2_PT_0_IN2,
      O => NlwInverterSignal_BYTE0_OBUF_D2_PT_0_IN2
    );
  NlwInverterBlock_BYTE0_OBUF_D2_PT_1_IN1 : X_INV
    port map (
      I => NlwBufferSignal_BYTE0_OBUF_D2_PT_1_IN1,
      O => NlwInverterSignal_BYTE0_OBUF_D2_PT_1_IN1
    );
  NlwInverterBlock_BYTE0_OBUF_D2_PT_1_IN2 : X_INV
    port map (
      I => NlwBufferSignal_BYTE0_OBUF_D2_PT_1_IN2,
      O => NlwInverterSignal_BYTE0_OBUF_D2_PT_1_IN2
    );
  NlwInverterBlock_BYTE0_OBUF_D2_PT_2_IN1 : X_INV
    port map (
      I => NlwBufferSignal_BYTE0_OBUF_D2_PT_2_IN1,
      O => NlwInverterSignal_BYTE0_OBUF_D2_PT_2_IN1
    );
  NlwInverterBlock_BYTE0_OBUF_D2_PT_2_IN2 : X_INV
    port map (
      I => NlwBufferSignal_BYTE0_OBUF_D2_PT_2_IN2,
      O => NlwInverterSignal_BYTE0_OBUF_D2_PT_2_IN2
    );
  NlwInverterBlock_BYTE0_OBUF_D2_PT_3_IN0 : X_INV
    port map (
      I => NlwBufferSignal_BYTE0_OBUF_D2_PT_3_IN0,
      O => NlwInverterSignal_BYTE0_OBUF_D2_PT_3_IN0
    );
  NlwInverterBlock_BYTE0_OBUF_D2_PT_3_IN2 : X_INV
    port map (
      I => NlwBufferSignal_BYTE0_OBUF_D2_PT_3_IN2,
      O => NlwInverterSignal_BYTE0_OBUF_D2_PT_3_IN2
    );
  NlwInverterBlock_BYTE1_OBUF_D2_PT_1_IN1 : X_INV
    port map (
      I => NlwBufferSignal_BYTE1_OBUF_D2_PT_1_IN1,
      O => NlwInverterSignal_BYTE1_OBUF_D2_PT_1_IN1
    );
  NlwInverterBlock_BYTE1_OBUF_D2_PT_1_IN2 : X_INV
    port map (
      I => NlwBufferSignal_BYTE1_OBUF_D2_PT_1_IN2,
      O => NlwInverterSignal_BYTE1_OBUF_D2_PT_1_IN2
    );
  NlwInverterBlock_BYTE1_OBUF_D2_PT_2_IN0 : X_INV
    port map (
      I => NlwBufferSignal_BYTE1_OBUF_D2_PT_2_IN0,
      O => NlwInverterSignal_BYTE1_OBUF_D2_PT_2_IN0
    );
  NlwInverterBlock_BYTE1_OBUF_D2_PT_2_IN1 : X_INV
    port map (
      I => NlwBufferSignal_BYTE1_OBUF_D2_PT_2_IN1,
      O => NlwInverterSignal_BYTE1_OBUF_D2_PT_2_IN1
    );
  NlwInverterBlock_BYTE1_OBUF_D2_PT_2_IN3 : X_INV
    port map (
      I => NlwBufferSignal_BYTE1_OBUF_D2_PT_2_IN3,
      O => NlwInverterSignal_BYTE1_OBUF_D2_PT_2_IN3
    );
  NlwInverterBlock_BYTE2_OBUF_D2_PT_1_IN1 : X_INV
    port map (
      I => NlwBufferSignal_BYTE2_OBUF_D2_PT_1_IN1,
      O => NlwInverterSignal_BYTE2_OBUF_D2_PT_1_IN1
    );
  NlwInverterBlock_BYTE2_OBUF_D2_PT_1_IN2 : X_INV
    port map (
      I => NlwBufferSignal_BYTE2_OBUF_D2_PT_1_IN2,
      O => NlwInverterSignal_BYTE2_OBUF_D2_PT_1_IN2
    );
  NlwInverterBlock_BYTE3_OBUF_D_IN0 : X_INV
    port map (
      I => NlwBufferSignal_BYTE3_OBUF_D_IN0,
      O => NlwInverterSignal_BYTE3_OBUF_D_IN0
    );
  NlwInverterBlock_BYTE3_OBUF_D2_IN0 : X_INV
    port map (
      I => NlwBufferSignal_BYTE3_OBUF_D2_IN0,
      O => NlwInverterSignal_BYTE3_OBUF_D2_IN0
    );
  NlwInverterBlock_BYTE3_OBUF_D2_IN1 : X_INV
    port map (
      I => NlwBufferSignal_BYTE3_OBUF_D2_IN1,
      O => NlwInverterSignal_BYTE3_OBUF_D2_IN1
    );
  NlwInverterBlock_DRAM_CS_OBUF_D2_PT_0_IN0 : X_INV
    port map (
      I => NlwBufferSignal_DRAM_CS_OBUF_D2_PT_0_IN0,
      O => NlwInverterSignal_DRAM_CS_OBUF_D2_PT_0_IN0
    );
  NlwInverterBlock_DRAM_CS_OBUF_D2_PT_0_IN1 : X_INV
    port map (
      I => NlwBufferSignal_DRAM_CS_OBUF_D2_PT_0_IN1,
      O => NlwInverterSignal_DRAM_CS_OBUF_D2_PT_0_IN1
    );
  NlwInverterBlock_DSACK1_OBUF_D2_PT_0_IN0 : X_INV
    port map (
      I => NlwBufferSignal_DSACK1_OBUF_D2_PT_0_IN0,
      O => NlwInverterSignal_DSACK1_OBUF_D2_PT_0_IN0
    );
  NlwInverterBlock_DSACK1_OBUF_D2_PT_0_IN1 : X_INV
    port map (
      I => NlwBufferSignal_DSACK1_OBUF_D2_PT_0_IN1,
      O => NlwInverterSignal_DSACK1_OBUF_D2_PT_0_IN1
    );
  NlwInverterBlock_DSACK1_OBUF_D2_PT_1_IN0 : X_INV
    port map (
      I => NlwBufferSignal_DSACK1_OBUF_D2_PT_1_IN0,
      O => NlwInverterSignal_DSACK1_OBUF_D2_PT_1_IN0
    );
  NlwInverterBlock_DSACK1_OBUF_D2_PT_1_IN1 : X_INV
    port map (
      I => NlwBufferSignal_DSACK1_OBUF_D2_PT_1_IN1,
      O => NlwInverterSignal_DSACK1_OBUF_D2_PT_1_IN1
    );
  NlwInverterBlock_DSACK1_OBUF_D2_PT_1_IN2 : X_INV
    port map (
      I => NlwBufferSignal_DSACK1_OBUF_D2_PT_1_IN2,
      O => NlwInverterSignal_DSACK1_OBUF_D2_PT_1_IN2
    );
  NlwInverterBlock_DSACK1_OBUF_D2_PT_2_IN0 : X_INV
    port map (
      I => NlwBufferSignal_DSACK1_OBUF_D2_PT_2_IN0,
      O => NlwInverterSignal_DSACK1_OBUF_D2_PT_2_IN0
    );
  NlwInverterBlock_DSACK1_OBUF_D2_PT_2_IN1 : X_INV
    port map (
      I => NlwBufferSignal_DSACK1_OBUF_D2_PT_2_IN1,
      O => NlwInverterSignal_DSACK1_OBUF_D2_PT_2_IN1
    );
  NlwInverterBlock_DSACK1_OBUF_D2_PT_2_IN2 : X_INV
    port map (
      I => NlwBufferSignal_DSACK1_OBUF_D2_PT_2_IN2,
      O => NlwInverterSignal_DSACK1_OBUF_D2_PT_2_IN2
    );
  NlwInverterBlock_ETHRNT_OBUF_D_IN0 : X_INV
    port map (
      I => NlwBufferSignal_ETHRNT_OBUF_D_IN0,
      O => NlwInverterSignal_ETHRNT_OBUF_D_IN0
    );
  NlwInverterBlock_ETHRNT_OBUF_D2_PT_0_IN0 : X_INV
    port map (
      I => NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN0,
      O => NlwInverterSignal_ETHRNT_OBUF_D2_PT_0_IN0
    );
  NlwInverterBlock_ETHRNT_OBUF_D2_PT_0_IN1 : X_INV
    port map (
      I => NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN1,
      O => NlwInverterSignal_ETHRNT_OBUF_D2_PT_0_IN1
    );
  NlwInverterBlock_ETHRNT_OBUF_D2_PT_0_IN4 : X_INV
    port map (
      I => NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN4,
      O => NlwInverterSignal_ETHRNT_OBUF_D2_PT_0_IN4
    );
  NlwInverterBlock_ETHRNT_OBUF_D2_PT_0_IN5 : X_INV
    port map (
      I => NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN5,
      O => NlwInverterSignal_ETHRNT_OBUF_D2_PT_0_IN5
    );
  NlwInverterBlock_ETHRNT_OBUF_D2_PT_0_IN6 : X_INV
    port map (
      I => NlwBufferSignal_ETHRNT_OBUF_D2_PT_0_IN6,
      O => NlwInverterSignal_ETHRNT_OBUF_D2_PT_0_IN6
    );
  NlwInverterBlock_ETHRNT_OBUF_D2_PT_1_IN0 : X_INV
    port map (
      I => NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN0,
      O => NlwInverterSignal_ETHRNT_OBUF_D2_PT_1_IN0
    );
  NlwInverterBlock_ETHRNT_OBUF_D2_PT_1_IN1 : X_INV
    port map (
      I => NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN1,
      O => NlwInverterSignal_ETHRNT_OBUF_D2_PT_1_IN1
    );
  NlwInverterBlock_ETHRNT_OBUF_D2_PT_1_IN4 : X_INV
    port map (
      I => NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN4,
      O => NlwInverterSignal_ETHRNT_OBUF_D2_PT_1_IN4
    );
  NlwInverterBlock_ETHRNT_OBUF_D2_PT_1_IN5 : X_INV
    port map (
      I => NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN5,
      O => NlwInverterSignal_ETHRNT_OBUF_D2_PT_1_IN5
    );
  NlwInverterBlock_ETHRNT_OBUF_D2_PT_1_IN6 : X_INV
    port map (
      I => NlwBufferSignal_ETHRNT_OBUF_D2_PT_1_IN6,
      O => NlwInverterSignal_ETHRNT_OBUF_D2_PT_1_IN6
    );
  NlwInverterBlock_ETHRNT_OBUF_D2_PT_2_IN0 : X_INV
    port map (
      I => NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN0,
      O => NlwInverterSignal_ETHRNT_OBUF_D2_PT_2_IN0
    );
  NlwInverterBlock_ETHRNT_OBUF_D2_PT_2_IN1 : X_INV
    port map (
      I => NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN1,
      O => NlwInverterSignal_ETHRNT_OBUF_D2_PT_2_IN1
    );
  NlwInverterBlock_ETHRNT_OBUF_D2_PT_2_IN4 : X_INV
    port map (
      I => NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN4,
      O => NlwInverterSignal_ETHRNT_OBUF_D2_PT_2_IN4
    );
  NlwInverterBlock_ETHRNT_OBUF_D2_PT_2_IN5 : X_INV
    port map (
      I => NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN5,
      O => NlwInverterSignal_ETHRNT_OBUF_D2_PT_2_IN5
    );
  NlwInverterBlock_ETHRNT_OBUF_D2_PT_2_IN6 : X_INV
    port map (
      I => NlwBufferSignal_ETHRNT_OBUF_D2_PT_2_IN6,
      O => NlwInverterSignal_ETHRNT_OBUF_D2_PT_2_IN6
    );
  NlwInverterBlock_FLOPPY_OBUF_D_IN0 : X_INV
    port map (
      I => NlwBufferSignal_FLOPPY_OBUF_D_IN0,
      O => NlwInverterSignal_FLOPPY_OBUF_D_IN0
    );
  NlwInverterBlock_FLOPPY_OBUF_D2_PT_0_IN0 : X_INV
    port map (
      I => NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN0,
      O => NlwInverterSignal_FLOPPY_OBUF_D2_PT_0_IN0
    );
  NlwInverterBlock_FLOPPY_OBUF_D2_PT_0_IN1 : X_INV
    port map (
      I => NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN1,
      O => NlwInverterSignal_FLOPPY_OBUF_D2_PT_0_IN1
    );
  NlwInverterBlock_FLOPPY_OBUF_D2_PT_0_IN2 : X_INV
    port map (
      I => NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN2,
      O => NlwInverterSignal_FLOPPY_OBUF_D2_PT_0_IN2
    );
  NlwInverterBlock_FLOPPY_OBUF_D2_PT_0_IN4 : X_INV
    port map (
      I => NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN4,
      O => NlwInverterSignal_FLOPPY_OBUF_D2_PT_0_IN4
    );
  NlwInverterBlock_FLOPPY_OBUF_D2_PT_0_IN5 : X_INV
    port map (
      I => NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN5,
      O => NlwInverterSignal_FLOPPY_OBUF_D2_PT_0_IN5
    );
  NlwInverterBlock_FLOPPY_OBUF_D2_PT_0_IN6 : X_INV
    port map (
      I => NlwBufferSignal_FLOPPY_OBUF_D2_PT_0_IN6,
      O => NlwInverterSignal_FLOPPY_OBUF_D2_PT_0_IN6
    );
  NlwInverterBlock_FLOPPY_OBUF_D2_PT_1_IN0 : X_INV
    port map (
      I => NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN0,
      O => NlwInverterSignal_FLOPPY_OBUF_D2_PT_1_IN0
    );
  NlwInverterBlock_FLOPPY_OBUF_D2_PT_1_IN1 : X_INV
    port map (
      I => NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN1,
      O => NlwInverterSignal_FLOPPY_OBUF_D2_PT_1_IN1
    );
  NlwInverterBlock_FLOPPY_OBUF_D2_PT_1_IN2 : X_INV
    port map (
      I => NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN2,
      O => NlwInverterSignal_FLOPPY_OBUF_D2_PT_1_IN2
    );
  NlwInverterBlock_FLOPPY_OBUF_D2_PT_1_IN4 : X_INV
    port map (
      I => NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN4,
      O => NlwInverterSignal_FLOPPY_OBUF_D2_PT_1_IN4
    );
  NlwInverterBlock_FLOPPY_OBUF_D2_PT_1_IN5 : X_INV
    port map (
      I => NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN5,
      O => NlwInverterSignal_FLOPPY_OBUF_D2_PT_1_IN5
    );
  NlwInverterBlock_FLOPPY_OBUF_D2_PT_1_IN6 : X_INV
    port map (
      I => NlwBufferSignal_FLOPPY_OBUF_D2_PT_1_IN6,
      O => NlwInverterSignal_FLOPPY_OBUF_D2_PT_1_IN6
    );
  NlwInverterBlock_FLOPPY_OBUF_D2_PT_2_IN0 : X_INV
    port map (
      I => NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN0,
      O => NlwInverterSignal_FLOPPY_OBUF_D2_PT_2_IN0
    );
  NlwInverterBlock_FLOPPY_OBUF_D2_PT_2_IN1 : X_INV
    port map (
      I => NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN1,
      O => NlwInverterSignal_FLOPPY_OBUF_D2_PT_2_IN1
    );
  NlwInverterBlock_FLOPPY_OBUF_D2_PT_2_IN2 : X_INV
    port map (
      I => NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN2,
      O => NlwInverterSignal_FLOPPY_OBUF_D2_PT_2_IN2
    );
  NlwInverterBlock_FLOPPY_OBUF_D2_PT_2_IN4 : X_INV
    port map (
      I => NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN4,
      O => NlwInverterSignal_FLOPPY_OBUF_D2_PT_2_IN4
    );
  NlwInverterBlock_FLOPPY_OBUF_D2_PT_2_IN5 : X_INV
    port map (
      I => NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN5,
      O => NlwInverterSignal_FLOPPY_OBUF_D2_PT_2_IN5
    );
  NlwInverterBlock_FLOPPY_OBUF_D2_PT_2_IN6 : X_INV
    port map (
      I => NlwBufferSignal_FLOPPY_OBUF_D2_PT_2_IN6,
      O => NlwInverterSignal_FLOPPY_OBUF_D2_PT_2_IN6
    );
  NlwInverterBlock_IDE_OBUF_D_IN0 : X_INV
    port map (
      I => NlwBufferSignal_IDE_OBUF_D_IN0,
      O => NlwInverterSignal_IDE_OBUF_D_IN0
    );
  NlwInverterBlock_IDE_OBUF_D2_PT_0_IN0 : X_INV
    port map (
      I => NlwBufferSignal_IDE_OBUF_D2_PT_0_IN0,
      O => NlwInverterSignal_IDE_OBUF_D2_PT_0_IN0
    );
  NlwInverterBlock_IDE_OBUF_D2_PT_0_IN1 : X_INV
    port map (
      I => NlwBufferSignal_IDE_OBUF_D2_PT_0_IN1,
      O => NlwInverterSignal_IDE_OBUF_D2_PT_0_IN1
    );
  NlwInverterBlock_IDE_OBUF_D2_PT_0_IN2 : X_INV
    port map (
      I => NlwBufferSignal_IDE_OBUF_D2_PT_0_IN2,
      O => NlwInverterSignal_IDE_OBUF_D2_PT_0_IN2
    );
  NlwInverterBlock_IDE_OBUF_D2_PT_0_IN3 : X_INV
    port map (
      I => NlwBufferSignal_IDE_OBUF_D2_PT_0_IN3,
      O => NlwInverterSignal_IDE_OBUF_D2_PT_0_IN3
    );
  NlwInverterBlock_IDE_OBUF_D2_PT_0_IN5 : X_INV
    port map (
      I => NlwBufferSignal_IDE_OBUF_D2_PT_0_IN5,
      O => NlwInverterSignal_IDE_OBUF_D2_PT_0_IN5
    );
  NlwInverterBlock_IDE_OBUF_D2_PT_0_IN6 : X_INV
    port map (
      I => NlwBufferSignal_IDE_OBUF_D2_PT_0_IN6,
      O => NlwInverterSignal_IDE_OBUF_D2_PT_0_IN6
    );
  NlwInverterBlock_IDE_OBUF_D2_PT_1_IN0 : X_INV
    port map (
      I => NlwBufferSignal_IDE_OBUF_D2_PT_1_IN0,
      O => NlwInverterSignal_IDE_OBUF_D2_PT_1_IN0
    );
  NlwInverterBlock_IDE_OBUF_D2_PT_1_IN1 : X_INV
    port map (
      I => NlwBufferSignal_IDE_OBUF_D2_PT_1_IN1,
      O => NlwInverterSignal_IDE_OBUF_D2_PT_1_IN1
    );
  NlwInverterBlock_IDE_OBUF_D2_PT_1_IN2 : X_INV
    port map (
      I => NlwBufferSignal_IDE_OBUF_D2_PT_1_IN2,
      O => NlwInverterSignal_IDE_OBUF_D2_PT_1_IN2
    );
  NlwInverterBlock_IDE_OBUF_D2_PT_1_IN3 : X_INV
    port map (
      I => NlwBufferSignal_IDE_OBUF_D2_PT_1_IN3,
      O => NlwInverterSignal_IDE_OBUF_D2_PT_1_IN3
    );
  NlwInverterBlock_IDE_OBUF_D2_PT_1_IN5 : X_INV
    port map (
      I => NlwBufferSignal_IDE_OBUF_D2_PT_1_IN5,
      O => NlwInverterSignal_IDE_OBUF_D2_PT_1_IN5
    );
  NlwInverterBlock_IDE_OBUF_D2_PT_1_IN6 : X_INV
    port map (
      I => NlwBufferSignal_IDE_OBUF_D2_PT_1_IN6,
      O => NlwInverterSignal_IDE_OBUF_D2_PT_1_IN6
    );
  NlwInverterBlock_IDE_OBUF_D2_PT_2_IN0 : X_INV
    port map (
      I => NlwBufferSignal_IDE_OBUF_D2_PT_2_IN0,
      O => NlwInverterSignal_IDE_OBUF_D2_PT_2_IN0
    );
  NlwInverterBlock_IDE_OBUF_D2_PT_2_IN1 : X_INV
    port map (
      I => NlwBufferSignal_IDE_OBUF_D2_PT_2_IN1,
      O => NlwInverterSignal_IDE_OBUF_D2_PT_2_IN1
    );
  NlwInverterBlock_IDE_OBUF_D2_PT_2_IN2 : X_INV
    port map (
      I => NlwBufferSignal_IDE_OBUF_D2_PT_2_IN2,
      O => NlwInverterSignal_IDE_OBUF_D2_PT_2_IN2
    );
  NlwInverterBlock_IDE_OBUF_D2_PT_2_IN3 : X_INV
    port map (
      I => NlwBufferSignal_IDE_OBUF_D2_PT_2_IN3,
      O => NlwInverterSignal_IDE_OBUF_D2_PT_2_IN3
    );
  NlwInverterBlock_IDE_OBUF_D2_PT_2_IN5 : X_INV
    port map (
      I => NlwBufferSignal_IDE_OBUF_D2_PT_2_IN5,
      O => NlwInverterSignal_IDE_OBUF_D2_PT_2_IN5
    );
  NlwInverterBlock_IDE_OBUF_D2_PT_2_IN6 : X_INV
    port map (
      I => NlwBufferSignal_IDE_OBUF_D2_PT_2_IN6,
      O => NlwInverterSignal_IDE_OBUF_D2_PT_2_IN6
    );
  NlwInverterBlock_IO_CS_OBUF_D_IN0 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D_IN0,
      O => NlwInverterSignal_IO_CS_OBUF_D_IN0
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_1_IN0 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN0,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_1_IN0
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_1_IN1 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN1,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_1_IN1
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_1_IN2 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN2,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_1_IN2
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_1_IN4 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN4,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_1_IN4
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_1_IN5 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN5,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_1_IN5
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_1_IN6 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_1_IN6,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_1_IN6
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_2_IN0 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN0,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_2_IN0
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_2_IN1 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN1,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_2_IN1
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_2_IN2 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN2,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_2_IN2
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_2_IN4 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN4,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_2_IN4
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_2_IN5 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN5,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_2_IN5
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_2_IN6 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_2_IN6,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_2_IN6
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_3_IN0 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN0,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_3_IN0
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_3_IN1 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN1,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_3_IN1
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_3_IN2 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN2,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_3_IN2
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_3_IN3 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN3,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_3_IN3
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_3_IN5 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN5,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_3_IN5
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_3_IN6 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_3_IN6,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_3_IN6
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_4_IN0 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN0,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_4_IN0
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_4_IN1 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN1,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_4_IN1
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_4_IN2 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN2,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_4_IN2
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_4_IN3 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN3,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_4_IN3
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_4_IN5 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN5,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_4_IN5
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_4_IN6 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_4_IN6,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_4_IN6
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_5_IN0 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN0,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_5_IN0
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_5_IN1 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN1,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_5_IN1
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_5_IN2 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN2,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_5_IN2
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_5_IN3 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN3,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_5_IN3
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_5_IN5 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN5,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_5_IN5
    );
  NlwInverterBlock_IO_CS_OBUF_D2_PT_5_IN6 : X_INV
    port map (
      I => NlwBufferSignal_IO_CS_OBUF_D2_PT_5_IN6,
      O => NlwInverterSignal_IO_CS_OBUF_D2_PT_5_IN6
    );
  NlwInverterBlock_KBD_CS_OBUF_D_IN0 : X_INV
    port map (
      I => NlwBufferSignal_KBD_CS_OBUF_D_IN0,
      O => NlwInverterSignal_KBD_CS_OBUF_D_IN0
    );
  NlwInverterBlock_KBD_CS_OBUF_D2_PT_0_IN0 : X_INV
    port map (
      I => NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN0,
      O => NlwInverterSignal_KBD_CS_OBUF_D2_PT_0_IN0
    );
  NlwInverterBlock_KBD_CS_OBUF_D2_PT_0_IN1 : X_INV
    port map (
      I => NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN1,
      O => NlwInverterSignal_KBD_CS_OBUF_D2_PT_0_IN1
    );
  NlwInverterBlock_KBD_CS_OBUF_D2_PT_0_IN2 : X_INV
    port map (
      I => NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN2,
      O => NlwInverterSignal_KBD_CS_OBUF_D2_PT_0_IN2
    );
  NlwInverterBlock_KBD_CS_OBUF_D2_PT_0_IN5 : X_INV
    port map (
      I => NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN5,
      O => NlwInverterSignal_KBD_CS_OBUF_D2_PT_0_IN5
    );
  NlwInverterBlock_KBD_CS_OBUF_D2_PT_0_IN6 : X_INV
    port map (
      I => NlwBufferSignal_KBD_CS_OBUF_D2_PT_0_IN6,
      O => NlwInverterSignal_KBD_CS_OBUF_D2_PT_0_IN6
    );
  NlwInverterBlock_KBD_CS_OBUF_D2_PT_1_IN0 : X_INV
    port map (
      I => NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN0,
      O => NlwInverterSignal_KBD_CS_OBUF_D2_PT_1_IN0
    );
  NlwInverterBlock_KBD_CS_OBUF_D2_PT_1_IN1 : X_INV
    port map (
      I => NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN1,
      O => NlwInverterSignal_KBD_CS_OBUF_D2_PT_1_IN1
    );
  NlwInverterBlock_KBD_CS_OBUF_D2_PT_1_IN2 : X_INV
    port map (
      I => NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN2,
      O => NlwInverterSignal_KBD_CS_OBUF_D2_PT_1_IN2
    );
  NlwInverterBlock_KBD_CS_OBUF_D2_PT_1_IN5 : X_INV
    port map (
      I => NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN5,
      O => NlwInverterSignal_KBD_CS_OBUF_D2_PT_1_IN5
    );
  NlwInverterBlock_KBD_CS_OBUF_D2_PT_1_IN6 : X_INV
    port map (
      I => NlwBufferSignal_KBD_CS_OBUF_D2_PT_1_IN6,
      O => NlwInverterSignal_KBD_CS_OBUF_D2_PT_1_IN6
    );
  NlwInverterBlock_KBD_CS_OBUF_D2_PT_2_IN0 : X_INV
    port map (
      I => NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN0,
      O => NlwInverterSignal_KBD_CS_OBUF_D2_PT_2_IN0
    );
  NlwInverterBlock_KBD_CS_OBUF_D2_PT_2_IN1 : X_INV
    port map (
      I => NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN1,
      O => NlwInverterSignal_KBD_CS_OBUF_D2_PT_2_IN1
    );
  NlwInverterBlock_KBD_CS_OBUF_D2_PT_2_IN2 : X_INV
    port map (
      I => NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN2,
      O => NlwInverterSignal_KBD_CS_OBUF_D2_PT_2_IN2
    );
  NlwInverterBlock_KBD_CS_OBUF_D2_PT_2_IN5 : X_INV
    port map (
      I => NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN5,
      O => NlwInverterSignal_KBD_CS_OBUF_D2_PT_2_IN5
    );
  NlwInverterBlock_KBD_CS_OBUF_D2_PT_2_IN6 : X_INV
    port map (
      I => NlwBufferSignal_KBD_CS_OBUF_D2_PT_2_IN6,
      O => NlwInverterSignal_KBD_CS_OBUF_D2_PT_2_IN6
    );
  NlwInverterBlock_ROM_OBUF_D_IN0 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D_IN0,
      O => NlwInverterSignal_ROM_OBUF_D_IN0
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_0_IN0 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_0_IN0,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_0_IN0
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_0_IN1 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_0_IN1,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_0_IN1
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_0_IN2 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_0_IN2,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_0_IN2
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_0_IN3 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_0_IN3,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_0_IN3
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_0_IN4 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_0_IN4,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_0_IN4
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_0_IN5 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_0_IN5,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_0_IN5
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_0_IN6 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_0_IN6,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_0_IN6
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_1_IN0 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_1_IN0,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_1_IN0
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_1_IN1 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_1_IN1,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_1_IN1
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_1_IN2 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_1_IN2,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_1_IN2
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_1_IN3 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_1_IN3,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_1_IN3
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_1_IN4 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_1_IN4,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_1_IN4
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_1_IN5 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_1_IN5,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_1_IN5
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_1_IN6 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_1_IN6,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_1_IN6
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_2_IN0 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_2_IN0,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_2_IN0
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_2_IN1 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_2_IN1,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_2_IN1
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_2_IN2 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_2_IN2,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_2_IN2
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_2_IN3 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_2_IN3,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_2_IN3
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_2_IN4 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_2_IN4,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_2_IN4
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_2_IN5 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_2_IN5,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_2_IN5
    );
  NlwInverterBlock_ROM_OBUF_D2_PT_2_IN6 : X_INV
    port map (
      I => NlwBufferSignal_ROM_OBUF_D2_PT_2_IN6,
      O => NlwInverterSignal_ROM_OBUF_D2_PT_2_IN6
    );
  NlwInverterBlock_RW0_OBUF_D2_PT_1_IN0 : X_INV
    port map (
      I => NlwBufferSignal_RW0_OBUF_D2_PT_1_IN0,
      O => NlwInverterSignal_RW0_OBUF_D2_PT_1_IN0
    );
  NlwInverterBlock_RW0_OBUF_D2_PT_1_IN1 : X_INV
    port map (
      I => NlwBufferSignal_RW0_OBUF_D2_PT_1_IN1,
      O => NlwInverterSignal_RW0_OBUF_D2_PT_1_IN1
    );
  NlwInverterBlock_RW1_OBUF_D2_PT_1_IN0 : X_INV
    port map (
      I => NlwBufferSignal_RW1_OBUF_D2_PT_1_IN0,
      O => NlwInverterSignal_RW1_OBUF_D2_PT_1_IN0
    );
  NlwInverterBlock_RW1_OBUF_D2_PT_1_IN1 : X_INV
    port map (
      I => NlwBufferSignal_RW1_OBUF_D2_PT_1_IN1,
      O => NlwInverterSignal_RW1_OBUF_D2_PT_1_IN1
    );
  NlwInverterBlock_RW1_OBUF_D2_PT_3_IN0 : X_INV
    port map (
      I => NlwBufferSignal_RW1_OBUF_D2_PT_3_IN0,
      O => NlwInverterSignal_RW1_OBUF_D2_PT_3_IN0
    );
  NlwInverterBlock_VGA_CS_OBUF_D_IN0 : X_INV
    port map (
      I => NlwBufferSignal_VGA_CS_OBUF_D_IN0,
      O => NlwInverterSignal_VGA_CS_OBUF_D_IN0
    );
  NlwInverterBlock_VGA_CS_OBUF_D2_PT_0_IN0 : X_INV
    port map (
      I => NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN0,
      O => NlwInverterSignal_VGA_CS_OBUF_D2_PT_0_IN0
    );
  NlwInverterBlock_VGA_CS_OBUF_D2_PT_0_IN1 : X_INV
    port map (
      I => NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN1,
      O => NlwInverterSignal_VGA_CS_OBUF_D2_PT_0_IN1
    );
  NlwInverterBlock_VGA_CS_OBUF_D2_PT_0_IN3 : X_INV
    port map (
      I => NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN3,
      O => NlwInverterSignal_VGA_CS_OBUF_D2_PT_0_IN3
    );
  NlwInverterBlock_VGA_CS_OBUF_D2_PT_0_IN4 : X_INV
    port map (
      I => NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN4,
      O => NlwInverterSignal_VGA_CS_OBUF_D2_PT_0_IN4
    );
  NlwInverterBlock_VGA_CS_OBUF_D2_PT_0_IN5 : X_INV
    port map (
      I => NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN5,
      O => NlwInverterSignal_VGA_CS_OBUF_D2_PT_0_IN5
    );
  NlwInverterBlock_VGA_CS_OBUF_D2_PT_0_IN6 : X_INV
    port map (
      I => NlwBufferSignal_VGA_CS_OBUF_D2_PT_0_IN6,
      O => NlwInverterSignal_VGA_CS_OBUF_D2_PT_0_IN6
    );
  NlwInverterBlock_VGA_CS_OBUF_D2_PT_1_IN0 : X_INV
    port map (
      I => NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN0,
      O => NlwInverterSignal_VGA_CS_OBUF_D2_PT_1_IN0
    );
  NlwInverterBlock_VGA_CS_OBUF_D2_PT_1_IN1 : X_INV
    port map (
      I => NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN1,
      O => NlwInverterSignal_VGA_CS_OBUF_D2_PT_1_IN1
    );
  NlwInverterBlock_VGA_CS_OBUF_D2_PT_1_IN3 : X_INV
    port map (
      I => NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN3,
      O => NlwInverterSignal_VGA_CS_OBUF_D2_PT_1_IN3
    );
  NlwInverterBlock_VGA_CS_OBUF_D2_PT_1_IN4 : X_INV
    port map (
      I => NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN4,
      O => NlwInverterSignal_VGA_CS_OBUF_D2_PT_1_IN4
    );
  NlwInverterBlock_VGA_CS_OBUF_D2_PT_1_IN5 : X_INV
    port map (
      I => NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN5,
      O => NlwInverterSignal_VGA_CS_OBUF_D2_PT_1_IN5
    );
  NlwInverterBlock_VGA_CS_OBUF_D2_PT_1_IN6 : X_INV
    port map (
      I => NlwBufferSignal_VGA_CS_OBUF_D2_PT_1_IN6,
      O => NlwInverterSignal_VGA_CS_OBUF_D2_PT_1_IN6
    );
  NlwInverterBlock_VGA_CS_OBUF_D2_PT_2_IN0 : X_INV
    port map (
      I => NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN0,
      O => NlwInverterSignal_VGA_CS_OBUF_D2_PT_2_IN0
    );
  NlwInverterBlock_VGA_CS_OBUF_D2_PT_2_IN1 : X_INV
    port map (
      I => NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN1,
      O => NlwInverterSignal_VGA_CS_OBUF_D2_PT_2_IN1
    );
  NlwInverterBlock_VGA_CS_OBUF_D2_PT_2_IN3 : X_INV
    port map (
      I => NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN3,
      O => NlwInverterSignal_VGA_CS_OBUF_D2_PT_2_IN3
    );
  NlwInverterBlock_VGA_CS_OBUF_D2_PT_2_IN4 : X_INV
    port map (
      I => NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN4,
      O => NlwInverterSignal_VGA_CS_OBUF_D2_PT_2_IN4
    );
  NlwInverterBlock_VGA_CS_OBUF_D2_PT_2_IN5 : X_INV
    port map (
      I => NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN5,
      O => NlwInverterSignal_VGA_CS_OBUF_D2_PT_2_IN5
    );
  NlwInverterBlock_VGA_CS_OBUF_D2_PT_2_IN6 : X_INV
    port map (
      I => NlwBufferSignal_VGA_CS_OBUF_D2_PT_2_IN6,
      O => NlwInverterSignal_VGA_CS_OBUF_D2_PT_2_IN6
    );
  NlwInverterBlock_nRW0_OBUF_D2_PT_1_IN0 : X_INV
    port map (
      I => NlwBufferSignal_nRW0_OBUF_D2_PT_1_IN0,
      O => NlwInverterSignal_nRW0_OBUF_D2_PT_1_IN0
    );
  NlwInverterBlock_nRW0_OBUF_D2_PT_1_IN1 : X_INV
    port map (
      I => NlwBufferSignal_nRW0_OBUF_D2_PT_1_IN1,
      O => NlwInverterSignal_nRW0_OBUF_D2_PT_1_IN1
    );
  NlwInverterBlock_nRW0_OBUF_D2_PT_3_IN1 : X_INV
    port map (
      I => NlwBufferSignal_nRW0_OBUF_D2_PT_3_IN1,
      O => NlwInverterSignal_nRW0_OBUF_D2_PT_3_IN1
    );
  NlwInverterBlock_nRW1_OBUF_D2_PT_1_IN0 : X_INV
    port map (
      I => NlwBufferSignal_nRW1_OBUF_D2_PT_1_IN0,
      O => NlwInverterSignal_nRW1_OBUF_D2_PT_1_IN0
    );
  NlwInverterBlock_nRW1_OBUF_D2_PT_1_IN1 : X_INV
    port map (
      I => NlwBufferSignal_nRW1_OBUF_D2_PT_1_IN1,
      O => NlwInverterSignal_nRW1_OBUF_D2_PT_1_IN1
    );
  NlwInverterBlock_nRW1_OBUF_D2_PT_3_IN0 : X_INV
    port map (
      I => NlwBufferSignal_nRW1_OBUF_D2_PT_3_IN0,
      O => NlwInverterSignal_nRW1_OBUF_D2_PT_3_IN0
    );
  NlwInverterBlock_nRW1_OBUF_D2_PT_3_IN1 : X_INV
    port map (
      I => NlwBufferSignal_nRW1_OBUF_D2_PT_3_IN1,
      O => NlwInverterSignal_nRW1_OBUF_D2_PT_3_IN1
    );
  NlwInverterBlock_READ_n_OBUF_D2_IN0 : X_INV
    port map (
      I => NlwBufferSignal_READ_n_OBUF_D2_IN0,
      O => NlwInverterSignal_READ_n_OBUF_D2_IN0
    );
  NlwInverterBlock_READ_n_OBUF_D2_IN1 : X_INV
    port map (
      I => NlwBufferSignal_READ_n_OBUF_D2_IN1,
      O => NlwInverterSignal_READ_n_OBUF_D2_IN1
    );
  NlwInverterBlock_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN0 : X_INV
    port map (
      I => NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN0,
      O => NlwInverterSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN0
    );
  NlwInverterBlock_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN1 : X_INV
    port map (
      I => NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN1,
      O => NlwInverterSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN1
    );
  NlwInverterBlock_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN2 : X_INV
    port map (
      I => NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN2,
      O => NlwInverterSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN2
    );
  NlwInverterBlock_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN4 : X_INV
    port map (
      I => NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN4,
      O => NlwInverterSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN4
    );
  NlwInverterBlock_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN5 : X_INV
    port map (
      I => NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN5,
      O => NlwInverterSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN5
    );
  NlwInverterBlock_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN6 : X_INV
    port map (
      I => NlwBufferSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN6,
      O => NlwInverterSignal_I_MEMW_OBUF_BUF1_EXP_tsimrenamed_net_IN6
    );
  NlwBlockROC : X_ROC
    generic map (ROC_WIDTH => 100 ns)
    port map (O => PRLD);

end Structure;


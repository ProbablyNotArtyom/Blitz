<?xml version="1.0" encoding="UTF-8"?>
<drawing version="7">
    <attr value="xc9500xl" name="DeviceFamilyName">
        <trait delete="all:0" />
        <trait editname="all:0" />
        <trait edittrait="all:0" />
    </attr>
    <netlist>
        <signal name="AS" />
        <signal name="a19" />
        <signal name="a20" />
        <signal name="a21" />
        <signal name="XLXN_140" />
        <signal name="XLXN_141" />
        <signal name="XLXN_142" />
        <signal name="XLXN_143" />
        <signal name="XLXN_144" />
        <signal name="XLXN_145" />
        <signal name="XLXN_146" />
        <signal name="XLXN_147" />
        <signal name="a26" />
        <signal name="a27" />
        <signal name="XLXN_174" />
        <signal name="XLXN_176" />
        <signal name="XLXN_177" />
        <signal name="ROM" />
        <signal name="FLOPPY" />
        <signal name="IDE" />
        <signal name="KBD_CS" />
        <signal name="VGA_CS" />
        <signal name="ETHRNT" />
        <signal name="XLXN_212" />
        <signal name="DRAM_CS" />
        <signal name="XLXN_250" />
        <signal name="XLXN_254" />
        <signal name="RW0" />
        <signal name="RW1" />
        <signal name="RW_N" />
        <signal name="XLXN_232" />
        <signal name="nRW0" />
        <signal name="nRW1" />
        <signal name="IO_CS" />
        <signal name="FC1" />
        <signal name="FC2" />
        <signal name="FC0" />
        <signal name="a16" />
        <signal name="FPU" />
        <signal name="XLXN_299" />
        <signal name="XLXN_309" />
        <signal name="XLXN_312" />
        <signal name="isa_mem_active" />
        <signal name="XLXN_318" />
        <port polarity="Input" name="AS" />
        <port polarity="Input" name="a19" />
        <port polarity="Input" name="a20" />
        <port polarity="Input" name="a21" />
        <port polarity="Input" name="a26" />
        <port polarity="Input" name="a27" />
        <port polarity="Output" name="ROM" />
        <port polarity="Output" name="FLOPPY" />
        <port polarity="Output" name="IDE" />
        <port polarity="Output" name="KBD_CS" />
        <port polarity="Output" name="VGA_CS" />
        <port polarity="Output" name="ETHRNT" />
        <port polarity="Output" name="DRAM_CS" />
        <port polarity="Output" name="RW0" />
        <port polarity="Output" name="RW1" />
        <port polarity="Input" name="RW_N" />
        <port polarity="Output" name="nRW0" />
        <port polarity="Output" name="nRW1" />
        <port polarity="Output" name="IO_CS" />
        <port polarity="Input" name="FC1" />
        <port polarity="Input" name="FC2" />
        <port polarity="Input" name="FC0" />
        <port polarity="Input" name="a16" />
        <port polarity="Output" name="FPU" />
        <port polarity="Output" name="isa_mem_active" />
        <blockdef name="nand3">
            <timestamp>2000-1-1T10:10:10</timestamp>
            <line x2="64" y1="-64" y2="-64" x1="0" />
            <line x2="64" y1="-128" y2="-128" x1="0" />
            <line x2="64" y1="-192" y2="-192" x1="0" />
            <line x2="216" y1="-128" y2="-128" x1="256" />
            <circle r="12" cx="204" cy="-128" />
            <line x2="144" y1="-176" y2="-176" x1="64" />
            <line x2="64" y1="-80" y2="-80" x1="144" />
            <arc ex="144" ey="-176" sx="144" sy="-80" r="48" cx="144" cy="-128" />
            <line x2="64" y1="-64" y2="-192" x1="64" />
        </blockdef>
        <blockdef name="or2">
            <timestamp>2000-1-1T10:10:10</timestamp>
            <line x2="64" y1="-64" y2="-64" x1="0" />
            <line x2="64" y1="-128" y2="-128" x1="0" />
            <line x2="192" y1="-96" y2="-96" x1="256" />
            <arc ex="192" ey="-96" sx="112" sy="-48" r="88" cx="116" cy="-136" />
            <arc ex="48" ey="-144" sx="48" sy="-48" r="56" cx="16" cy="-96" />
            <line x2="48" y1="-144" y2="-144" x1="112" />
            <arc ex="112" ey="-144" sx="192" sy="-96" r="88" cx="116" cy="-56" />
            <line x2="48" y1="-48" y2="-48" x1="112" />
        </blockdef>
        <blockdef name="d3_8e">
            <timestamp>2000-1-1T10:10:10</timestamp>
            <line x2="64" y1="-576" y2="-576" x1="0" />
            <line x2="64" y1="-512" y2="-512" x1="0" />
            <line x2="64" y1="-448" y2="-448" x1="0" />
            <line x2="320" y1="-576" y2="-576" x1="384" />
            <line x2="320" y1="-512" y2="-512" x1="384" />
            <line x2="320" y1="-448" y2="-448" x1="384" />
            <line x2="320" y1="-384" y2="-384" x1="384" />
            <line x2="320" y1="-320" y2="-320" x1="384" />
            <line x2="320" y1="-256" y2="-256" x1="384" />
            <line x2="320" y1="-192" y2="-192" x1="384" />
            <line x2="320" y1="-128" y2="-128" x1="384" />
            <rect width="256" x="64" y="-640" height="576" />
            <line x2="64" y1="-128" y2="-128" x1="0" />
        </blockdef>
        <blockdef name="inv">
            <timestamp>2000-1-1T10:10:10</timestamp>
            <line x2="64" y1="-32" y2="-32" x1="0" />
            <line x2="160" y1="-32" y2="-32" x1="224" />
            <line x2="128" y1="-64" y2="-32" x1="64" />
            <line x2="64" y1="-32" y2="0" x1="128" />
            <line x2="64" y1="0" y2="-64" x1="64" />
            <circle r="16" cx="144" cy="-32" />
        </blockdef>
        <blockdef name="d2_4e">
            <timestamp>2000-1-1T10:10:10</timestamp>
            <rect width="256" x="64" y="-384" height="320" />
            <line x2="64" y1="-128" y2="-128" x1="0" />
            <line x2="64" y1="-256" y2="-256" x1="0" />
            <line x2="64" y1="-320" y2="-320" x1="0" />
            <line x2="320" y1="-128" y2="-128" x1="384" />
            <line x2="320" y1="-192" y2="-192" x1="384" />
            <line x2="320" y1="-256" y2="-256" x1="384" />
            <line x2="320" y1="-320" y2="-320" x1="384" />
        </blockdef>
        <blockdef name="nand2">
            <timestamp>2000-1-1T10:10:10</timestamp>
            <line x2="64" y1="-64" y2="-64" x1="0" />
            <line x2="64" y1="-128" y2="-128" x1="0" />
            <line x2="216" y1="-96" y2="-96" x1="256" />
            <circle r="12" cx="204" cy="-96" />
            <line x2="64" y1="-48" y2="-144" x1="64" />
            <line x2="144" y1="-144" y2="-144" x1="64" />
            <line x2="64" y1="-48" y2="-48" x1="144" />
            <arc ex="144" ey="-144" sx="144" sy="-48" r="48" cx="144" cy="-96" />
        </blockdef>
        <blockdef name="and2">
            <timestamp>2000-1-1T10:10:10</timestamp>
            <line x2="64" y1="-64" y2="-64" x1="0" />
            <line x2="64" y1="-128" y2="-128" x1="0" />
            <line x2="192" y1="-96" y2="-96" x1="256" />
            <arc ex="144" ey="-144" sx="144" sy="-48" r="48" cx="144" cy="-96" />
            <line x2="64" y1="-48" y2="-48" x1="144" />
            <line x2="144" y1="-144" y2="-144" x1="64" />
            <line x2="64" y1="-48" y2="-144" x1="64" />
        </blockdef>
        <blockdef name="or3">
            <timestamp>2000-1-1T10:10:10</timestamp>
            <line x2="48" y1="-64" y2="-64" x1="0" />
            <line x2="72" y1="-128" y2="-128" x1="0" />
            <line x2="48" y1="-192" y2="-192" x1="0" />
            <line x2="192" y1="-128" y2="-128" x1="256" />
            <arc ex="192" ey="-128" sx="112" sy="-80" r="88" cx="116" cy="-168" />
            <arc ex="48" ey="-176" sx="48" sy="-80" r="56" cx="16" cy="-128" />
            <line x2="48" y1="-64" y2="-80" x1="48" />
            <line x2="48" y1="-192" y2="-176" x1="48" />
            <line x2="48" y1="-80" y2="-80" x1="112" />
            <arc ex="112" ey="-176" sx="192" sy="-128" r="88" cx="116" cy="-88" />
            <line x2="48" y1="-176" y2="-176" x1="112" />
        </blockdef>
        <block symbolname="d3_8e" name="XLXI_10">
            <attr value="INV" name="VhdlModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="VeriModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="XILINX" name="Level">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <blockpin signalname="a19" name="A0" />
            <blockpin signalname="a20" name="A1" />
            <blockpin signalname="a21" name="A2" />
            <blockpin signalname="XLXN_174" name="E" />
            <blockpin signalname="XLXN_140" name="D0" />
            <blockpin signalname="XLXN_141" name="D1" />
            <blockpin signalname="XLXN_142" name="D2" />
            <blockpin signalname="XLXN_143" name="D3" />
            <blockpin signalname="XLXN_144" name="D4" />
            <blockpin signalname="XLXN_145" name="D5" />
            <blockpin signalname="XLXN_146" name="D6" />
            <blockpin signalname="XLXN_147" name="D7" />
        </block>
        <block symbolname="or3" name="XLXI_67">
            <attr value="INV" name="VhdlModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="VeriModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="Device">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <blockpin signalname="XLXN_299" name="I0" />
            <blockpin signalname="AS" name="I1" />
            <blockpin signalname="a16" name="I2" />
            <blockpin signalname="FPU" name="O" />
        </block>
        <block symbolname="nand3" name="XLXI_74">
            <attr value="INV" name="VhdlModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="VeriModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="Device">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <blockpin signalname="FC2" name="I0" />
            <blockpin signalname="FC1" name="I1" />
            <blockpin signalname="FC0" name="I2" />
            <blockpin signalname="XLXN_299" name="O" />
        </block>
        <block symbolname="d2_4e" name="XLXI_75">
            <attr value="INV" name="VhdlModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="VeriModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="XILINX" name="Level">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <blockpin signalname="a26" name="A0" />
            <blockpin signalname="a27" name="A1" />
            <blockpin signalname="XLXN_212" name="E" />
            <blockpin signalname="XLXN_174" name="D0" />
            <blockpin signalname="XLXN_254" name="D1" />
            <blockpin signalname="XLXN_176" name="D2" />
            <blockpin signalname="XLXN_177" name="D3" />
        </block>
        <block symbolname="and2" name="XLXI_76">
            <attr value="INV" name="VhdlModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="VeriModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="Device">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <blockpin signalname="IDE" name="I0" />
            <blockpin signalname="FLOPPY" name="I1" />
            <blockpin signalname="IO_CS" name="O" />
        </block>
        <block symbolname="nand2" name="XLXI_77">
            <attr value="INV" name="VhdlModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="VeriModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="Device">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <blockpin signalname="XLXN_254" name="I0" />
            <blockpin signalname="a21" name="I1" />
            <blockpin signalname="XLXN_312" name="O" />
        </block>
        <block symbolname="nand2" name="XLXI_79">
            <attr value="INV" name="VhdlModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="VeriModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="Device">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <blockpin signalname="XLXN_254" name="I0" />
            <blockpin signalname="XLXN_250" name="I1" />
            <blockpin signalname="XLXN_232" name="O" />
        </block>
        <block symbolname="or2" name="XLXI_80">
            <attr value="INV" name="VhdlModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="VeriModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="Device">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <blockpin signalname="XLXN_232" name="I0" />
            <blockpin signalname="XLXN_309" name="I1" />
            <blockpin signalname="nRW1" name="O" />
        </block>
        <block symbolname="or2" name="XLXI_82">
            <attr value="INV" name="VhdlModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="VeriModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="Device">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <blockpin signalname="XLXN_232" name="I0" />
            <blockpin signalname="RW_N" name="I1" />
            <blockpin signalname="RW1" name="O" />
        </block>
        <block symbolname="or2" name="XLXI_83">
            <attr value="INV" name="VhdlModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="VeriModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="Device">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <blockpin signalname="XLXN_312" name="I0" />
            <blockpin signalname="XLXN_309" name="I1" />
            <blockpin signalname="nRW0" name="O" />
        </block>
        <block symbolname="or2" name="XLXI_84">
            <attr value="INV" name="VhdlModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="VeriModel">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <attr value="INV" name="Device">
                <trait verilog="all:0 wsynop:1 wsynth:1" />
                <trait vhdl="all:0 wa:1 wd:1" />
            </attr>
            <blockpin signalname="XLXN_312" name="I0" />
            <blockpin signalname="RW_N" name="I1" />
            <blockpin signalname="RW0" name="O" />
        </block>
        <block symbolname="inv" name="XLXI_85">
            <blockpin signalname="XLXN_140" name="I" />
            <blockpin signalname="ROM" name="O" />
        </block>
        <block symbolname="inv" name="XLXI_86">
            <blockpin signalname="XLXN_141" name="I" />
            <blockpin signalname="FLOPPY" name="O" />
        </block>
        <block symbolname="inv" name="XLXI_87">
            <blockpin signalname="XLXN_142" name="I" />
            <blockpin signalname="IDE" name="O" />
        </block>
        <block symbolname="inv" name="XLXI_88">
            <blockpin signalname="XLXN_143" name="I" />
            <blockpin signalname="KBD_CS" name="O" />
        </block>
        <block symbolname="inv" name="XLXI_89">
            <blockpin signalname="XLXN_144" name="I" />
            <blockpin signalname="VGA_CS" name="O" />
        </block>
        <block symbolname="inv" name="XLXI_90">
            <blockpin signalname="XLXN_145" name="I" />
            <blockpin signalname="ETHRNT" name="O" />
        </block>
        <block symbolname="inv" name="XLXI_91">
            <blockpin signalname="AS" name="I" />
            <blockpin signalname="XLXN_212" name="O" />
        </block>
        <block symbolname="inv" name="XLXI_92">
            <blockpin signalname="XLXN_176" name="I" />
            <blockpin signalname="DRAM_CS" name="O" />
        </block>
        <block symbolname="inv" name="XLXI_93">
            <blockpin signalname="a21" name="I" />
            <blockpin signalname="XLXN_250" name="O" />
        </block>
        <block symbolname="inv" name="XLXI_94">
            <blockpin signalname="FPU" name="I" />
            <blockpin signalname="isa_mem_active" name="O" />
        </block>
        <block symbolname="inv" name="XLXI_95">
            <blockpin signalname="RW_N" name="I" />
            <blockpin signalname="XLXN_309" name="O" />
        </block>
    </netlist>
    <sheet sheetnum="1" width="5440" height="3520">
        <branch name="a19">
            <wire x2="640" y1="320" y2="320" x1="480" />
        </branch>
        <iomarker fontsize="28" x="480" y="320" name="a19" orien="R180" />
        <branch name="a21">
            <wire x2="640" y1="448" y2="448" x1="480" />
        </branch>
        <branch name="a20">
            <wire x2="640" y1="384" y2="384" x1="480" />
        </branch>
        <iomarker fontsize="28" x="480" y="448" name="a21" orien="R180" />
        <iomarker fontsize="28" x="480" y="384" name="a20" orien="R180" />
        <branch name="XLXN_140">
            <wire x2="1056" y1="320" y2="320" x1="1024" />
        </branch>
        <branch name="XLXN_141">
            <wire x2="1056" y1="384" y2="384" x1="1024" />
        </branch>
        <branch name="XLXN_142">
            <wire x2="1056" y1="448" y2="448" x1="1024" />
        </branch>
        <branch name="XLXN_143">
            <wire x2="1056" y1="512" y2="512" x1="1024" />
        </branch>
        <branch name="XLXN_144">
            <wire x2="1056" y1="576" y2="576" x1="1024" />
        </branch>
        <branch name="XLXN_145">
            <wire x2="1056" y1="640" y2="640" x1="1024" />
        </branch>
        <branch name="XLXN_146">
            <wire x2="1056" y1="704" y2="704" x1="1024" />
        </branch>
        <branch name="XLXN_147">
            <wire x2="1056" y1="768" y2="768" x1="1024" />
        </branch>
        <instance x="640" y="896" name="XLXI_10" orien="R0" />
        <iomarker fontsize="28" x="480" y="1040" name="a26" orien="R180" />
        <iomarker fontsize="28" x="480" y="1104" name="a27" orien="R180" />
        <branch name="XLXN_174">
            <wire x2="608" y1="768" y2="880" x1="608" />
            <wire x2="1296" y1="880" y2="880" x1="608" />
            <wire x2="1296" y1="880" y2="1040" x1="1296" />
            <wire x2="640" y1="768" y2="768" x1="608" />
            <wire x2="1296" y1="1040" y2="1040" x1="1024" />
        </branch>
        <branch name="XLXN_176">
            <wire x2="1056" y1="1168" y2="1168" x1="1024" />
        </branch>
        <branch name="ROM">
            <wire x2="1440" y1="320" y2="320" x1="1280" />
        </branch>
        <iomarker fontsize="28" x="1440" y="320" name="ROM" orien="R0" />
        <branch name="FLOPPY">
            <wire x2="1392" y1="384" y2="384" x1="1280" />
            <wire x2="1440" y1="384" y2="384" x1="1392" />
            <wire x2="1392" y1="384" y2="416" x1="1392" />
            <wire x2="1648" y1="416" y2="416" x1="1392" />
        </branch>
        <iomarker fontsize="28" x="1440" y="384" name="FLOPPY" orien="R0" />
        <branch name="IDE">
            <wire x2="1392" y1="448" y2="448" x1="1280" />
            <wire x2="1440" y1="448" y2="448" x1="1392" />
            <wire x2="1392" y1="448" y2="480" x1="1392" />
            <wire x2="1648" y1="480" y2="480" x1="1392" />
        </branch>
        <iomarker fontsize="28" x="1440" y="448" name="IDE" orien="R0" />
        <branch name="KBD_CS">
            <wire x2="1440" y1="512" y2="512" x1="1280" />
        </branch>
        <iomarker fontsize="28" x="1440" y="512" name="KBD_CS" orien="R0" />
        <branch name="VGA_CS">
            <wire x2="1440" y1="576" y2="576" x1="1280" />
        </branch>
        <iomarker fontsize="28" x="1440" y="576" name="VGA_CS" orien="R0" />
        <branch name="ETHRNT">
            <wire x2="1440" y1="640" y2="640" x1="1280" />
        </branch>
        <iomarker fontsize="28" x="1440" y="640" name="ETHRNT" orien="R0" />
        <branch name="AS">
            <wire x2="384" y1="1232" y2="1232" x1="368" />
        </branch>
        <branch name="DRAM_CS">
            <wire x2="1328" y1="1168" y2="1168" x1="1280" />
        </branch>
        <iomarker fontsize="28" x="1328" y="1168" name="DRAM_CS" orien="R0" />
        <branch name="a21">
            <attrtext style="alignment:SOFT-RIGHT;fontsize:28;fontname:Arial" attrname="Name" x="1600" y="1040" type="branch" />
            <wire x2="1648" y1="1040" y2="1040" x1="1600" />
            <wire x2="1680" y1="1040" y2="1040" x1="1648" />
            <wire x2="1648" y1="1040" y2="1200" x1="1648" />
            <wire x2="1664" y1="1200" y2="1200" x1="1648" />
        </branch>
        <branch name="XLXN_250">
            <wire x2="1904" y1="1200" y2="1200" x1="1888" />
        </branch>
        <branch name="XLXN_254">
            <wire x2="1616" y1="1104" y2="1104" x1="1024" />
            <wire x2="1680" y1="1104" y2="1104" x1="1616" />
            <wire x2="1616" y1="1104" y2="1264" x1="1616" />
            <wire x2="1904" y1="1264" y2="1264" x1="1616" />
        </branch>
        <branch name="RW0">
            <wire x2="2672" y1="1424" y2="1424" x1="2592" />
        </branch>
        <branch name="RW1">
            <wire x2="2672" y1="1664" y2="1664" x1="2592" />
        </branch>
        <branch name="XLXN_232">
            <wire x2="2288" y1="1232" y2="1232" x1="2160" />
            <wire x2="2288" y1="1232" y2="1696" x1="2288" />
            <wire x2="2288" y1="1696" y2="1824" x1="2288" />
            <wire x2="2480" y1="1824" y2="1824" x1="2288" />
            <wire x2="2336" y1="1696" y2="1696" x1="2288" />
        </branch>
        <branch name="nRW0">
            <wire x2="2800" y1="1552" y2="1552" x1="2720" />
        </branch>
        <branch name="nRW1">
            <wire x2="2816" y1="1792" y2="1792" x1="2736" />
        </branch>
        <iomarker fontsize="28" x="2672" y="1424" name="RW0" orien="R0" />
        <iomarker fontsize="28" x="2672" y="1664" name="RW1" orien="R0" />
        <iomarker fontsize="28" x="2800" y="1552" name="nRW0" orien="R0" />
        <iomarker fontsize="28" x="2816" y="1792" name="nRW1" orien="R0" />
        <branch name="XLXN_212">
            <wire x2="640" y1="1232" y2="1232" x1="608" />
        </branch>
        <branch name="a27">
            <wire x2="640" y1="1104" y2="1104" x1="480" />
        </branch>
        <branch name="a26">
            <wire x2="640" y1="1040" y2="1040" x1="480" />
        </branch>
        <branch name="XLXN_177">
            <wire x2="1056" y1="1232" y2="1232" x1="1024" />
        </branch>
        <branch name="IO_CS">
            <wire x2="1936" y1="448" y2="448" x1="1904" />
        </branch>
        <iomarker fontsize="28" x="1936" y="448" name="IO_CS" orien="R0" />
        <branch name="FC1">
            <wire x2="672" y1="1680" y2="1680" x1="512" />
        </branch>
        <branch name="FC2">
            <wire x2="672" y1="1744" y2="1744" x1="512" />
        </branch>
        <branch name="FC0">
            <wire x2="672" y1="1616" y2="1616" x1="512" />
        </branch>
        <instance x="928" y="1648" name="XLXI_67" orien="R0" />
        <branch name="a16">
            <wire x2="928" y1="1456" y2="1456" x1="768" />
        </branch>
        <branch name="AS">
            <attrtext style="alignment:SOFT-RIGHT;fontsize:28;fontname:Arial" attrname="Name" x="912" y="1520" type="branch" />
            <wire x2="928" y1="1520" y2="1520" x1="912" />
        </branch>
        <branch name="FPU">
            <wire x2="1152" y1="1680" y2="1680" x1="1136" />
            <wire x2="1216" y1="1680" y2="1680" x1="1152" />
            <wire x2="1136" y1="1680" y2="1936" x1="1136" />
            <wire x2="1152" y1="1936" y2="1936" x1="1136" />
            <wire x2="1216" y1="1520" y2="1520" x1="1184" />
            <wire x2="1232" y1="1520" y2="1520" x1="1216" />
            <wire x2="1216" y1="1520" y2="1680" x1="1216" />
        </branch>
        <branch name="XLXN_299">
            <wire x2="928" y1="1584" y2="1680" x1="928" />
        </branch>
        <iomarker fontsize="28" x="512" y="1680" name="FC1" orien="R180" />
        <iomarker fontsize="28" x="512" y="1744" name="FC2" orien="R180" />
        <iomarker fontsize="28" x="512" y="1616" name="FC0" orien="R180" />
        <iomarker fontsize="28" x="768" y="1456" name="a16" orien="R180" />
        <iomarker fontsize="28" x="1232" y="1520" name="FPU" orien="R0" />
        <iomarker fontsize="28" x="368" y="1232" name="AS" orien="R180" />
        <branch name="RW_N">
            <wire x2="1984" y1="1392" y2="1392" x1="1888" />
            <wire x2="1984" y1="1392" y2="1520" x1="1984" />
            <wire x2="1984" y1="1520" y2="1632" x1="1984" />
            <wire x2="2087" y1="1632" y2="1632" x1="1984" />
            <wire x2="2159" y1="1632" y2="1632" x1="2087" />
            <wire x2="2240" y1="1632" y2="1632" x1="2159" />
            <wire x2="2336" y1="1632" y2="1632" x1="2240" />
            <wire x2="2000" y1="1520" y2="1520" x1="1984" />
            <wire x2="2336" y1="1392" y2="1392" x1="1984" />
        </branch>
        <branch name="XLXN_312">
            <wire x2="2256" y1="1072" y2="1072" x1="1936" />
            <wire x2="2256" y1="1072" y2="1456" x1="2256" />
            <wire x2="2336" y1="1456" y2="1456" x1="2256" />
            <wire x2="2256" y1="1456" y2="1584" x1="2256" />
            <wire x2="2464" y1="1584" y2="1584" x1="2256" />
        </branch>
        <branch name="XLXN_309">
            <wire x2="2240" y1="1520" y2="1520" x1="2224" />
            <wire x2="2464" y1="1520" y2="1520" x1="2240" />
            <wire x2="2240" y1="1520" y2="1760" x1="2240" />
            <wire x2="2480" y1="1760" y2="1760" x1="2240" />
        </branch>
        <iomarker fontsize="28" x="1504" y="1936" name="isa_mem_active" orien="R0" />
        <branch name="isa_mem_active">
            <wire x2="1488" y1="1936" y2="1936" x1="1376" />
            <wire x2="1504" y1="1936" y2="1936" x1="1488" />
        </branch>
        <instance x="672" y="1808" name="XLXI_74" orien="R0" />
        <instance x="640" y="1360" name="XLXI_75" orien="R0" />
        <instance x="1648" y="544" name="XLXI_76" orien="R0" />
        <instance x="1680" y="1168" name="XLXI_77" orien="R0" />
        <instance x="1904" y="1328" name="XLXI_79" orien="R0" />
        <instance x="2480" y="1888" name="XLXI_80" orien="R0" />
        <instance x="2336" y="1760" name="XLXI_82" orien="R0" />
        <instance x="2464" y="1648" name="XLXI_83" orien="R0" />
        <instance x="2336" y="1520" name="XLXI_84" orien="R0" />
        <iomarker fontsize="28" x="1888" y="1392" name="RW_N" orien="R180" />
        <instance x="1056" y="352" name="XLXI_85" orien="R0" />
        <instance x="1056" y="416" name="XLXI_86" orien="R0" />
        <instance x="1056" y="480" name="XLXI_87" orien="R0" />
        <instance x="1056" y="544" name="XLXI_88" orien="R0" />
        <instance x="1056" y="608" name="XLXI_89" orien="R0" />
        <instance x="1056" y="672" name="XLXI_90" orien="R0" />
        <instance x="384" y="1264" name="XLXI_91" orien="R0" />
        <instance x="1056" y="1200" name="XLXI_92" orien="R0" />
        <instance x="1664" y="1232" name="XLXI_93" orien="R0" />
        <instance x="1152" y="1968" name="XLXI_94" orien="R0" />
        <instance x="2000" y="1552" name="XLXI_95" orien="R0" />
    </sheet>
</drawing>
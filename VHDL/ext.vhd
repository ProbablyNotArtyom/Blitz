
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity flipflop_d is
	port(
		d,clk	: in std_logic;
		q,nq	: out std_logic;
		reset	: in std_logic	
	);
end flipflop_d;

architecture Behavioral of flipflop_d is
begin
	process(clk, reset)
	begin
		if (reset = '0') then
			q <= '0';
			nq <= '1';
		elsif (rising_edge(clk)) then
			q <= d;
			nq <= not d;
		end if;
	end process;
end Behavioral;
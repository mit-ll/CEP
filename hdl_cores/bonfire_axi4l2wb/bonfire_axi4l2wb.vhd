----------------------------------------------------------------------------------
-- Module Name:    bonfire_axi4l2wb - Behavioral

-- The Bonfire Processor Project, (c) 2016,2017 Thomas Hornschuh

-- Simple AXI4 Lite Slave to Wishbone  master bridge

-- License: See LICENSE or LICENSE.txt File in git project root.
--
-- Generics:
--     ADRWIDTH  : integer := 15; 
--     Width of the AXI Address Bus, the Wishbone Adr- Bus coresponds with it, but without the lowest adress bits
--     FAST_READ_TERM : boolean := TRUE 
--      When TRUE it  allows AXI read termination in same cycle as wb_ack_i is raised (in case RRREADY is asserted)
--      which leads to a AXI4 read cycle latency of two clocks in best case
--      when FALSE the Wishbone data are always registered and the AXI4 cycle is terminated earliest one clock after
--      wb_ack_i. This leads to a minimum AXI4 read latency of three cycles but saves a 32 Bit Mux and
--      avoids a long combinatorical data path

----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.all;
library work;

entity bonfire_axi4l2wb is

generic (
    ADRWIDTH  : integer := 15; -- Width of the AXI Address Bus, the Wishbone Adr- Bus coresponds with it, but without the lowest adress bits
    FAST_READ_TERM : boolean := TRUE -- TRUE: Allows AXI read termination in same cycle as 
    );

  port (
    ---------------------------------------------------------------------------
    -- AXI Interface
    ---------------------------------------------------------------------------
    -- Clock and Reset
    S_AXI_ACLK    : in  std_logic;
    S_AXI_ARESETN : in  std_logic;
    -- Write Address Channel
    S_AXI_AWADDR  : in  std_logic_vector(ADRWIDTH-1 downto 0);
    S_AXI_AWVALID : in  std_logic;
    S_AXI_AWREADY : out std_logic;
    -- Write Data Channel
    S_AXI_WDATA   : in  std_logic_vector(31 downto 0);
    S_AXI_WSTRB   : in  std_logic_vector(3 downto 0);
    S_AXI_WVALID  : in  std_logic;
    S_AXI_WREADY  : out std_logic;
    -- Read Address Channel
    S_AXI_ARADDR  : in  std_logic_vector(ADRWIDTH-1 downto 0);
    S_AXI_ARVALID : in  std_logic;
    S_AXI_ARREADY : out std_logic;
    -- Read Data Channel
    S_AXI_RDATA   : out std_logic_vector(31 downto 0);
    S_AXI_RRESP   : out std_logic_vector(1 downto 0);
    S_AXI_RVALID  : out std_logic;
    S_AXI_RREADY  : in  std_logic;
    -- Write Response Channel
    S_AXI_BRESP   : out std_logic_vector(1 downto 0);
    S_AXI_BVALID  : out std_logic;
    S_AXI_BREADY  : in  std_logic;

    -- Wishbone interface
    wb_clk_o      : out std_logic;
    wb_rst_o      : out std_logic;

    wb_addr_o     : out std_logic_vector(ADRWIDTH-1 downto 0);
    wb_dat_o      : out std_logic_vector(31 downto 0);
    wb_we_o       : out std_logic;
    wb_sel_o      : out std_logic_vector(3 downto 0);
    wb_stb_o      : out std_logic;
    wb_cyc_o      : out std_logic;

    wb_dat_i      : in  std_logic_vector(31 downto 0);
    wb_ack_i      : in  std_logic
    );

end bonfire_axi4l2wb;


architecture Behavioral of bonfire_axi4l2wb is

-- Attribute Infos for Xilinx Vivado IP Integrator Block designs
-- Should not have negative influence on other platforms. 

ATTRIBUTE X_INTERFACE_INFO : STRING;
ATTRIBUTE X_INTERFACE_INFO of  wb_clk_o : SIGNAL is "xilinx.com:signal:clock:1.0 wb_clk_o CLK";
--ATTRIBUTE X_INTERFACE_INFO of  rst_i : SIGNAL is "xilinx.com:signal:reset:1.0 rst_i RESET";

ATTRIBUTE X_INTERFACE_PARAMETER : STRING;
ATTRIBUTE X_INTERFACE_PARAMETER of wb_clk_o : SIGNAL is "ASSOCIATED_BUSIF WB_MASTER";
--ATTRIBUTE X_INTERFACE_PARAMETER of rst_i : SIGNAL is "ASSOCIATED_BUSIF WB_DB";

ATTRIBUTE X_INTERFACE_INFO OF wb_cyc_o: SIGNAL IS "bonfire.eu:wb:Wishbone_master:1.0 WB_MASTER wb_dbus_cyc_o";
ATTRIBUTE X_INTERFACE_INFO OF wb_stb_o: SIGNAL IS "bonfire.eu:wb:Wishbone_master:1.0 WB_MASTER wb_dbus_stb_o";
ATTRIBUTE X_INTERFACE_INFO OF wb_we_o: SIGNAL IS "bonfire.eu:wb:Wishbone_master:1.0  WB_MASTER wb_dbus_we_o";
ATTRIBUTE X_INTERFACE_INFO OF wb_ack_i: SIGNAL IS "bonfire.eu:wb:Wishbone_master:1.0 WB_MASTER wb_dbus_ack_i";
ATTRIBUTE X_INTERFACE_INFO OF wb_addr_o: SIGNAL IS "bonfire.eu:wb:Wishbone_master:1.0 WB_MASTER wb_dbus_adr_o";
ATTRIBUTE X_INTERFACE_INFO OF wb_dat_o: SIGNAL IS "bonfire.eu:wb:Wishbone_master:1.0 WB_MASTER wb_dbus_dat_o";
ATTRIBUTE X_INTERFACE_INFO OF wb_dat_i: SIGNAL IS "bonfire.eu:wb:Wishbone_master:1.0 WB_MASTER wb_dbus_dat_i";
ATTRIBUTE X_INTERFACE_INFO OF wb_sel_o: SIGNAL IS "bonfire.eu:wb:Wishbone_master:1.0 WB_MASTER wb_dbus_sel_o";


signal ar_taken : std_logic:='0';
signal aw_taken : std_logic:='0';

--signal wb_address : std_logic_vector(wb_addr_o'range);
signal axi_wr_adr : std_logic_vector(wb_addr_o'range);
signal axi_rd_adr : std_logic_vector(wb_addr_o'range);

signal we : std_logic := '0';
signal stb : std_logic := '0';
signal sel : std_logic_vector(3 downto 0) := (others=>'0');

signal read_buffer : std_logic_vector(wb_dat_o'range);
signal axi_wready : std_logic;

type t_state is (s_idle,s_read,s_write,s_write_response,s_write_finish,s_read_finish);
signal state : t_state := s_idle;

begin

   wb_clk_o <= S_AXI_ACLK;
   wb_rst_o <= not S_AXI_ARESETN;

   wb_addr_o <= axi_wr_adr when stb='1' and we='1' else
                axi_rd_adr when stb='1' and we='0' else (others=>'X');

   wb_stb_o <= stb;
   wb_cyc_o <= stb;
   wb_sel_o <= sel;
   wb_we_o <=  we;

   S_AXI_BRESP <= "00";
   S_AXI_RRESP <= "00";


    S_AXI_AWREADY <= not aw_taken;
    S_AXI_ARREADY <= not ar_taken;

    achannels: process(S_AXI_ACLK)
    begin
       if rising_edge(S_AXI_ACLK) then
         if S_AXI_ARESETN='0' then
            aw_taken <= '0';
            ar_taken <= '0';
         elsif stb='1' then
           if wb_ack_i='1' then
             if we='1' then
               aw_taken<='0';
             else
               ar_taken<='0';
             end if;
           end if;
         else
           if  S_AXI_AWVALID='1' and aw_taken='0' then
             aw_taken <= '1';
             axi_wr_adr <= S_AXI_AWADDR(wb_addr_o'high downto wb_addr_o'low);
           end if;

           if S_AXI_ARVALID='1' and ar_taken='0' then
             ar_taken <= '1';
             axi_rd_adr <= S_AXI_ARADDR(wb_addr_o'high downto wb_addr_o'low);
           end if;
         end if;
       end if;

    end process;


  axi_wready <= '1' when aw_taken='1' and (state=s_idle or state=s_read_finish)
                  else '0';


  S_AXI_WREADY <= axi_wready;


  read_response: process(state,wb_ack_i,S_AXI_RREADY,wb_dat_i,read_buffer)
  begin

    S_AXI_RVALID <= '0';
    S_AXI_RDATA <= (others=>'X'); -- don't care...
   
   if  FAST_READ_TERM and state=s_read and wb_ack_i='1' then
      S_AXI_RVALID <= '1';
      S_AXI_RDATA <= wb_dat_i;
   elsif state=s_read_finish then
      S_AXI_RVALID <= '1';
      S_AXI_RDATA <= read_buffer;
  end if;
   

  end process;



  -- AXI State machine
  process(S_AXI_ACLK)
  
    procedure process_adr_phase is
    begin
      S_AXI_BVALID <= '0';
     -- process AXI address phase
     if  S_AXI_ARVALID='1' or ar_taken='1' then
       stb <= '1';
       we <= '0';
       sel <= (others=> '1');
       state <= s_read;

     elsif aw_taken='1' then
       if S_AXI_WVALID='1' then
         sel <= S_AXI_WSTRB;
         wb_dat_o <= S_AXI_WDATA;
         stb <= '1';
         we <= '1';
         state <= s_write_response;
       else
         -- when no valid write channel yet, wait for it
         state <= s_write;
       end if;
     else
       state <= s_idle;  
     end if;
    end procedure; 
  
  begin
    if rising_edge(S_AXI_ACLK) then
       if S_AXI_ARESETN='0' then
         state <= s_idle;
         we <= '0';
         stb <= '0';
         sel <= (others=>'0');

         S_AXI_BVALID <= '0';
       else
         case state is
           when s_idle =>
             process_adr_phase;

           when s_write => -- process "delayed" write channel
             if S_AXI_WVALID='1' then
               sel <= S_AXI_WSTRB;
               wb_dat_o <= S_AXI_WDATA;
               stb <= '1';
               we <= '1';
               state <= s_write_response;
             end if;
           when s_write_response =>
             if wb_ack_i = '1' then
               S_AXI_BVALID <= '1';
               if S_AXI_BREADY='1' then
                 state <= s_idle;
               else
                 state <= s_write_finish;
               end if;
               stb <= '0';
               we <= '0';
            end if;
           when  s_write_finish =>
             if S_AXI_BREADY='1' then
               S_AXI_BVALID <= '0';
               state <= s_idle;
             end if;

           when s_read =>
             if wb_ack_i='1' then
               stb <= '0';
               -- when AXI side is ready  
               --we can terminate AXI cycle now
               if FAST_READ_TERM and S_AXI_RREADY='1' then
                 state <= s_idle;
               else
                 -- otherwise we must wait
                 state <= s_read_finish;
                 read_buffer <= wb_dat_i;
               end if;
             end if;
           when s_read_finish =>
             if S_AXI_RREADY='1' then
               process_adr_phase;  
             end if;
         end case;
       end if;
    end if;

  end process;


end Behavioral;

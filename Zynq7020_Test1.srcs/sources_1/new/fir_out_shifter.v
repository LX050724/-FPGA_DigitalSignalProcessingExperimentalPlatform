`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2022/03/30 12:28:11
// Design Name: 
// Module Name: fir_out_shifter
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module fir_out_shifter#(
	parameter IN_WIDTH = 8,
	parameter OUT_WIDTH = 8
	)(
	input aclk,
    input [$clog2(IN_WIDTH)-1:0] shift,

    input [IN_WIDTH-1:0] s_axis_tdata,
    input s_axis_tvalid,
    input s_axis_tlast,
    output s_axis_tready,

    output [OUT_WIDTH-1:0] m_axis_tdata,
    output m_axis_tvalid,
    input m_axis_tready
    );

parameter MAX = 2 ** OUT_WIDTH / 2 - 1;
parameter MIN = -MAX - 1;

reg [$clog2(IN_WIDTH)-1:0] shift_reg;

always @(posedge aclk) begin
	shift_reg <= shift;
end

assign m_axis_tvalid = s_axis_tvalid;
assign s_axis_tready = m_axis_tready;

function [IN_WIDTH-1:0] limit;
    input [IN_WIDTH-1:0] in;
    begin
        limit = $signed(in) > MAX ? MAX :
        		$signed(in) < MIN ? MIN : in;
    end
endfunction

assign m_axis_tdata = limit(($signed(s_axis_tdata)) >>> shift_reg);

endmodule

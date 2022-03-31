`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2022/03/29 10:05:33
// Design Name: 
// Module Name: AXI4_Stream_Switch_1_2
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


module AXI4_Stream_Switch_1_2#(parameter WIDTH = 8)(
    input switch,
    input aclk,
    input aresten,

    input [WIDTH-1:0] s_axis_tdata,
    input [WIDTH/8-1:0] s_axis_tkeep,
    input s_axis_tvalid,
    input s_axis_tlast,
    output s_axis_tready,

    output [WIDTH-1:0] m_axis_0_tdata,
    output [WIDTH+1/8-1:0] m_axis_0_tkeep,
    output m_axis_0_tvalid,
    output m_axis_0_tlast,
    input m_axis_0_tready,

    output [WIDTH-1:0] m_axis_1_tdata,
    output [WIDTH/8-1:0] m_axis_1_tkeep,
    output m_axis_1_tvalid,
    output m_axis_1_tlast,
    input m_axis_1_tready
    );

    reg switch_reg;
    
    always@(posedge aclk) begin
        if (!aresten) 
            switch_reg <= 0;
        else
            switch_reg <= switch;
    end
    

    assign s_axis_tready = switch_reg ? m_axis_1_tready : m_axis_0_tready;

    assign m_axis_0_tdata = switch_reg ? 0 : s_axis_tdata;
    assign m_axis_1_tdata = switch_reg ? s_axis_tdata : 0;

    assign m_axis_0_tvalid = switch_reg ? 0 : s_axis_tvalid;
    assign m_axis_1_tvalid = switch_reg ? s_axis_tvalid : 0;

    assign m_axis_0_tlast = switch_reg ? 0 : s_axis_tlast;
    assign m_axis_1_tlast = switch_reg ? s_axis_tlast : 0;

    assign m_axis_0_tkeep = switch_reg ? 0 : s_axis_tkeep;
    assign m_axis_1_tkeep = switch_reg ? s_axis_tkeep : 0;
endmodule

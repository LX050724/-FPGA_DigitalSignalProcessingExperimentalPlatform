`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2022/01/03 15:05:18
// Design Name: 
// Module Name: AXI4_Stream_Switch
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


module AXI4_Stream_Switch#(parameter WIDTH = 8)(
    input switch,
    input aclk,
    input aresten,
    
    input [WIDTH-1:0] s_axis_0_tdata,
    input s_axis_0_tvalid,
    input s_axis_0_tlast,
    output s_axis_0_tready,
    
    input [WIDTH-1:0] s_axis_1_tdata,
    input s_axis_1_tvalid,
    input s_axis_1_tlast,
    output s_axis_1_tready,
    
    output [WIDTH-1:0] m_axis_tdata,
    output m_axis_tvalid,
    output  m_axis_tlast,
    input m_axis_tready
    );
    
    reg switch_reg;
    
    always@(posedge aclk) begin
        if (!aresten) 
            switch_reg <= 0;
        else
            switch_reg <= switch;
    end
    
    assign m_axis_tdata = switch_reg ? s_axis_1_tdata : s_axis_0_tdata;
    assign m_axis_tvalid = switch_reg ? s_axis_1_tvalid : s_axis_0_tvalid;
    assign s_axis_0_tready = switch_reg ? 0 : m_axis_tready;
    assign s_axis_1_tready = switch_reg ? m_axis_tready : 0;
    assign m_axis_tlast = switch_reg ? s_axis_1_tlast : s_axis_0_tlast;
    
endmodule

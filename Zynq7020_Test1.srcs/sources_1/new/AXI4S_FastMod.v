`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company:
// Engineer:
//
// Create Date: 2022/01/09 09:54:28
// Design Name:
// Module Name: AXI4S_FastMod
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


module AXI4S_FastMod#(
parameter WIDTH = 8,
parameter WIDTH_TUSER = 8
)(
    input aclk,
    input aresetn,
    input [WIDTH-1:0] s_axis_tdata,
    input [WIDTH_TUSER-1:0] s_axis_tuser,
    input s_axis_tvalid,
    output s_axis_tready,
    input s_axis_tlast,
    
    output [WIDTH-1:0] m_axis_tdata,
    output [WIDTH_TUSER-1:0] m_axis_tuser,
    output m_axis_tvalid,
    input m_axis_tready,
    output m_axis_tlast
    );
    
    function [WIDTH-1:0] max;
        input [WIDTH-1:0] a, b;
        begin
            max = a > b ? a : b;
        end
    endfunction
    
    function [WIDTH/2-1:0] abs;
        input [WIDTH/2-1:0] a;
        begin
            abs = a[WIDTH/2-1] ? 1 + (~a) : a;
        end
    endfunction
    
    wire [WIDTH-1:0] IM_abs = abs(s_axis_tdata[WIDTH-1:WIDTH/2]);
    wire [WIDTH-1:0] RE_abs = abs(s_axis_tdata[WIDTH/2-1:0]);
    
    reg [WIDTH-1:0] x_max, x_min;
    reg [WIDTH-1:0] t1, t2, t3;
    reg [WIDTH-1:0] mod;
    reg tlast_delay[2:0];
    reg [WIDTH_TUSER-1:0] tuser_delay[2:0];

    assign m_axis_tdata = mod[WIDTH-1] ? 0 : mod;
    assign s_axis_tready = m_axis_tready;
    assign m_axis_tvalid = s_axis_tvalid;
    assign m_axis_tlast = tlast_delay[2];
    assign m_axis_tuser = tuser_delay[2];

    always@(posedge aclk)
    begin
        if (!aresetn) begin
            x_max <= 0;
            x_min <= 0;
            t1    <= 0;
            t2    <= 0;
            t3    <= 0;
            mod   <= 0;
        end
        else if (s_axis_tready && m_axis_tvalid) begin
            if (IM_abs > RE_abs) begin
                x_max <= IM_abs;
                x_min <= RE_abs;
            end
            else begin
                x_max <= RE_abs;
                x_min <= IM_abs;
            end
            t1  <= x_max;
            t2  <= (x_max + (x_max >> 5) - 1) + ((x_min >> 2) + (x_min >> 5));
            t3  <= ((x_max >> 1) + (x_max >> 2) + (x_max >> 4)) + ((x_min >> 1) + (x_min >> 4) + (x_min >> 5));
            mod <= max(max(t1, t2), t3);
        end
    end
    
    integer i = 0;
    
    always@(posedge aclk)
    begin
        if (!aresetn) begin
            for (i = 0; i < 2; i = i + 1) begin
                tlast_delay[i] <= 0;
                tuser_delay[i] <= 0;
            end
        end
        else if (s_axis_tready && m_axis_tvalid) begin
            tlast_delay[0] <= s_axis_tlast;
            tuser_delay[0] <= s_axis_tuser;
            for (i = 1; i < 3; i = i + 1) begin
                tlast_delay[i] <= tlast_delay[i - 1];
                tuser_delay[i] <= tuser_delay[i - 1];
            end
        end
    end
endmodule

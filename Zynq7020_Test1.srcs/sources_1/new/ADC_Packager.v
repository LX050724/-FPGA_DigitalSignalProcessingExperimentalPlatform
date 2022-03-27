`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2022/01/23 12:55:10
// Design Name: 
// Module Name: ADC_Packager
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


module ADC_Packager#( 
    parameter WIDTH = 8,
    parameter COUNT = 8192
) (
    input aclk,
    input aresetn,

    input start,

    input [WIDTH-1:0] s_axis_tdata,
    input s_axis_tvalid,
    output s_axis_tready,

    output [WIDTH-1:0] m_axis_tdata,
    output m_axis_tvalid,
    output m_axis_tlast
    );
    
    assign s_axis_tready = 1;
    
    parameter IDLE = 0;
    parameter RUNNING = 1;
    parameter WAIT_START_DOWN = 2;
    
    reg [WIDTH-1:0] tdata;
    reg tvalid;
    reg tlast;

    reg [15:0] cnt;
    reg [ 1:0] status;
    
    always@(posedge aclk) begin
        if (!aresetn) begin
            cnt <= 0;
            status <= IDLE;
            tlast <= 0;
        end else begin
            status <= status;
            case (status)
                IDLE:
                    if (start) begin
                        status <= RUNNING;
                    end
                RUNNING:
                    if (cnt < COUNT - 1) begin
                        cnt <= cnt + (s_axis_tvalid ? 1 : 0);
                    end else begin
                        tlast <= 1;
                        status <= WAIT_START_DOWN;
                        cnt <= 0;
                    end
                WAIT_START_DOWN:
                    if (!start) begin
                        status <= IDLE;
                    end
            endcase
            if (tlast) tlast <= 0;
        end
    end
    
    
    always @(posedge aclk) begin
        if (!aresetn) begin
            tdata <= 0;
            tvalid <= 0;
        end else begin
            if (status == RUNNING) begin
                tdata <= s_axis_tdata;
                tvalid <= s_axis_tvalid;
            end else begin
                tdata <= 0;
                tvalid <= 0;
            end
        end
    end
    
    assign m_axis_tdata = tdata;
    assign m_axis_tlast = tlast;
    assign m_axis_tvalid = tvalid;
    
endmodule

`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/12/24 13:01:26
// Design Name: 
// Module Name: AXI4Stream_2_DAC
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


module AXI4Stream_2_DAC(
    input  [7:0] Offset,
    (* X_INTERFACE_INFO = "xilinx.com:user:DAC_Interface:1.0 DAC_Port DAC_Data" *)
    output [7:0] DAC_Data,
    (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 DAC_CLK CLK" *)
    (* X_INTERFACE_PARAMETER = "ASSOCIATED_BUSIF DAC_Port, ASSOCIATED_RESET aresetn" *)
    output DAC_CLK,
    (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 aclk CLK" *)
    (* X_INTERFACE_PARAMETER = "ASSOCIATED_BUSIF S_AXIS, ASSOCIATED_RESET aresetn" *)
    input aclk,
    (* X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 aresetn RST" *)
    (* X_INTERFACE_PARAMETER = "POLARITY ACTIVE_LOW" *)
    input aresetn,
    (* X_INTERFACE_INFO = "xilinx.com:interface:axis:1.0 S_AXIS TDATA" *)
    input [7:0] S_AXIS_tdata,
    (* X_INTERFACE_INFO = "xilinx.com:interface:axis:1.0 S_AXIS TVALID" *)
    input S_AXIS_tvalid,
    (* X_INTERFACE_INFO = "xilinx.com:interface:axis:1.0 S_AXIS TREADY" *)
    output  S_AXIS_tready
    );
    
    reg [7:0] data;
    
    wire en = aresetn & S_AXIS_tvalid;
    assign S_AXIS_tready = 1;
    assign DAC_CLK = en ? aclk : 1'd0;
    assign DAC_Data = en ? data : 8'd0;
    
    always@(posedge aclk) begin
        if (!aresetn)
            data <= 8'd0;
        else
            data <= Offset - S_AXIS_tdata;
    end
endmodule

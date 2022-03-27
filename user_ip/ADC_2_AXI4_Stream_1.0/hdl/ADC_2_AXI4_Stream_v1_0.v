`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/12/22 16:42:51
// Design Name: 
// Module Name: ADC_AXI4Stream
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


module ADC_2_AXI4_Stream#( parameter PACKAGE_SIZE = 8192 )(
    (* X_INTERFACE_INFO = "xilinx.com:user:ADC_Interface:1.0 ADC_Port ADC_Data" *)
    input [7:0] ADC_Data,
    input [7:0] Offset,
    (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 ADC_CLK CLK" *)
    (* X_INTERFACE_PARAMETER = "ASSOCIATED_BUSIF ADC_Port, ASSOCIATED_RESET aresetn" *)
    output ADC_CLK,
    (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 aclk CLK" *)
    (* X_INTERFACE_PARAMETER = "ASSOCIATED_BUSIF M_AXIS, ASSOCIATED_RESET aresetn" *)
    input aclk,
    (* X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 aresetn RST" *)
    (* X_INTERFACE_PARAMETER = "POLARITY ACTIVE_LOW" *)
    input aresetn,
    (* X_INTERFACE_INFO = "xilinx.com:interface:axis:1.0 M_AXIS TDATA" *)
    output [7:0] m_axis_tdata,
    (* X_INTERFACE_INFO = "xilinx.com:interface:axis:1.0 M_AXIS TVALID" *)
    output m_axis_tvalid,
    (* X_INTERFACE_INFO = "xilinx.com:interface:axis:1.0 M_AXIS TREADY" *)
    input  m_axis_tready,
    (* X_INTERFACE_INFO = "xilinx.com:interface:axis:1.0 M_AXIS TLAST" *)
    output m_axis_tlast
    );
    
    reg [7:0] ADC_Data_reg;
    reg [15:0] cnt;
    
    assign m_axis_tdata  = m_axis_tready ? ADC_Data_reg : 8'd0;
    assign ADC_CLK = aclk;
    assign m_axis_tvalid = aresetn;
    assign m_axis_tlast = (cnt == (PACKAGE_SIZE  - 1)) ? 1'b1 : 1'b0;
    
    always@(posedge aclk) begin
        if (!aresetn) begin
            ADC_Data_reg <= 0;
        end else begin
            ADC_Data_reg <= ADC_Data - Offset;
        end
    end
    
    always@(posedge aclk) begin
        if (!aresetn) begin
            cnt <= 0;
        end else begin
            if (cnt == (PACKAGE_SIZE  - 1))
                cnt <= 16'd0;
            else
                cnt <= cnt + 16'd1;
        end
    end
    
endmodule

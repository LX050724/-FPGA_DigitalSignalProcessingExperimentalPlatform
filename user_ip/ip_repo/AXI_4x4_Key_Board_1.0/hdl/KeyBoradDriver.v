`timescale 1 ns / 1 ps

//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/12/19 17:43:56
// Design Name: 
// Module Name: KeyBoradDriver
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


module KeyBoradDriver(
    output [3:0] Row,
    input  [3:0] Col,
    output [15:0] Key,
    input CLK,
    input RSTn
    );
    
    reg [3:0] Row_reg;
    reg [15:0] Key_reg;
    reg [1:0] scan;
    
    assign Key = Key_reg;
    assign Row = Row_reg;
    
    always@(negedge CLK or negedge RSTn) begin
        if (!RSTn) begin
            scan <= 2'd0;
            Row_reg <= 4'd1;
        end else begin
            case (scan)
                2'd0: Row_reg <= 4'b0001;
                2'd1: Row_reg <= 4'b0010;
                2'd2: Row_reg <= 4'b0100;
                2'd3: Row_reg <= 4'b1000;
            endcase
            scan <= scan + 2'd1;
        end
    end
    
    always@(posedge CLK or negedge RSTn) begin
        if (!RSTn) begin
            Key_reg <= 16'd0;
        end else begin
            case (scan)
                2'd0: Key_reg[ 3: 0] <= Col;
                2'd1: Key_reg[ 7: 4] <= Col;
                2'd2: Key_reg[11: 8] <= Col;
                2'd3: Key_reg[15:12] <= Col;
            endcase
        end
    end
    
endmodule



module FFT_Packager#(
    parameter WIDTH = 8
) (
    input aclk,
    input aresetn,

    input dma_tready,

    input [WIDTH-1:0] s_axis_tdata,
    input s_axis_tvalid,
    output s_axis_tready,
    input s_axis_tlast,

    output [WIDTH-1:0] m_axis_tdata,
    output m_axis_tvalid,
    output m_axis_tlast
);

assign s_axis_tready = 1;

parameter IDLE = 0;
parameter WAIT_FFT_HEAD = 1;
parameter RUNNING = 2;
parameter WAIT_TREADY_DOWN = 3;

reg [1:0] status;

reg [WIDTH-1:0] tdata;
reg tvalid;
reg tlast;

always @(posedge aclk) begin
    if (!aresetn) begin
        status <= IDLE;
    end else begin
        status <= status;
        case (status)
            IDLE: 
                if (dma_tready == 1) begin
                    status <= WAIT_FFT_HEAD;
                end
            WAIT_FFT_HEAD:
                if (s_axis_tlast == 1) begin
                    status <= RUNNING;
                end
            RUNNING:
                if (s_axis_tlast == 1) begin
                    status <= WAIT_TREADY_DOWN;
                end
            WAIT_TREADY_DOWN:
                if (dma_tready == 0) begin
                    status <= IDLE;
                end
        endcase
    end
end

always @(posedge aclk) begin
    if (!aresetn) begin
        tdata <= 0;
        tvalid <= 0;
        tlast <= 0;
    end else begin
        if (status == RUNNING) begin
            tdata <= s_axis_tdata;
            tvalid <= s_axis_tvalid;
            tlast <= s_axis_tlast;
        end else begin
            tdata <= 0;
            tvalid <= 0;
            tlast <= 0;
        end
    end
end

assign m_axis_tdata = tdata;
assign m_axis_tlast = tlast;
assign m_axis_tvalid = tvalid;
    
endmodule
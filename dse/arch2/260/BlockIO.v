// Module Name: BlockIO
// Module Type: MODULE_CELL 
// Include Module: config_cell
// Input Ports: in0 in1
// Output Ports: out0
// Special Ports: clk reset config_clk config_reset config_in config_out
// Config Width:


module BlockIO(clk, reset, config_clk, config_reset, config_in, config_out, in0, in1, out0);
    parameter size = 32;

    input clk, reset, config_clk, config_reset, config_in;
    output config_out;
    input [size-1:0] in0, in1;
    output reg [size-1:0] out0;

    wire mux_sel;
    reg [size-1:0] in;

    config_cell #1 config_cell_1(
        .config_clk(config_clk), 
        .config_reset(config_reset), 
        .config_in(config_in), 
        .config_out(config_out), 
        .config_sig(mux_sel)
    );

    always @*
        case (mux_sel)
            0 : in = in0;
            1 : in = in1;
        endcase

    always @(posedge clk, posedge reset)
        if (reset == 1)
            out0 <= 0;
        else
            out0 <= in;

endmodule


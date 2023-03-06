// Module Name: MEM
// Module Type: FUNC_CELL
// Include Module:
// Input Ports: in0 in1
// Output Ports: out0
// Special Ports: clk reset config_sig
// Config Width: 1

module MEM(clk, reset, config_sig, in0, in1, out0);
    parameter size = 32;
    // Specifying the ports
    input clk, reset, config_sig;
    input [size-1:0] in0, in1;
    output reg [size-1:0] out0;

    always @(posedge clk, posedge reset)
        if (reset == 1)
            out0 <= 0;
        else if (config_sig == 0)
            out0 <= in0;
        else
            out0 <= in1;
endmodule


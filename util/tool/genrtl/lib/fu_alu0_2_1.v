// Module Name: fu_alu0_2_1
// Module Type: FUNC_CELL
// Include Module: 
// Input Ports: in0 in1
// Output Ports: out0
// Special Ports: clk config_sig
// Config Width: 2

module fu_alu0_2_1(clk, config_sig, in0, in1, out0);
    parameter size = 32;
    // Specifying the ports
    input clk;
    input [size-1:0] in0;
    input [size-1:0] in1;
    output reg [size-1:0] out0;
    input [1:0] config_sig;

    // Declaring wires to direct module output into multiplexer
    wire [size-1:0] add_sel;
    wire [size-1:0] sub_sel;

    // Declaring the submodules
    assign add_sel = in0 + in1;
    assign sub_sel = in0 - in1;

    always @(posedge clk)
        case (config_sig)
            0: out0 = add_sel;
            1: out0 = sub_sel;
            2: out0 = in0;
            3: out0 = in1;
            default: out0 = 0;
        endcase
endmodule


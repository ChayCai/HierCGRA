// Module Name: fu_alu_2_1
// Module Type: FUNC_CELL
// Include Module: 
// Input Ports: in0 in1
// Output Ports: out0
// Special Ports: clk config_sig
// Config Width: 4

module fu_alu_2_1(clk, config_sig, in0, in1, out0);
    parameter size = 32;
    // Specifying the ports
    input clk;
    input [size-1:0] in0;
    input [size-1:0] in1;
    output reg [size-1:0] out0;
    input [3:0] config_sig;

    // Declaring wires to direct module output into multiplexer
    wire [size-1:0] add_sel;
    wire [size-1:0] sub_sel;
    wire [size-1:0] mul_sel;
    wire [size-1:0] and_sel;
    wire [size-1:0] or_sel;
    wire [size-1:0] xor_sel;
    wire [size-1:0] shl_sel;
    wire [size-1:0] shr_sel;

    // Declaring the submodules
    assign add_sel = in0 + in1;
    assign sub_sel = in0 - in1;
    assign mul_sel = in0 * in1;
    assign and_sel = in0 & in1;
    assign or_sel = in0 | in1;
    assign xor_sel = in0 ^ in1;
    assign shl_sel = in0 << in1;
    assign shr_sel = in0 >> in1;

    always @(posedge clk)
        case (config_sig)
            0: out0 = add_sel;
            1: out0 = sub_sel;
            2: out0 = mul_sel;
            3: out0 = and_sel;
            4: out0 = or_sel;
            5: out0 = xor_sel;
            6: out0 = shl_sel;
            7: out0 = shr_sel;
            8: out0 = in0;
            9: out0 = in1;
            default: out0 = 0;
        endcase
endmodule


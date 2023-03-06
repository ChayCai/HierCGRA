// Module Name: FULLYCONN_4X1
// Module Type: SWITCH_CELL
// Include Module: config_cell
// Input Ports: in0 in1 in2 in3
// Output Ports: out0
// Special Ports: config_clk config_reset config_in config_out
// Config Width: 


module FULLYCONN_4X1(config_clk, config_reset, config_in, config_out, in0, in1, in2, in3, out0);
	parameter size = 32;

	input config_clk, config_reset, config_in;
	output config_out;
	input [size-1:0] in0, in1, in2, in3;
	output reg [size-1:0] out0;

	wire [1:0] mux_sel_0;

	config_cell #2 config_cell_0(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_in),
		.config_out(config_out),
		.config_sig(mux_sel_0)
	);

	always @(*)
		case (mux_sel_0)
			0: out0 = in0;
			1: out0 = in1;
			2: out0 = in2;
			3: out0 = in3;
			default: out0 = 0;
		endcase


endmodule

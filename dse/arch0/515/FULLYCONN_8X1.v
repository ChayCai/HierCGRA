// Module Name: FULLYCONN_8X1
// Module Type: SWITCH_CELL
// Include Module: config_cell
// Input Ports: in0 in1 in2 in3 in4 in5 in6 in7
// Output Ports: out0
// Special Ports: config_clk config_reset config_in config_out
// Config Width: 


module FULLYCONN_8X1(config_clk, config_reset, config_in, config_out, in0, in1, in2, in3, in4, in5, in6, in7, out0);
	parameter size = 32;

	input config_clk, config_reset, config_in;
	output config_out;
	input [size-1:0] in0, in1, in2, in3, in4, in5, in6, in7;
	output reg [size-1:0] out0;

	wire [2:0] mux_sel_0;

	config_cell #3 config_cell_0(
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
			4: out0 = in4;
			5: out0 = in5;
			6: out0 = in6;
			7: out0 = in7;
			default: out0 = 0;
		endcase


endmodule

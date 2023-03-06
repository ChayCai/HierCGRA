// Module Name: FULLYCONN_16X1
// Module Type: SWITCH_CELL
// Include Module: config_cell
// Input Ports: in0 in1 in2 in3 in4 in5 in6 in7 in8 in9 in10 in11 in12 in13 in14 in15
// Output Ports: out0
// Special Ports: config_clk config_reset config_in config_out
// Config Width: 


module FULLYCONN_16X1(config_clk, config_reset, config_in, config_out, in0, in1, in2, in3, in4, in5, in6, in7, in8, in9, in10, in11, in12, in13, in14, in15, out0);
	parameter size = 32;

	input config_clk, config_reset, config_in;
	output config_out;
	input [size-1:0] in0, in1, in2, in3, in4, in5, in6, in7, in8, in9, in10, in11, in12, in13, in14, in15;
	output reg [size-1:0] out0;

	wire [3:0] mux_sel_0;

	config_cell #4 config_cell_0(
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
			8: out0 = in8;
			9: out0 = in9;
			10: out0 = in10;
			11: out0 = in11;
			12: out0 = in12;
			13: out0 = in13;
			14: out0 = in14;
			15: out0 = in15;
			default: out0 = 0;
		endcase


endmodule

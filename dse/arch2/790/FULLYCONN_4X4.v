// Module Name: FULLYCONN_4X4
// Module Type: SWITCH_CELL
// Include Module: config_cell
// Input Ports: in0 in1 in2 in3
// Output Ports: out0 out1 out2 out3
// Special Ports: config_clk config_reset config_in config_out
// Config Width: 


module FULLYCONN_4X4(config_clk, config_reset, config_in, config_out, in0, in1, in2, in3, out0, out1, out2, out3);
	parameter size = 32;

	input config_clk, config_reset, config_in;
	output config_out;
	input [size-1:0] in0, in1, in2, in3;
	output reg [size-1:0] out0, out1, out2, out3;

	wire [1:0] mux_sel_0;
	wire [1:0] mux_sel_1;
	wire [1:0] mux_sel_2;
	wire [1:0] mux_sel_3;
	wire config_wire_1;
	wire config_wire_2;
	wire config_wire_3;

	config_cell #2 config_cell_0(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_in),
		.config_out(config_wire_1),
		.config_sig(mux_sel_0)
	);

	config_cell #2 config_cell_1(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_1),
		.config_out(config_wire_2),
		.config_sig(mux_sel_1)
	);

	config_cell #2 config_cell_2(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_2),
		.config_out(config_wire_3),
		.config_sig(mux_sel_2)
	);

	config_cell #2 config_cell_3(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_3),
		.config_out(config_out),
		.config_sig(mux_sel_3)
	);

	always @(*)
		case (mux_sel_0)
			0: out0 = in0;
			1: out0 = in1;
			2: out0 = in2;
			3: out0 = in3;
			default: out0 = 0;
		endcase

	always @(*)
		case (mux_sel_1)
			0: out1 = in0;
			1: out1 = in1;
			2: out1 = in2;
			3: out1 = in3;
			default: out1 = 0;
		endcase

	always @(*)
		case (mux_sel_2)
			0: out2 = in0;
			1: out2 = in1;
			2: out2 = in2;
			3: out2 = in3;
			default: out2 = 0;
		endcase

	always @(*)
		case (mux_sel_3)
			0: out3 = in0;
			1: out3 = in1;
			2: out3 = in2;
			3: out3 = in3;
			default: out3 = 0;
		endcase


endmodule

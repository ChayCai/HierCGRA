// Module Name: FULLYCONN_1X2
// Module Type: SWITCH_CELL
// Include Module: config_cell
// Input Ports: in0
// Output Ports: out0 out1
// Special Ports: config_clk config_reset config_in config_out
// Config Width: 


module FULLYCONN_1X2(config_clk, config_reset, config_in, config_out, in0, out0, out1);
	parameter size = 32;

	input config_clk, config_reset, config_in;
	output config_out;
	input [size-1:0] in0;
	output reg [size-1:0] out0, out1;

	wire mux_sel_0;
	wire mux_sel_1;
	wire config_wire_1;

	config_cell #1 config_cell_0(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_in),
		.config_out(config_wire_1),
		.config_sig(mux_sel_0)
	);

	config_cell #1 config_cell_1(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_1),
		.config_out(config_out),
		.config_sig(mux_sel_1)
	);

	always @(*)
		case (mux_sel_0)
			0: out0 = in0;
			default: out0 = 0;
		endcase

	always @(*)
		case (mux_sel_1)
			0: out1 = in0;
			default: out1 = 0;
		endcase


endmodule
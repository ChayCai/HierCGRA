// Module Name: BlockPE2
// Module Type: MODULE_CELL
// Include Module: FULLYCONN_2X1 FULLYCONN_4X4 MEM ALU1
// Input Ports: in0 in1
// Output Ports: out0
// Special Ports: reset config_clk config_reset config_out config_in clk
// Config Width: 


module BlockPE2(reset, config_clk, config_reset, config_out, config_in, clk, in0, in1, out0);
	parameter size = 32;

	input reset, config_clk, config_reset, config_in, clk;
	output config_out;
	input [size-1:0] in0, in1;
	output [size-1:0] out0;

	wire [3:0] config_sig_0;
	wire config_sig_1;
	wire config_wire_0;
	wire config_wire_1;
	wire config_wire_2;
	wire [size-1:0] SW_INPORT_FU_out2;
	wire [size-1:0] SW_INPORT_FU_out1;
	wire [size-1:0] in0;
	wire [size-1:0] ALU0_out0;
	wire [size-1:0] in1;
	wire [size-1:0] SW_INPORT_FU_out0;
	wire [size-1:0] SW_INPORT_FU_out3;
	wire [size-1:0] MEM0_out0;
	wire [size-1:0] SW_FU_OUTPORT_out0;

	config_cell #4 config_cell_0(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_in),
		.config_out(config_wire_0),
		.config_sig(config_sig_0)
	);

	config_cell #1 config_cell_1(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_0),
		.config_out(config_wire_1),
		.config_sig(config_sig_1)
	);

	ALU1 ALU0(
		.clk(clk),
		.config_sig(config_sig_0),
		.in0(SW_INPORT_FU_out0),
		.in1(SW_INPORT_FU_out1),
		.out0(ALU0_out0)
	);

	MEM MEM0(
		.clk(clk),
		.reset(reset),
		.config_sig(config_sig_1),
		.in0(SW_INPORT_FU_out2),
		.in1(SW_INPORT_FU_out3),
		.out0(MEM0_out0)
	);

	FULLYCONN_2X1 SW_FU_OUTPORT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_1),
		.config_out(config_wire_2),
		.in0(ALU0_out0),
		.in1(MEM0_out0),
		.out0(SW_FU_OUTPORT_out0)
	);

	FULLYCONN_4X4 SW_INPORT_FU(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_2),
		.config_out(config_out),
		.in0(in0),
		.in1(in1),
		.in2(ALU0_out0),
		.in3(MEM0_out0),
		.out0(SW_INPORT_FU_out0),
		.out1(SW_INPORT_FU_out1),
		.out2(SW_INPORT_FU_out2),
		.out3(SW_INPORT_FU_out3)
	);

	assign out0 = SW_FU_OUTPORT_out0;


endmodule

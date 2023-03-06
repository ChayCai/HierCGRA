// Module Name: Core
// Module Type: MODULE_CELL
// Include Module: FULLYCONN_8X1 FULLYCONN_10X1 BlockPE7 FULLYCONN_7X1 BlockPE6 BlockPE5 BlockIO BlockPE8 BlockPE1 BlockMEM BlockPE0 BlockPE11 BlockPE9 BlockPE13 FULLYCONN_1X1 BlockPE2 BlockPE4 BlockPE10 BlockPE14 BlockPE12 BlockPE15 FULLYCONN_9X1 BlockPE3
// Input Ports: in0 in1 in2 in3 in4 in5 in6 in7
// Output Ports: out0 out1 out2 out3 out4 out5 out6 out7
// Special Ports: clk reset config_clk config_reset config_in config_out
// Config Width: 


module Core(clk, reset, config_clk, config_reset, config_in, config_out, in0, in1, in2, in3, in4, in5, in6, in7, out0, out1, out2, out3, out4, out5, out6, out7);
	parameter size = 32;

	input clk, reset, config_clk, config_reset, config_in;
	output config_out;
	input [size-1:0] in0, in1, in2, in3, in4, in5, in6, in7;
	output [size-1:0] out0, out1, out2, out3, out4, out5, out6, out7;

	wire config_wire_0;
	wire config_wire_1;
	wire config_wire_2;
	wire config_wire_3;
	wire config_wire_4;
	wire config_wire_5;
	wire config_wire_6;
	wire config_wire_7;
	wire config_wire_8;
	wire config_wire_9;
	wire config_wire_10;
	wire config_wire_11;
	wire config_wire_12;
	wire config_wire_13;
	wire config_wire_14;
	wire config_wire_15;
	wire config_wire_16;
	wire config_wire_17;
	wire config_wire_18;
	wire config_wire_19;
	wire config_wire_20;
	wire config_wire_21;
	wire config_wire_22;
	wire config_wire_23;
	wire config_wire_24;
	wire config_wire_25;
	wire config_wire_26;
	wire config_wire_27;
	wire config_wire_28;
	wire config_wire_29;
	wire config_wire_30;
	wire config_wire_31;
	wire config_wire_32;
	wire config_wire_33;
	wire config_wire_34;
	wire config_wire_35;
	wire config_wire_36;
	wire config_wire_37;
	wire config_wire_38;
	wire config_wire_39;
	wire config_wire_40;
	wire config_wire_41;
	wire config_wire_42;
	wire config_wire_43;
	wire config_wire_44;
	wire config_wire_45;
	wire config_wire_46;
	wire config_wire_47;
	wire config_wire_48;
	wire config_wire_49;
	wire config_wire_50;
	wire config_wire_51;
	wire config_wire_52;
	wire config_wire_53;
	wire config_wire_54;
	wire config_wire_55;
	wire config_wire_56;
	wire config_wire_57;
	wire config_wire_58;
	wire config_wire_59;
	wire config_wire_60;
	wire config_wire_61;
	wire config_wire_62;
	wire config_wire_63;
	wire config_wire_64;
	wire config_wire_65;
	wire config_wire_66;
	wire config_wire_67;
	wire config_wire_68;
	wire config_wire_69;
	wire config_wire_70;
	wire config_wire_71;
	wire config_wire_72;
	wire config_wire_73;
	wire config_wire_74;
	wire config_wire_75;
	wire config_wire_76;
	wire config_wire_77;
	wire config_wire_78;
	wire config_wire_79;
	wire config_wire_80;
	wire config_wire_81;
	wire config_wire_82;
	wire config_wire_83;
	wire config_wire_84;
	wire config_wire_85;
	wire config_wire_86;
	wire config_wire_87;
	wire config_wire_88;
	wire config_wire_89;
	wire config_wire_90;
	wire config_wire_91;
	wire config_wire_92;
	wire config_wire_93;
	wire config_wire_94;
	wire config_wire_95;
	wire config_wire_96;
	wire config_wire_97;
	wire config_wire_98;
	wire config_wire_99;
	wire config_wire_100;
	wire config_wire_101;
	wire config_wire_102;
	wire config_wire_103;
	wire config_wire_104;
	wire config_wire_105;
	wire config_wire_106;
	wire config_wire_107;
	wire config_wire_108;
	wire config_wire_109;
	wire [size-1:0] BlockIO2_out0;
	wire [size-1:0] SW7X1_BlockPE14_in1_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockIO2_in0_CONNECT_out0;
	wire [size-1:0] SW7X1_BlockPE13_in1_CONNECT_out0;
	wire [size-1:0] BlockIO5_out0;
	wire [size-1:0] BlockIO6_out0;
	wire [size-1:0] SW7X1_BlockPE2_in0_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockIO9_in0_CONNECT_out0;
	wire [size-1:0] BlockMEM4_out0;
	wire [size-1:0] SW8X1_BlockPE6_in0_CONNECT_out0;
	wire [size-1:0] SW9X1_BlockPE12_in1_CONNECT_out0;
	wire [size-1:0] SW9X1_BlockPE4_in0_CONNECT_out0;
	wire [size-1:0] SW8X1_BlockPE5_in0_CONNECT_out0;
	wire [size-1:0] SW7X1_BlockPE13_in0_CONNECT_out0;
	wire [size-1:0] BlockMEM3_out0;
	wire [size-1:0] BlockIO4_out0;
	wire [size-1:0] BlockIO11_out0;
	wire [size-1:0] SW8X1_BlockPE6_in1_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockIO3_in1_CONNECT_out0;
	wire [size-1:0] SW9X1_BlockPE11_in1_CONNECT_out0;
	wire [size-1:0] SW9X1_BlockPE11_in0_CONNECT_out0;
	wire [size-1:0] BlockIO3_out0;
	wire [size-1:0] BlockPE13_out0;
	wire [size-1:0] BlockIO12_out0;
	wire [size-1:0] SW1X1_BlockMEM1_in0_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockIO8_in1_CONNECT_out0;
	wire [size-1:0] BlockMEM6_out0;
	wire [size-1:0] BlockIO10_out0;
	wire [size-1:0] SW9X1_BlockPE15_in0_CONNECT_out0;
	wire [size-1:0] BlockMEM7_out0;
	wire [size-1:0] SW9X1_BlockPE15_in1_CONNECT_out0;
	wire [size-1:0] SW10X1_BlockPE7_in0_CONNECT_out0;
	wire [size-1:0] SW9X1_BlockPE3_in1_CONNECT_out0;
	wire [size-1:0] BlockIO7_out0;
	wire [size-1:0] SW1X1_BlockIO7_in0_CONNECT_out0;
	wire [size-1:0] BlockPE4_out0;
	wire [size-1:0] SW1X1_BlockIO10_in0_CONNECT_out0;
	wire [size-1:0] BlockPE11_out0;
	wire [size-1:0] SW8X1_BlockPE0_in1_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockIO10_in1_CONNECT_out0;
	wire [size-1:0] SW9X1_BlockPE12_in0_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockMEM0_in0_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockIO7_in1_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockMEM4_in1_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockMEM3_in1_CONNECT_out0;
	wire [size-1:0] SW9X1_BlockPE3_in0_CONNECT_out0;
	wire [size-1:0] BlockPE6_out0;
	wire [size-1:0] BlockPE9_out0;
	wire [size-1:0] SW1X1_BlockMEM0_in1_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockIO6_in0_CONNECT_out0;
	wire [size-1:0] SW7X1_BlockPE14_in0_CONNECT_out0;
	wire [size-1:0] BlockPE7_out0;
	wire [size-1:0] BlockIO8_out0;
	wire [size-1:0] SW1X1_BlockMEM2_in1_CONNECT_out0;
	wire [size-1:0] BlockPE10_out0;
	wire [size-1:0] SW7X1_BlockPE2_in1_CONNECT_out0;
	wire [size-1:0] SW8X1_BlockPE5_in1_CONNECT_out0;
	wire [size-1:0] BlockPE5_out0;
	wire [size-1:0] BlockPE12_out0;
	wire [size-1:0] BlockPE15_out0;
	wire [size-1:0] SW1X1_BlockIO5_in1_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockMEM1_in1_CONNECT_out0;
	wire [size-1:0] BlockPE3_out0;
	wire [size-1:0] BlockMEM0_out0;
	wire [size-1:0] BlockIO9_out0;
	wire [size-1:0] SW9X1_BlockPE9_in1_CONNECT_out0;
	wire [size-1:0] BlockMEM5_out0;
	wire [size-1:0] SW1X1_BlockMEM2_in0_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockIO4_in1_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockIO6_in1_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockIO9_in1_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockMEM4_in0_CONNECT_out0;
	wire [size-1:0] SW9X1_BlockPE1_in1_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockMEM3_in0_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockIO11_in0_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockMEM5_in0_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockIO11_in1_CONNECT_out0;
	wire [size-1:0] SW10X1_BlockPE7_in1_CONNECT_out0;
	wire [size-1:0] BlockPE8_out0;
	wire [size-1:0] SW1X1_BlockIO5_in0_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockIO3_in0_CONNECT_out0;
	wire [size-1:0] SW9X1_BlockPE1_in0_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockMEM5_in1_CONNECT_out0;
	wire [size-1:0] SW9X1_BlockPE9_in0_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockIO12_in0_CONNECT_out0;
	wire [size-1:0] SW9X1_BlockPE4_in1_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockIO2_in1_CONNECT_out0;
	wire [size-1:0] BlockMEM1_out0;
	wire [size-1:0] SW7X1_BlockPE10_in1_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockIO0_in1_CONNECT_out0;
	wire [size-1:0] BlockPE2_out0;
	wire [size-1:0] SW1X1_BlockIO1_in1_CONNECT_out0;
	wire [size-1:0] BlockIO1_out0;
	wire [size-1:0] SW1X1_BlockMEM7_in1_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockMEM6_in0_CONNECT_out0;
	wire [size-1:0] BlockPE14_out0;
	wire [size-1:0] BlockPE1_out0;
	wire [size-1:0] SW1X1_BlockIO1_in0_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockMEM7_in0_CONNECT_out0;
	wire [size-1:0] BlockPE0_out0;
	wire [size-1:0] SW1X1_BlockMEM6_in1_CONNECT_out0;
	wire [size-1:0] SW7X1_BlockPE10_in0_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockIO0_in0_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockIO12_in1_CONNECT_out0;
	wire [size-1:0] SW8X1_BlockPE0_in0_CONNECT_out0;
	wire [size-1:0] SW1X1_BlockIO4_in0_CONNECT_out0;
	wire [size-1:0] BlockMEM2_out0;
	wire [size-1:0] SW8X1_BlockPE8_in1_CONNECT_out0;
	wire [size-1:0] BlockIO0_out0;
	wire [size-1:0] SW1X1_BlockIO8_in0_CONNECT_out0;
	wire [size-1:0] SW8X1_BlockPE8_in0_CONNECT_out0;

	BlockIO BlockIO0(
		.clk(clk),
		.reset(reset),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_in),
		.config_out(config_wire_0),
		.in0(SW1X1_BlockIO0_in0_CONNECT_out0),
		.in1(SW1X1_BlockIO0_in1_CONNECT_out0),
		.out0(BlockIO0_out0)
	);

	BlockIO BlockIO1(
		.clk(clk),
		.reset(reset),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_0),
		.config_out(config_wire_1),
		.in0(SW1X1_BlockIO1_in0_CONNECT_out0),
		.in1(SW1X1_BlockIO1_in1_CONNECT_out0),
		.out0(BlockIO1_out0)
	);

	BlockIO BlockIO10(
		.clk(clk),
		.reset(reset),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_1),
		.config_out(config_wire_2),
		.in0(SW1X1_BlockIO10_in0_CONNECT_out0),
		.in1(SW1X1_BlockIO10_in1_CONNECT_out0),
		.out0(BlockIO10_out0)
	);

	BlockIO BlockIO11(
		.clk(clk),
		.reset(reset),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_2),
		.config_out(config_wire_3),
		.in0(SW1X1_BlockIO11_in0_CONNECT_out0),
		.in1(SW1X1_BlockIO11_in1_CONNECT_out0),
		.out0(BlockIO11_out0)
	);

	BlockIO BlockIO12(
		.clk(clk),
		.reset(reset),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_3),
		.config_out(config_wire_4),
		.in0(SW1X1_BlockIO12_in0_CONNECT_out0),
		.in1(SW1X1_BlockIO12_in1_CONNECT_out0),
		.out0(BlockIO12_out0)
	);

	BlockIO BlockIO2(
		.clk(clk),
		.reset(reset),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_4),
		.config_out(config_wire_5),
		.in0(SW1X1_BlockIO2_in0_CONNECT_out0),
		.in1(SW1X1_BlockIO2_in1_CONNECT_out0),
		.out0(BlockIO2_out0)
	);

	BlockIO BlockIO3(
		.clk(clk),
		.reset(reset),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_5),
		.config_out(config_wire_6),
		.in0(SW1X1_BlockIO3_in0_CONNECT_out0),
		.in1(SW1X1_BlockIO3_in1_CONNECT_out0),
		.out0(BlockIO3_out0)
	);

	BlockIO BlockIO4(
		.clk(clk),
		.reset(reset),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_6),
		.config_out(config_wire_7),
		.in0(SW1X1_BlockIO4_in0_CONNECT_out0),
		.in1(SW1X1_BlockIO4_in1_CONNECT_out0),
		.out0(BlockIO4_out0)
	);

	BlockIO BlockIO5(
		.clk(clk),
		.reset(reset),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_7),
		.config_out(config_wire_8),
		.in0(SW1X1_BlockIO5_in0_CONNECT_out0),
		.in1(SW1X1_BlockIO5_in1_CONNECT_out0),
		.out0(BlockIO5_out0)
	);

	BlockIO BlockIO6(
		.clk(clk),
		.reset(reset),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_8),
		.config_out(config_wire_9),
		.in0(SW1X1_BlockIO6_in0_CONNECT_out0),
		.in1(SW1X1_BlockIO6_in1_CONNECT_out0),
		.out0(BlockIO6_out0)
	);

	BlockIO BlockIO7(
		.clk(clk),
		.reset(reset),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_9),
		.config_out(config_wire_10),
		.in0(SW1X1_BlockIO7_in0_CONNECT_out0),
		.in1(SW1X1_BlockIO7_in1_CONNECT_out0),
		.out0(BlockIO7_out0)
	);

	BlockIO BlockIO8(
		.clk(clk),
		.reset(reset),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_10),
		.config_out(config_wire_11),
		.in0(SW1X1_BlockIO8_in0_CONNECT_out0),
		.in1(SW1X1_BlockIO8_in1_CONNECT_out0),
		.out0(BlockIO8_out0)
	);

	BlockIO BlockIO9(
		.clk(clk),
		.reset(reset),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_11),
		.config_out(config_wire_12),
		.in0(SW1X1_BlockIO9_in0_CONNECT_out0),
		.in1(SW1X1_BlockIO9_in1_CONNECT_out0),
		.out0(BlockIO9_out0)
	);

	BlockMEM BlockMEM0(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_13),
		.config_in(config_wire_12),
		.reset(reset),
		.clk(clk),
		.in0(SW1X1_BlockMEM0_in0_CONNECT_out0),
		.in1(SW1X1_BlockMEM0_in1_CONNECT_out0),
		.out0(BlockMEM0_out0)
	);

	BlockMEM BlockMEM1(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_14),
		.config_in(config_wire_13),
		.reset(reset),
		.clk(clk),
		.in0(SW1X1_BlockMEM1_in0_CONNECT_out0),
		.in1(SW1X1_BlockMEM1_in1_CONNECT_out0),
		.out0(BlockMEM1_out0)
	);

	BlockMEM BlockMEM2(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_15),
		.config_in(config_wire_14),
		.reset(reset),
		.clk(clk),
		.in0(SW1X1_BlockMEM2_in0_CONNECT_out0),
		.in1(SW1X1_BlockMEM2_in1_CONNECT_out0),
		.out0(BlockMEM2_out0)
	);

	BlockMEM BlockMEM3(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_16),
		.config_in(config_wire_15),
		.reset(reset),
		.clk(clk),
		.in0(SW1X1_BlockMEM3_in0_CONNECT_out0),
		.in1(SW1X1_BlockMEM3_in1_CONNECT_out0),
		.out0(BlockMEM3_out0)
	);

	BlockMEM BlockMEM4(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_17),
		.config_in(config_wire_16),
		.reset(reset),
		.clk(clk),
		.in0(SW1X1_BlockMEM4_in0_CONNECT_out0),
		.in1(SW1X1_BlockMEM4_in1_CONNECT_out0),
		.out0(BlockMEM4_out0)
	);

	BlockMEM BlockMEM5(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_18),
		.config_in(config_wire_17),
		.reset(reset),
		.clk(clk),
		.in0(SW1X1_BlockMEM5_in0_CONNECT_out0),
		.in1(SW1X1_BlockMEM5_in1_CONNECT_out0),
		.out0(BlockMEM5_out0)
	);

	BlockMEM BlockMEM6(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_19),
		.config_in(config_wire_18),
		.reset(reset),
		.clk(clk),
		.in0(SW1X1_BlockMEM6_in0_CONNECT_out0),
		.in1(SW1X1_BlockMEM6_in1_CONNECT_out0),
		.out0(BlockMEM6_out0)
	);

	BlockMEM BlockMEM7(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_20),
		.config_in(config_wire_19),
		.reset(reset),
		.clk(clk),
		.in0(SW1X1_BlockMEM7_in0_CONNECT_out0),
		.in1(SW1X1_BlockMEM7_in1_CONNECT_out0),
		.out0(BlockMEM7_out0)
	);

	BlockPE0 BlockPE0(
		.reset(reset),
		.clk(clk),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_21),
		.config_in(config_wire_20),
		.in0(SW8X1_BlockPE0_in0_CONNECT_out0),
		.in1(SW8X1_BlockPE0_in1_CONNECT_out0),
		.out0(BlockPE0_out0)
	);

	BlockPE1 BlockPE1(
		.reset(reset),
		.clk(clk),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_22),
		.config_in(config_wire_21),
		.in0(SW9X1_BlockPE1_in0_CONNECT_out0),
		.in1(SW9X1_BlockPE1_in1_CONNECT_out0),
		.out0(BlockPE1_out0)
	);

	BlockPE10 BlockPE10(
		.reset(reset),
		.clk(clk),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_23),
		.config_in(config_wire_22),
		.in0(SW7X1_BlockPE10_in0_CONNECT_out0),
		.in1(SW7X1_BlockPE10_in1_CONNECT_out0),
		.out0(BlockPE10_out0)
	);

	BlockPE11 BlockPE11(
		.reset(reset),
		.clk(clk),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_24),
		.config_in(config_wire_23),
		.in0(SW9X1_BlockPE11_in0_CONNECT_out0),
		.in1(SW9X1_BlockPE11_in1_CONNECT_out0),
		.out0(BlockPE11_out0)
	);

	BlockPE12 BlockPE12(
		.reset(reset),
		.clk(clk),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_25),
		.config_in(config_wire_24),
		.in0(SW9X1_BlockPE12_in0_CONNECT_out0),
		.in1(SW9X1_BlockPE12_in1_CONNECT_out0),
		.out0(BlockPE12_out0)
	);

	BlockPE13 BlockPE13(
		.reset(reset),
		.clk(clk),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_26),
		.config_in(config_wire_25),
		.in0(SW7X1_BlockPE13_in0_CONNECT_out0),
		.in1(SW7X1_BlockPE13_in1_CONNECT_out0),
		.out0(BlockPE13_out0)
	);

	BlockPE14 BlockPE14(
		.reset(reset),
		.clk(clk),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_27),
		.config_in(config_wire_26),
		.in0(SW7X1_BlockPE14_in0_CONNECT_out0),
		.in1(SW7X1_BlockPE14_in1_CONNECT_out0),
		.out0(BlockPE14_out0)
	);

	BlockPE15 BlockPE15(
		.reset(reset),
		.clk(clk),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_28),
		.config_in(config_wire_27),
		.in0(SW9X1_BlockPE15_in0_CONNECT_out0),
		.in1(SW9X1_BlockPE15_in1_CONNECT_out0),
		.out0(BlockPE15_out0)
	);

	BlockPE2 BlockPE2(
		.reset(reset),
		.clk(clk),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_29),
		.config_in(config_wire_28),
		.in0(SW7X1_BlockPE2_in0_CONNECT_out0),
		.in1(SW7X1_BlockPE2_in1_CONNECT_out0),
		.out0(BlockPE2_out0)
	);

	BlockPE3 BlockPE3(
		.reset(reset),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_30),
		.config_in(config_wire_29),
		.clk(clk),
		.in0(SW9X1_BlockPE3_in0_CONNECT_out0),
		.in1(SW9X1_BlockPE3_in1_CONNECT_out0),
		.out0(BlockPE3_out0)
	);

	BlockPE4 BlockPE4(
		.reset(reset),
		.clk(clk),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_31),
		.config_in(config_wire_30),
		.in0(SW9X1_BlockPE4_in0_CONNECT_out0),
		.in1(SW9X1_BlockPE4_in1_CONNECT_out0),
		.out0(BlockPE4_out0)
	);

	BlockPE5 BlockPE5(
		.reset(reset),
		.clk(clk),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_32),
		.config_in(config_wire_31),
		.in0(SW8X1_BlockPE5_in0_CONNECT_out0),
		.in1(SW8X1_BlockPE5_in1_CONNECT_out0),
		.out0(BlockPE5_out0)
	);

	BlockPE6 BlockPE6(
		.reset(reset),
		.clk(clk),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_33),
		.config_in(config_wire_32),
		.in0(SW8X1_BlockPE6_in0_CONNECT_out0),
		.in1(SW8X1_BlockPE6_in1_CONNECT_out0),
		.out0(BlockPE6_out0)
	);

	BlockPE7 BlockPE7(
		.reset(reset),
		.clk(clk),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_34),
		.config_in(config_wire_33),
		.in0(SW10X1_BlockPE7_in0_CONNECT_out0),
		.in1(SW10X1_BlockPE7_in1_CONNECT_out0),
		.out0(BlockPE7_out0)
	);

	BlockPE8 BlockPE8(
		.reset(reset),
		.clk(clk),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_35),
		.config_in(config_wire_34),
		.in0(SW8X1_BlockPE8_in0_CONNECT_out0),
		.in1(SW8X1_BlockPE8_in1_CONNECT_out0),
		.out0(BlockPE8_out0)
	);

	BlockPE9 BlockPE9(
		.reset(reset),
		.clk(clk),
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_out(config_wire_36),
		.config_in(config_wire_35),
		.in0(SW9X1_BlockPE9_in0_CONNECT_out0),
		.in1(SW9X1_BlockPE9_in1_CONNECT_out0),
		.out0(BlockPE9_out0)
	);

	FULLYCONN_10X1 SW10X1_BlockPE7_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_36),
		.config_out(config_wire_37),
		.in0(BlockPE2_out0),
		.in1(BlockPE3_out0),
		.in2(BlockPE4_out0),
		.in3(BlockPE4_out0),
		.in4(BlockPE6_out0),
		.in5(BlockPE10_out0),
		.in6(BlockPE11_out0),
		.in7(BlockPE12_out0),
		.in8(BlockIO7_out0),
		.in9(BlockMEM5_out0),
		.out0(SW10X1_BlockPE7_in0_CONNECT_out0)
	);

	FULLYCONN_10X1 SW10X1_BlockPE7_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_37),
		.config_out(config_wire_38),
		.in0(BlockPE2_out0),
		.in1(BlockPE3_out0),
		.in2(BlockPE4_out0),
		.in3(BlockPE4_out0),
		.in4(BlockPE6_out0),
		.in5(BlockPE10_out0),
		.in6(BlockPE11_out0),
		.in7(BlockPE12_out0),
		.in8(BlockIO7_out0),
		.in9(BlockMEM5_out0),
		.out0(SW10X1_BlockPE7_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO0_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_38),
		.config_out(config_wire_39),
		.in0(BlockPE0_out0),
		.out0(SW1X1_BlockIO0_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO0_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_39),
		.config_out(config_wire_40),
		.in0(BlockPE0_out0),
		.out0(SW1X1_BlockIO0_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO10_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_40),
		.config_out(config_wire_41),
		.in0(BlockPE10_out0),
		.out0(SW1X1_BlockIO10_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO10_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_41),
		.config_out(config_wire_42),
		.in0(BlockPE10_out0),
		.out0(SW1X1_BlockIO10_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO11_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_42),
		.config_out(config_wire_43),
		.in0(BlockPE11_out0),
		.out0(SW1X1_BlockIO11_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO11_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_43),
		.config_out(config_wire_44),
		.in0(BlockPE11_out0),
		.out0(SW1X1_BlockIO11_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO12_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_44),
		.config_out(config_wire_45),
		.in0(BlockPE12_out0),
		.out0(SW1X1_BlockIO12_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO12_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_45),
		.config_out(config_wire_46),
		.in0(BlockPE12_out0),
		.out0(SW1X1_BlockIO12_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO1_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_46),
		.config_out(config_wire_47),
		.in0(BlockPE1_out0),
		.out0(SW1X1_BlockIO1_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO1_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_47),
		.config_out(config_wire_48),
		.in0(BlockPE1_out0),
		.out0(SW1X1_BlockIO1_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO2_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_48),
		.config_out(config_wire_49),
		.in0(BlockPE2_out0),
		.out0(SW1X1_BlockIO2_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO2_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_49),
		.config_out(config_wire_50),
		.in0(BlockPE2_out0),
		.out0(SW1X1_BlockIO2_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO3_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_50),
		.config_out(config_wire_51),
		.in0(BlockPE3_out0),
		.out0(SW1X1_BlockIO3_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO3_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_51),
		.config_out(config_wire_52),
		.in0(BlockPE3_out0),
		.out0(SW1X1_BlockIO3_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO4_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_52),
		.config_out(config_wire_53),
		.in0(BlockPE4_out0),
		.out0(SW1X1_BlockIO4_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO4_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_53),
		.config_out(config_wire_54),
		.in0(BlockPE4_out0),
		.out0(SW1X1_BlockIO4_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO5_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_54),
		.config_out(config_wire_55),
		.in0(BlockPE5_out0),
		.out0(SW1X1_BlockIO5_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO5_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_55),
		.config_out(config_wire_56),
		.in0(BlockPE5_out0),
		.out0(SW1X1_BlockIO5_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO6_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_56),
		.config_out(config_wire_57),
		.in0(BlockPE6_out0),
		.out0(SW1X1_BlockIO6_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO6_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_57),
		.config_out(config_wire_58),
		.in0(BlockPE6_out0),
		.out0(SW1X1_BlockIO6_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO7_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_58),
		.config_out(config_wire_59),
		.in0(BlockPE7_out0),
		.out0(SW1X1_BlockIO7_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO7_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_59),
		.config_out(config_wire_60),
		.in0(BlockPE7_out0),
		.out0(SW1X1_BlockIO7_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO8_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_60),
		.config_out(config_wire_61),
		.in0(BlockPE8_out0),
		.out0(SW1X1_BlockIO8_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO8_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_61),
		.config_out(config_wire_62),
		.in0(BlockPE8_out0),
		.out0(SW1X1_BlockIO8_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO9_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_62),
		.config_out(config_wire_63),
		.in0(BlockPE9_out0),
		.out0(SW1X1_BlockIO9_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockIO9_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_63),
		.config_out(config_wire_64),
		.in0(BlockPE9_out0),
		.out0(SW1X1_BlockIO9_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockMEM0_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_64),
		.config_out(config_wire_65),
		.in0(BlockPE0_out0),
		.out0(SW1X1_BlockMEM0_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockMEM0_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_65),
		.config_out(config_wire_66),
		.in0(BlockPE0_out0),
		.out0(SW1X1_BlockMEM0_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockMEM1_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_66),
		.config_out(config_wire_67),
		.in0(BlockPE4_out0),
		.out0(SW1X1_BlockMEM1_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockMEM1_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_67),
		.config_out(config_wire_68),
		.in0(BlockPE4_out0),
		.out0(SW1X1_BlockMEM1_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockMEM2_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_68),
		.config_out(config_wire_69),
		.in0(BlockPE8_out0),
		.out0(SW1X1_BlockMEM2_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockMEM2_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_69),
		.config_out(config_wire_70),
		.in0(BlockPE8_out0),
		.out0(SW1X1_BlockMEM2_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockMEM3_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_70),
		.config_out(config_wire_71),
		.in0(BlockPE12_out0),
		.out0(SW1X1_BlockMEM3_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockMEM3_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_71),
		.config_out(config_wire_72),
		.in0(BlockPE12_out0),
		.out0(SW1X1_BlockMEM3_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockMEM4_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_72),
		.config_out(config_wire_73),
		.in0(BlockPE3_out0),
		.out0(SW1X1_BlockMEM4_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockMEM4_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_73),
		.config_out(config_wire_74),
		.in0(BlockPE3_out0),
		.out0(SW1X1_BlockMEM4_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockMEM5_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_74),
		.config_out(config_wire_75),
		.in0(BlockPE7_out0),
		.out0(SW1X1_BlockMEM5_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockMEM5_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_75),
		.config_out(config_wire_76),
		.in0(BlockPE7_out0),
		.out0(SW1X1_BlockMEM5_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockMEM6_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_76),
		.config_out(config_wire_77),
		.in0(BlockPE11_out0),
		.out0(SW1X1_BlockMEM6_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockMEM6_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_77),
		.config_out(config_wire_78),
		.in0(BlockPE11_out0),
		.out0(SW1X1_BlockMEM6_in1_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockMEM7_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_78),
		.config_out(config_wire_79),
		.in0(BlockPE15_out0),
		.out0(SW1X1_BlockMEM7_in0_CONNECT_out0)
	);

	FULLYCONN_1X1 SW1X1_BlockMEM7_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_79),
		.config_out(config_wire_80),
		.in0(BlockPE15_out0),
		.out0(SW1X1_BlockMEM7_in1_CONNECT_out0)
	);

	FULLYCONN_7X1 SW7X1_BlockPE10_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_80),
		.config_out(config_wire_81),
		.in0(BlockPE5_out0),
		.in1(BlockPE6_out0),
		.in2(BlockPE7_out0),
		.in3(BlockPE9_out0),
		.in4(BlockPE11_out0),
		.in5(BlockPE14_out0),
		.in6(BlockIO10_out0),
		.out0(SW7X1_BlockPE10_in0_CONNECT_out0)
	);

	FULLYCONN_7X1 SW7X1_BlockPE10_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_81),
		.config_out(config_wire_82),
		.in0(BlockPE5_out0),
		.in1(BlockPE6_out0),
		.in2(BlockPE7_out0),
		.in3(BlockPE9_out0),
		.in4(BlockPE11_out0),
		.in5(BlockPE14_out0),
		.in6(BlockIO10_out0),
		.out0(SW7X1_BlockPE10_in1_CONNECT_out0)
	);

	FULLYCONN_7X1 SW7X1_BlockPE13_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_82),
		.config_out(config_wire_83),
		.in0(BlockPE0_out0),
		.in1(BlockPE1_out0),
		.in2(BlockPE2_out0),
		.in3(BlockPE9_out0),
		.in4(BlockPE10_out0),
		.in5(BlockPE12_out0),
		.in6(BlockPE14_out0),
		.out0(SW7X1_BlockPE13_in0_CONNECT_out0)
	);

	FULLYCONN_7X1 SW7X1_BlockPE13_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_83),
		.config_out(config_wire_84),
		.in0(BlockPE0_out0),
		.in1(BlockPE1_out0),
		.in2(BlockPE2_out0),
		.in3(BlockPE9_out0),
		.in4(BlockPE10_out0),
		.in5(BlockPE12_out0),
		.in6(BlockPE14_out0),
		.out0(SW7X1_BlockPE13_in1_CONNECT_out0)
	);

	FULLYCONN_7X1 SW7X1_BlockPE14_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_84),
		.config_out(config_wire_85),
		.in0(BlockPE1_out0),
		.in1(BlockPE2_out0),
		.in2(BlockPE9_out0),
		.in3(BlockPE10_out0),
		.in4(BlockPE11_out0),
		.in5(BlockPE13_out0),
		.in6(BlockPE15_out0),
		.out0(SW7X1_BlockPE14_in0_CONNECT_out0)
	);

	FULLYCONN_7X1 SW7X1_BlockPE14_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_85),
		.config_out(config_wire_86),
		.in0(BlockPE1_out0),
		.in1(BlockPE2_out0),
		.in2(BlockPE9_out0),
		.in3(BlockPE10_out0),
		.in4(BlockPE11_out0),
		.in5(BlockPE13_out0),
		.in6(BlockPE15_out0),
		.out0(SW7X1_BlockPE14_in1_CONNECT_out0)
	);

	FULLYCONN_7X1 SW7X1_BlockPE2_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_86),
		.config_out(config_wire_87),
		.in0(BlockPE1_out0),
		.in1(BlockPE3_out0),
		.in2(BlockPE5_out0),
		.in3(BlockPE6_out0),
		.in4(BlockPE7_out0),
		.in5(BlockPE14_out0),
		.in6(BlockIO2_out0),
		.out0(SW7X1_BlockPE2_in0_CONNECT_out0)
	);

	FULLYCONN_7X1 SW7X1_BlockPE2_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_87),
		.config_out(config_wire_88),
		.in0(BlockPE1_out0),
		.in1(BlockPE3_out0),
		.in2(BlockPE5_out0),
		.in3(BlockPE6_out0),
		.in4(BlockPE7_out0),
		.in5(BlockPE14_out0),
		.in6(BlockIO2_out0),
		.out0(SW7X1_BlockPE2_in1_CONNECT_out0)
	);

	FULLYCONN_8X1 SW8X1_BlockPE0_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_88),
		.config_out(config_wire_89),
		.in0(BlockPE1_out0),
		.in1(BlockPE3_out0),
		.in2(BlockPE4_out0),
		.in3(BlockPE5_out0),
		.in4(BlockPE7_out0),
		.in5(BlockPE12_out0),
		.in6(BlockIO0_out0),
		.in7(BlockMEM0_out0),
		.out0(SW8X1_BlockPE0_in0_CONNECT_out0)
	);

	FULLYCONN_8X1 SW8X1_BlockPE0_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_89),
		.config_out(config_wire_90),
		.in0(BlockPE1_out0),
		.in1(BlockPE3_out0),
		.in2(BlockPE4_out0),
		.in3(BlockPE5_out0),
		.in4(BlockPE7_out0),
		.in5(BlockPE12_out0),
		.in6(BlockIO0_out0),
		.in7(BlockMEM0_out0),
		.out0(SW8X1_BlockPE0_in1_CONNECT_out0)
	);

	FULLYCONN_8X1 SW8X1_BlockPE5_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_90),
		.config_out(config_wire_91),
		.in0(BlockPE0_out0),
		.in1(BlockPE1_out0),
		.in2(BlockPE2_out0),
		.in3(BlockPE4_out0),
		.in4(BlockPE6_out0),
		.in5(BlockPE9_out0),
		.in6(BlockPE10_out0),
		.in7(BlockIO5_out0),
		.out0(SW8X1_BlockPE5_in0_CONNECT_out0)
	);

	FULLYCONN_8X1 SW8X1_BlockPE5_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_91),
		.config_out(config_wire_92),
		.in0(BlockPE0_out0),
		.in1(BlockPE1_out0),
		.in2(BlockPE2_out0),
		.in3(BlockPE4_out0),
		.in4(BlockPE6_out0),
		.in5(BlockPE9_out0),
		.in6(BlockPE10_out0),
		.in7(BlockIO5_out0),
		.out0(SW8X1_BlockPE5_in1_CONNECT_out0)
	);

	FULLYCONN_8X1 SW8X1_BlockPE6_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_92),
		.config_out(config_wire_93),
		.in0(BlockPE1_out0),
		.in1(BlockPE2_out0),
		.in2(BlockPE5_out0),
		.in3(BlockPE7_out0),
		.in4(BlockPE9_out0),
		.in5(BlockPE10_out0),
		.in6(BlockPE11_out0),
		.in7(BlockIO6_out0),
		.out0(SW8X1_BlockPE6_in0_CONNECT_out0)
	);

	FULLYCONN_8X1 SW8X1_BlockPE6_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_93),
		.config_out(config_wire_94),
		.in0(BlockPE1_out0),
		.in1(BlockPE2_out0),
		.in2(BlockPE5_out0),
		.in3(BlockPE7_out0),
		.in4(BlockPE9_out0),
		.in5(BlockPE10_out0),
		.in6(BlockPE11_out0),
		.in7(BlockIO6_out0),
		.out0(SW8X1_BlockPE6_in1_CONNECT_out0)
	);

	FULLYCONN_8X1 SW8X1_BlockPE8_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_94),
		.config_out(config_wire_95),
		.in0(BlockPE4_out0),
		.in1(BlockPE5_out0),
		.in2(BlockPE7_out0),
		.in3(BlockPE9_out0),
		.in4(BlockPE11_out0),
		.in5(BlockPE12_out0),
		.in6(BlockIO8_out0),
		.in7(BlockMEM2_out0),
		.out0(SW8X1_BlockPE8_in0_CONNECT_out0)
	);

	FULLYCONN_8X1 SW8X1_BlockPE8_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_95),
		.config_out(config_wire_96),
		.in0(BlockPE4_out0),
		.in1(BlockPE5_out0),
		.in2(BlockPE7_out0),
		.in3(BlockPE9_out0),
		.in4(BlockPE11_out0),
		.in5(BlockPE12_out0),
		.in6(BlockIO8_out0),
		.in7(BlockMEM2_out0),
		.out0(SW8X1_BlockPE8_in1_CONNECT_out0)
	);

	FULLYCONN_9X1 SW9X1_BlockPE11_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_96),
		.config_out(config_wire_97),
		.in0(BlockPE0_out0),
		.in1(BlockPE6_out0),
		.in2(BlockPE7_out0),
		.in3(BlockPE8_out0),
		.in4(BlockPE10_out0),
		.in5(BlockPE14_out0),
		.in6(BlockPE15_out0),
		.in7(BlockIO11_out0),
		.in8(BlockMEM6_out0),
		.out0(SW9X1_BlockPE11_in0_CONNECT_out0)
	);

	FULLYCONN_9X1 SW9X1_BlockPE11_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_97),
		.config_out(config_wire_98),
		.in0(BlockPE0_out0),
		.in1(BlockPE6_out0),
		.in2(BlockPE7_out0),
		.in3(BlockPE8_out0),
		.in4(BlockPE10_out0),
		.in5(BlockPE14_out0),
		.in6(BlockPE15_out0),
		.in7(BlockIO11_out0),
		.in8(BlockMEM6_out0),
		.out0(SW9X1_BlockPE11_in1_CONNECT_out0)
	);

	FULLYCONN_9X1 SW9X1_BlockPE12_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_98),
		.config_out(config_wire_99),
		.in0(BlockPE0_out0),
		.in1(BlockPE1_out0),
		.in2(BlockPE8_out0),
		.in3(BlockPE9_out0),
		.in4(BlockPE11_out0),
		.in5(BlockPE13_out0),
		.in6(BlockPE15_out0),
		.in7(BlockIO12_out0),
		.in8(BlockMEM3_out0),
		.out0(SW9X1_BlockPE12_in0_CONNECT_out0)
	);

	FULLYCONN_9X1 SW9X1_BlockPE12_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_99),
		.config_out(config_wire_100),
		.in0(BlockPE0_out0),
		.in1(BlockPE1_out0),
		.in2(BlockPE8_out0),
		.in3(BlockPE9_out0),
		.in4(BlockPE11_out0),
		.in5(BlockPE13_out0),
		.in6(BlockPE15_out0),
		.in7(BlockIO12_out0),
		.in8(BlockMEM3_out0),
		.out0(SW9X1_BlockPE12_in1_CONNECT_out0)
	);

	FULLYCONN_9X1 SW9X1_BlockPE15_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_100),
		.config_out(config_wire_101),
		.in0(BlockPE2_out0),
		.in1(BlockPE3_out0),
		.in2(BlockPE4_out0),
		.in3(BlockPE10_out0),
		.in4(BlockPE11_out0),
		.in5(BlockPE12_out0),
		.in6(BlockPE12_out0),
		.in7(BlockPE14_out0),
		.in8(BlockMEM7_out0),
		.out0(SW9X1_BlockPE15_in0_CONNECT_out0)
	);

	FULLYCONN_9X1 SW9X1_BlockPE15_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_101),
		.config_out(config_wire_102),
		.in0(BlockPE2_out0),
		.in1(BlockPE3_out0),
		.in2(BlockPE4_out0),
		.in3(BlockPE10_out0),
		.in4(BlockPE11_out0),
		.in5(BlockPE12_out0),
		.in6(BlockPE12_out0),
		.in7(BlockPE14_out0),
		.in8(BlockMEM7_out0),
		.out0(SW9X1_BlockPE15_in1_CONNECT_out0)
	);

	FULLYCONN_9X1 SW9X1_BlockPE1_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_102),
		.config_out(config_wire_103),
		.in0(BlockPE0_out0),
		.in1(BlockPE2_out0),
		.in2(BlockPE4_out0),
		.in3(BlockPE5_out0),
		.in4(BlockPE6_out0),
		.in5(BlockPE12_out0),
		.in6(BlockPE13_out0),
		.in7(BlockPE14_out0),
		.in8(BlockIO1_out0),
		.out0(SW9X1_BlockPE1_in0_CONNECT_out0)
	);

	FULLYCONN_9X1 SW9X1_BlockPE1_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_103),
		.config_out(config_wire_104),
		.in0(BlockPE0_out0),
		.in1(BlockPE2_out0),
		.in2(BlockPE4_out0),
		.in3(BlockPE5_out0),
		.in4(BlockPE6_out0),
		.in5(BlockPE12_out0),
		.in6(BlockPE13_out0),
		.in7(BlockPE14_out0),
		.in8(BlockIO1_out0),
		.out0(SW9X1_BlockPE1_in1_CONNECT_out0)
	);

	FULLYCONN_9X1 SW9X1_BlockPE3_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_104),
		.config_out(config_wire_105),
		.in0(BlockPE0_out0),
		.in1(BlockPE0_out0),
		.in2(BlockPE2_out0),
		.in3(BlockPE6_out0),
		.in4(BlockPE7_out0),
		.in5(BlockPE14_out0),
		.in6(BlockPE15_out0),
		.in7(BlockIO3_out0),
		.in8(BlockMEM4_out0),
		.out0(SW9X1_BlockPE3_in0_CONNECT_out0)
	);

	FULLYCONN_9X1 SW9X1_BlockPE3_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_105),
		.config_out(config_wire_106),
		.in0(BlockPE0_out0),
		.in1(BlockPE0_out0),
		.in2(BlockPE2_out0),
		.in3(BlockPE6_out0),
		.in4(BlockPE7_out0),
		.in5(BlockPE14_out0),
		.in6(BlockPE15_out0),
		.in7(BlockIO3_out0),
		.in8(BlockMEM4_out0),
		.out0(SW9X1_BlockPE3_in1_CONNECT_out0)
	);

	FULLYCONN_9X1 SW9X1_BlockPE4_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_106),
		.config_out(config_wire_107),
		.in0(BlockPE0_out0),
		.in1(BlockPE1_out0),
		.in2(BlockPE5_out0),
		.in3(BlockPE7_out0),
		.in4(BlockPE8_out0),
		.in5(BlockPE9_out0),
		.in6(BlockPE11_out0),
		.in7(BlockIO4_out0),
		.in8(BlockMEM1_out0),
		.out0(SW9X1_BlockPE4_in0_CONNECT_out0)
	);

	FULLYCONN_9X1 SW9X1_BlockPE4_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_107),
		.config_out(config_wire_108),
		.in0(BlockPE0_out0),
		.in1(BlockPE1_out0),
		.in2(BlockPE5_out0),
		.in3(BlockPE7_out0),
		.in4(BlockPE8_out0),
		.in5(BlockPE9_out0),
		.in6(BlockPE11_out0),
		.in7(BlockIO4_out0),
		.in8(BlockMEM1_out0),
		.out0(SW9X1_BlockPE4_in1_CONNECT_out0)
	);

	FULLYCONN_9X1 SW9X1_BlockPE9_in0_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_108),
		.config_out(config_wire_109),
		.in0(BlockPE4_out0),
		.in1(BlockPE5_out0),
		.in2(BlockPE6_out0),
		.in3(BlockPE8_out0),
		.in4(BlockPE10_out0),
		.in5(BlockPE12_out0),
		.in6(BlockPE13_out0),
		.in7(BlockPE14_out0),
		.in8(BlockIO9_out0),
		.out0(SW9X1_BlockPE9_in0_CONNECT_out0)
	);

	FULLYCONN_9X1 SW9X1_BlockPE9_in1_CONNECT(
		.config_clk(config_clk),
		.config_reset(config_reset),
		.config_in(config_wire_109),
		.config_out(config_out),
		.in0(BlockPE4_out0),
		.in1(BlockPE5_out0),
		.in2(BlockPE6_out0),
		.in3(BlockPE8_out0),
		.in4(BlockPE10_out0),
		.in5(BlockPE12_out0),
		.in6(BlockPE13_out0),
		.in7(BlockPE14_out0),
		.in8(BlockIO9_out0),
		.out0(SW9X1_BlockPE9_in1_CONNECT_out0)
	);



endmodule

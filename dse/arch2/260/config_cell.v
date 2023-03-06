// Module Name: config_cell
// Module Type: CONFIG_CELL 
// Include Module:
// Input Ports: 
// Output Ports:
// Special Ports: config_clk config_reset config_in config_out config_sig
// Config Width: 

module config_cell(config_clk, config_reset, config_in, config_out, config_sig);
    parameter size = 1;
    input config_clk, config_reset, config_in;
    output config_out;
    output [size-1:0] config_sig;
    reg [size-1:0] temp;

    always @(posedge config_clk, posedge config_reset)
        if (config_reset)
            temp = 0;
        else
            begin
                temp = temp >> 1;
                temp[size-1] = config_in;
            end
    assign config_sig = temp;
    assign config_out = temp[0];
endmodule

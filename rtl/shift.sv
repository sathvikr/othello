module shift(
    input logic [63:0] disks,
    input logic [2:0] dir,
    output logic [63:0] result
);

localparam [63:0] MASK0 = 64'h7F7F7F7F7F7F7F7F;
localparam [63:0] MASK1 = 64'h007F7F7F7F7F7F7F;
localparam [63:0] MASK2 = 64'hFFFFFFFFFFFFFFFF;
localparam [63:0] MASK3 = 64'h00FEFEFEFEFEFEFE;
localparam [63:0] MASK4 = 64'hFEFEFEFEFEFEFEFE;
localparam [63:0] MASK5 = 64'hFEFEFEFEFEFEFE00;
localparam [63:0] MASK6 = 64'hFFFFFFFFFFFFFFFF;
localparam [63:0] MASK7 = 64'h7F7F7F7F7F7F7F00;

localparam [7:0] LSHIFT0 = 8'd0;
localparam [7:0] LSHIFT1 = 8'd0;
localparam [7:0] LSHIFT2 = 8'd0;
localparam [7:0] LSHIFT3 = 8'd0;
localparam [7:0] LSHIFT4 = 8'd1;
localparam [7:0] LSHIFT5 = 8'd9;
localparam [7:0] LSHIFT6 = 8'd8;
localparam [7:0] LSHIFT7 = 8'd7;

localparam [7:0] RSHIFT0 = 8'd1;
localparam [7:0] RSHIFT1 = 8'd9;
localparam [7:0] RSHIFT2 = 8'd8;
localparam [7:0] RSHIFT3 = 8'd7;
localparam [7:0] RSHIFT4 = 8'd0;
localparam [7:0] RSHIFT5 = 8'd0;
localparam [7:0] RSHIFT6 = 8'd0;
localparam [7:0] RSHIFT7 = 8'd0;

logic [63:0] mask;
logic [7:0] lshift, rshift;

always_comb begin
    case(dir)
        3'd0: begin mask = MASK0; lshift = LSHIFT0; rshift = RSHIFT0; end
        3'd1: begin mask = MASK1; lshift = LSHIFT1; rshift = RSHIFT1; end
        3'd2: begin mask = MASK2; lshift = LSHIFT2; rshift = RSHIFT2; end
        3'd3: begin mask = MASK3; lshift = LSHIFT3; rshift = RSHIFT3; end
        3'd4: begin mask = MASK4; lshift = LSHIFT4; rshift = RSHIFT4; end
        3'd5: begin mask = MASK5; lshift = LSHIFT5; rshift = RSHIFT5; end
        3'd6: begin mask = MASK6; lshift = LSHIFT6; rshift = RSHIFT6; end
        3'd7: begin mask = MASK7; lshift = LSHIFT7; rshift = RSHIFT7; end
    endcase
end

assign result = dir < 4 ? ((disks >> rshift) & mask) : ((disks << lshift) & mask);

endmodule
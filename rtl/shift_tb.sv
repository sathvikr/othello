module shift_tb;

    logic [63:0] disks;
    logic [2:0] dir;
    logic [63:0] result;

    shift uut(.*);

    initial begin
        for (int i = 0; i < 20; i++) begin
            disks = $urandom % 64'hFFFFFFFFFFFFFFFF;
            dir = $urandom % 8;
            #10;
            $display("disks = %h, dir = %d, result = %h", disks, dir, result);
        end

        $finish;
    end
endmodule

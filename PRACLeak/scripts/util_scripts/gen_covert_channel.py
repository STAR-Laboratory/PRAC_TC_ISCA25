def get_addr_from_row(row, bank=17):
    addr = row
    addr <<= 12
    addr |= bank
    addr <<= 6
    return addr

def gen_c_activity_flush_trace(row=0, num_addrs=1, filename="c_activity_flush.trace"):
    with open(filename, "w") as f:
        for i in range(num_addrs):
            addr = get_addr_from_row(row, bank=i)
            f.write(f"0 {addr}\n")

def gen_activity_sender(msg, NBO, row=0, filename="c_activity_send.trace"):
    with open(filename, "w") as f:
        curr_row = 0
        addr = get_addr_from_row(row, bank=0)
        f.write(f"127 {addr}\n")
        for num in msg:
            if int(num) == 0:       # don't access target
                for i in range(int(NBO * 2 * 1.22)):
                    addr = get_addr_from_row(curr_row % (1 << 16))
                    curr_row += 1
                    f.write(f"127 {addr}\n")
            elif int(num) == 1:     # access target addr
                for i in range(NBO):
                    addr = get_addr_from_row(row, bank=0)
                    f.write(f"127 {addr}\n")
                    addr = get_addr_from_row(curr_row % (1 << 16))
                    curr_row += 1
                    f.write(f"127 {addr}\n")
                        
        # pad the rest with dummy acts
        for row in range(1 << 16):
            addr = get_addr_from_row(row, bank=18)
            f.write(f"127 {addr}\n")

def gen_activity_recv(filename="c_activity_recv.trace"):
    with open(filename, "w") as f:
        for row in range(1 << 16):
            addr = get_addr_from_row(row, bank=19)
            f.write(f"127 {addr}\n")

#===============================================================================

def gen_c_activation_flush_trace(period, access_bank=0, filename="c_activation_flush.trace"):
    with open(filename, "w") as f:
        for i in range(period):
            addr = get_addr_from_row(i, bank=access_bank)
            f.write(f"0 {addr}\n")

def gen_activation_sender(msg, NBO, filename="c_activation_send.trace"):
    access_bank, dump_bank = 0, 1
    access_row, dump_row = 0, 0
    with open(filename, "w") as f:
        while msg > 0:
            num = msg % 512
            msg //= 512

            # Send phase
            for i in range(num):
                addr = get_addr_from_row(access_row % (1 << 16), bank=access_bank)
                f.write(f"127 {addr}\n")
                addr = get_addr_from_row(dump_row % (1 << 16), bank = dump_bank)
                dump_row += 1
                f.write(f"127 {addr}\n")
            access_row += 1

            for i in range(int((NBO - num) * 2.5)):
                addr = get_addr_from_row(dump_row % (1 << 16), bank = dump_bank)
                dump_row += 1
                f.write(f"127 {addr}\n")
            
            # Wait phase
            for i in range(int(2190 / 512 * NBO)):
                addr = get_addr_from_row(dump_row % (1 << 16), bank = dump_bank)
                dump_row += 1
                f.write(f"127 {addr}\n")

        # pad the rest with dummy acts
        for row in range(1 << 16):
            addr = get_addr_from_row(row, bank=18)
            f.write(f"127 {addr}\n")

def gen_activation_receiver(period, NBO, filename="c_activation_recv.trace"):
    access_bank, dump_bank = 0, 2
    access_row, dump_row = 0, 0
    with open(filename, "w") as f:
        for _ in range(period):
            # Wait phase
            for i in range(int(940 / 512 * NBO)):
                addr = get_addr_from_row(dump_row % (1 << 16), bank = dump_bank)
                dump_row += 1
                f.write(f"127 {addr}\n")

            # Check phase
            for i in range(NBO):
                addr = get_addr_from_row(access_row % (1 << 16), bank=access_bank)
                f.write(f"127 {addr}\n")
                addr = get_addr_from_row(dump_row % (1 << 16), bank = dump_bank)
                dump_row += 1
                f.write(f"127 {addr}\n")
            access_row += 1

            # Wait phase
            for i in range(int(100 / 512 * NBO)):
                addr = get_addr_from_row(dump_row % (1 << 16), bank = dump_bank)
                dump_row += 1
                f.write(f"127 {addr}\n")

def create_msg(msg_lst):
    msg = 0
    for num in msg_lst:
        msg <<= 9
        msg |= num
    return msg, len(msg_lst)

if __name__ == "__main__":
    NBO = 1024
    gen_c_activity_flush_trace()
    # gen_activity_sender("101100101011100011010011", NBO)
    # gen_activity_sender("100000000000000000000111", NBO)
    gen_activity_sender("111111111111111111111111", NBO)
    gen_activity_recv()

    msg_lst = [350, 150, 50, 10, 100, 200, 300, 400, 500, 350, 400, 200, 30]
    msg_lst = [num//2 for num in msg_lst]
    msg, period = create_msg(msg_lst)
    print("msg = {:b}".format(msg))

    gen_c_activation_flush_trace(period)
    gen_activation_sender(msg, NBO)
    gen_activation_receiver(period, NBO)

    print("Done.")

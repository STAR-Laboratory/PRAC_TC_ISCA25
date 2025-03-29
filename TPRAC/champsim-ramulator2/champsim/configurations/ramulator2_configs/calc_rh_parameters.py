from math import ceil, floor

def get_abo_only_parameters(tRH):
    nrh_nbo_pairs = [
        (128, 80),
        (256, 222),
        (512, 481),
        (1024, 995),
        (2048, 2022),
        (4096, 4072),
    ]
    for nrh, NBO in nrh_nbo_pairs:
        if tRH <= nrh:
            return NBO
    return 32

def get_bat_parameters(tRH):
    nrh_nbo_pairs = [
        (128, 12),
        (256, 21),
        (512, 49),
        (1024, 110),
        (2048, 244),
        (4096, 544),
    ]
    for nrh, NBO in nrh_nbo_pairs:
        if tRH <= nrh:
            return NBO
    return 32

def get_tprac_parameters(tRH):
    nrh_nbo_pairs = [
        (128, 974),
        (256, 1442),
        (512, 2898),
        (1024, 6070),
        (2048, 13038),
        (4096, 28638),
    ]
    for nrh, NBO in nrh_nbo_pairs:
        if tRH <= nrh:
            return NBO
    return 32

if __name__ == "__main__":
    print(get_abo_only_parameters(1024))
    print(get_bat_parameters(1024))
    print(get_tprac_parameters(1024))
    
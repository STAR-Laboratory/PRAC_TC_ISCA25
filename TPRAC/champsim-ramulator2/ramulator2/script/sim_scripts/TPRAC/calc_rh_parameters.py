from math import ceil, floor

# def get_para_parameters(tRH):
#     threshold = 1 - (10**-15)**(1/tRH)
#     return threshold

# def get_graphene_parameters(tRH):
#     tREFW = 32000000
#     tRC = 46
#     k = 1
#     num_table_entries = int(ceil((tREFW/tRC)/tRH * ((k+1)/(k)) - 1))
#     activation_threshold = int(floor(tRH / (k+1)))
#     reset_period_ns = int(tREFW / k)
#     return num_table_entries, activation_threshold, reset_period_ns

# def get_hydra_parameters(tRH):
#     hydra_tracking_threshold = int(floor(tRH / 2))
#     hydra_group_threshold = int(floor(hydra_tracking_threshold * 4 / 5))
#     hydra_row_group_size = 128
#     hydra_reset_period_ns = 32000000
#     hydra_rcc_num_per_rank = 4096
#     hydra_rcc_policy = "RANDOM"
#     return hydra_tracking_threshold, hydra_group_threshold, hydra_row_group_size, hydra_reset_period_ns, hydra_rcc_num_per_rank, hydra_rcc_policy

# def get_twice_parameters(tRH):
#     tREFW = 32000000
#     tREFI = 3900
#     twice_rh_threshold = int(floor(tRH / 2))
#     twice_pruning_interval_threshold = twice_rh_threshold / (tREFW / tREFI)
#     return twice_rh_threshold, twice_pruning_interval_threshold

# def get_rrs_parameters(tRH):
#     tREFW = 32000000
#     tRC = 46
#     reset_period_ns = tREFW
#     rss_threshold = int(floor(tRH / 6))
#     num_hrt_entries = int(ceil((tREFW/tRC)/rss_threshold))
#     num_rit_entries = int(ceil((tREFW/tRC)/rss_threshold))*2
#     return num_hrt_entries, num_rit_entries, rss_threshold, reset_period_ns

# def get_oraclerh_parameters(tRH):
#     return tRH

# def get_mithril_parameters(tRH):
#     if not hasattr(get_graphene_parameters, "cache"):
#         get_mithril_parameters.cache = {}
#     if tRH in get_mithril_parameters.cache:
#         cache_entry = get_mithril_parameters.cache[tRH]
#         rfmTH, nEntry = cache_entry[0], cache_entry[1]
#         return rfmTH, nEntry
#     tREFW = 32000000
#     tRC = 46
#     tRFC = 195
#     tRFM = 195
#     tREFI = 3900
#     rfmTH = 512
#     upper_bound = tRH / 2
#     nEntry = 1
#     prev_m = 1e10
#     while True:
#         multiplier = (tREFW * (1 - tRFC / tREFI)) / (tRC * rfmTH + tRFM) - 2
#         m = (rfmTH / nEntry) * multiplier
#         for k in range(1, nEntry + 1):
#             m += rfmTH / k
#         if m < upper_bound:
#             break
#         nEntry = int((nEntry + 1) * 1.1)
#         if prev_m < m:
#             prev_m = 1e10 
#             rfmTH = int(rfmTH // 2)
#             nEntry = 1
#         else:
#             prev_m = m
#     get_mithril_parameters.cache[tRH] = [rfmTH, nEntry]
#     return rfmTH, nEntry

# def get_rega_parameters(tRH):
#     SUBARR_SIZE = 512
#     V = int(ceil(SUBARR_SIZE / tRH)) 
#     T = int(ceil(tRH / SUBARR_SIZE))
#     return int(ceil(32 + (V - 1) * 17.5)), V, T

# def get_aqua_parameters(tRH):
#     tREFW = 32000000
#     tRC = 46
#     reset_period_ns = tREFW
#     art_threshold = int(floor(tRH / 2))
#     num_art_entries = int(ceil((tREFW/tRC)/art_threshold))
#     t_agg = art_threshold * tRC
#     t_move = (128*5 + tRC) * 2
#     num_qrows_per_bank = int(ceil(tREFW / (16 * t_move + t_agg)))
#     num_fpt_entries = num_qrows_per_bank
#     return art_threshold, num_art_entries, num_qrows_per_bank, num_fpt_entries, reset_period_ns

# def get_pride_parameters(tRH):
#     nrh_rfm_pairs = [
#         (125, 2),
#         (250, 5),
#         (500, 10),
#         (1000, 21),
#         (2000, 44),
#         (4000, 89),
#     ]
#     for nrh, BAT in nrh_rfm_pairs:
#         if tRH <= nrh:
#             return BAT
#     return 75

def get_abo_only_parameters(tRH):
    nrh_nbo_pairs = [
        (128, 100),
        (256, 250),
        (512, 500),
        (1024, 1000),
        (2048, 2000),
        (4096, 4000),
    ]
    for nrh, NBO in nrh_nbo_pairs:
        if tRH <= nrh:
            return NBO
    return 32

def get_bat_parameters(tRH):
    nrh_nbo_pairs = [
        (128, 10),
        (256, 26),
        (512, 60),
        (1024, 261),
        (2048, 530),
        (4096, 530),
    ]
    for nrh, NBO in nrh_nbo_pairs:
        if tRH <= nrh:
            return NBO
    return 32

def get_tprac_s_parameters(tRH):
    nrh_nbo_pairs = [
        (128, 4),
        (256, 2),
        (512, 1),
        (1024, 0.25),
        (2048, 0.125),
        (4096, 0.125),
    ]
    for nrh, NBO in nrh_nbo_pairs:
        if tRH <= nrh:
            return NBO
    return 32

if __name__ == "__main__":
    print(get_abo_only_parameters(512))
    print(get_bat_parameters(512))
    print(get_tprac_s_parameters(512))
#     print(get_pride_parameters(250))
#     print(get_pride_parameters(500))
#     print(get_pride_parameters(1000))
#     print(get_pride_parameters(2000))
#     print(get_pride_parameters(4000))
    
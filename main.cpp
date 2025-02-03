#include <stdio.h>
#include <umd/device/device_api_metal.h>
#include <umd/device/tt_cluster_descriptor.h>

static char const* tt_logo =
"       TTTTTTT                 TTTT       \n"
"    TTTTTTTTTTTTT              TTTTTTT    \n"
" TTTTTTTTTTTTTTTTTTT           TTTTTTTTTT \n"
"TTTTTTTTTTTTTTTTTTTTTTTT       TTTTTTTTTTT\n"
"TTTTTTTTTTTTTTTTTTTTTTTTTTT    TTTTTTTTTTT\n"
"TTTTTTTTTTTTTTTTTTTTTTTTTTTTTT TTTTTTTTTTT\n"
"TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n"
"TTTTTTTTTTT  TTTTTTTTTTTTTTTT  TTTTTTTTTTT\n"
"TTTTTTTTTTT      TTTTTTTT      TTTTTTTTTTT\n"
"TTTTTTTTTTT         TT         TTTTTTTTTTT\n"
"  TTTTTTTTT                    TTTTTTTTT  \n"
"      TTTTT                    TTTTT      \n"
"         TT                    TT         \n"
"          TTTT              TTTT          \n"
"          TTTTTTTT      TTTTTTTT          \n"
"          TTTTTTTTTTTTTTTTTTTTTT          \n"
"          TTTTTTTTTTTTTTTTTTTTTT          \n"
"          TTTTTTTTTTTTTTTTTTTTTT          \n"
"          TTTTTTTTTTTTTTTTTTTTTT          \n"
"            TTTTTTTTTTTTTTTTTT            \n"
"               TTTTTTTTTTTT               \n"
"                  TTTTTT                  \n"
;

#define LOGO_WIDTH (42)

std::ostream& operator<<(std::ostream& os, BoardType& ty) {
    char const* str;
    switch (ty) {
        case E75: str = "Grayskull E75"; break;
        case E150: str = "Grayskull E150"; break;
        case E300: str = "Grayskull E300"; break;
        case N150: str = "Wormhole N150"; break;
        case N300: str = "Wormhole N300"; break;
        case P100: str = "Blackhole P100"; break;
        case P150A: str = "Blackhole P150A"; break;
        case P300: str = "Blackhole P300"; break;
        case GALAXY: str = "Galaxy"; break;
        case UNKNOWN: str = "unknown"; break;
    }
    return os << str;
}

typedef enum : int {
    SZ_B = 0,
    SZ_KIB,
    SZ_MIB,
    SZ_GIB,
    SZ_TIB,
} SizeUnit;

std::ostream& operator<<(std::ostream& os, SizeUnit u) {
    char const* str;
    switch (u) {
        case SZ_B: str = "B"; break;
        case SZ_KIB: str = "KiB"; break;
        case SZ_MIB: str = "MiB"; break;
        case SZ_GIB: str = "GiB"; break;
        case SZ_TIB: str = "TiB"; break;
    }
    return os << str;
}

static std::pair<SizeUnit, std::uint64_t> size_unit(std::uint64_t val) {
    SizeUnit unit = SZ_B;
    while (val % 1024 == 0) {
        unit = (SizeUnit)(((int)unit)+1);
        val /= 1024;
    }
    return {unit,val};
}

class SizeUnitVal {
public:
    std::pair<SizeUnit, uint64_t> value;
    SizeUnitVal(std::uint64_t input) : value(size_unit(input)) {}
};

std::ostream& operator<<(std::ostream& os, SizeUnitVal u) {
    return os << u.value.second << " " << u.value.first;
}

int main() {
    char padding[LOGO_WIDTH + 1];
    memset(padding, ' ', LOGO_WIDTH);
    padding[LOGO_WIDTH] = '\0';

    printf("\033[s");
    fflush(stdout);

    size_t printed_rows = 0;

    for (chip_id_t dev_id : tt::umd::Cluster::detect_available_device_ids())
    {
        std::set<chip_id_t> cluster_devices { dev_id };
        auto dev = tt::umd::Cluster(cluster_devices);

        auto cluster_desc = dev.get_cluster_description();
        auto board = cluster_desc->get_board_type(dev_id);

        printf("%s  ", padding);
        std::cout << board;
        printf("\n");
        printed_rows ++;

        auto asics = dev.get_target_device_ids();
        size_t i = 0;
        for (chip_id_t asic : asics) {
            auto asic_desc = dev.get_soc_descriptor(asic);
            auto cores_tensix = asic_desc.get_cores(CoreType::TENSIX);
            auto cores_pcie = asic_desc.get_cores(CoreType::PCIE);
            auto cores_active_eth = asic_desc.get_cores(CoreType::ACTIVE_ETH);

            auto dram_ch_num = dev.get_num_dram_channels(asic);
            std::uint64_t dram_total = 0;
            std::vector<std::uint64_t> dram_ch_sizes;
            dram_ch_sizes.reserve(dram_ch_num);
            for (size_t i = 0; i < dram_ch_num; i ++) {
                std::uint64_t sz = dev.get_dram_channel_size(asic, i);
                dram_total += sz;
                dram_ch_sizes.push_back(sz);
            }
            bool dram_all_ch_same_size = true;
            if (dram_ch_num > 0) {
                for (size_t i = 1; i < dram_ch_num; i ++) {
                    if (dram_ch_sizes[i] != dram_ch_sizes[0]) {
                        dram_all_ch_same_size = false;
                        break;
                    }
                }
            }

            printf("%s  ", padding);
            if (i + 1 < asics.size()) {
                printf("╠ ");
            } else {
                printf("╚ ");
            }

            printf("ASIC %zu : ", i);
            printf("%zu Tensix Cores ", cores_tensix.size());
            printf("%zu PCIe Cores ", cores_pcie.size());
            printf("%zu Active Ethernet Cores ", cores_active_eth.size());

            std::cout << "DRAM: " << SizeUnitVal(dram_total) << " Total";

            if (dram_all_ch_same_size && dram_ch_num > 0) {
                std::cout << " (" << dram_ch_num << " x " << SizeUnitVal(dram_ch_sizes[0]) << ")";
            }

            printf("\n");
            printed_rows ++;

            i ++;
        }

        dev.close_device();
    }

    printf("\033[u\e[0;34m%s\e[0m", tt_logo);
    fflush(stdout);

    for (; printed_rows > 0; printed_rows --)
        printf("\n");
    fflush(stdout);
}

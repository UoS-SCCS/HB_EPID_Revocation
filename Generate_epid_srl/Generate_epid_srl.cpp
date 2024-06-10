/***************************************************************************
 * File:        Generate_epid_srl.cpp
 * Description: Code to generate test revocation lists for hash based EPID
 *
 * Author:      Chris Newton
 * Created:     Tuesday 14 November 2023
 *
 * (C) Copyright 2023, University of Surrey.
 *
 ****************************************************************************/
#include <cmath>
#include <cinttypes>
#include <cstring>
#include <thread>
#include <exception>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "Io_utils.h"
#include "Clock_utils.h"

#include "picnic.h"
extern "C" {
#include "picnic_types.h"
#include "picnic3_impl.h"
}

#include "Hbgs_param.h"
#include "Picnic_mpc_functions.h"
#include "Mpc_utils.h"
#include "Mpc_parameters.h"
#include "Lowmc64.h"
#include "Hb_epid_revocation_lists.h"
#include "Generate_epid_srl.h"

bool generate_epid_revocation_lists(Epid_sig_rl &srlist, size_t n_srl_entries)
{
    paramset_t paramset;
    get_param_set(get_picnic_parameter_set_id(), &paramset);

    Lowmc_matrices::assign_lowmc_matrices();

    Lowmc_state_words64 first{ 0 };
    Lowmc_state_words64 sk{ 0 };
    Lowmc_state_words64 second{ 0 };

    for (size_t i = 0; i < n_srl_entries; ++i) {
        if (picnic_random_bytes(reinterpret_cast<uint8_t *>(first),
              Mpc_parameters::lowmc_state_bytes_)
            != 0) {
            std::cerr << "Failed to generate first\n";
            return false;
        }
        zeroTrailingBits(reinterpret_cast<uint8_t *>(first),
          Mpc_parameters::lowmc_state_bits_);

        if (picnic_random_bytes(reinterpret_cast<uint8_t *>(sk),
              Mpc_parameters::lowmc_state_bytes_)
            != 0) {
            std::cerr << "Failed to generate sk\n";
            return false;
        }
        zeroTrailingBits(
          reinterpret_cast<uint8_t *>(sk), Mpc_parameters::lowmc_state_bits_);

        lowmc64(second, sk, first, &paramset);

        Epid_rl_entry<Sigrl> srle(first, second);
        srlist.add(srle);
    }

    return true;
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        usage(std::cout, argv[0]);
        return EXIT_FAILURE;
    }

    std::string base_dir{ argv[1] };
    std::string revocation_file{ argv[2] };
    size_t n_srl_entries = std::strtoul(argv[3], nullptr, 10);

    Epid_sig_rl sig_rl;

    generate_epid_revocation_lists(sig_rl, n_srl_entries);

    std::string filename = make_filename(base_dir, revocation_file);
    std::string srl_filename = filename + '.' + sigrl_file_ext;
    std::ofstream srl_os{ srl_filename };
    if (!srl_os) {
        std::cerr << "Unable to create the file " << srl_filename << '\n';
        return EXIT_FAILURE;
    }
    sig_rl.print_rl(srl_os);
    srl_os.close();
}

void usage(std::ostream &os, std::string program)
{
    os << green
       << "A program to generate and save the revocation data for the hash "
          "based EPID protocol.\n"
       << normal << program
       << " <base dir> <revocation file name> <number of srl entries> \n\n";
}

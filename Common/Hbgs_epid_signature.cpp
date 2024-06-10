/***************************************************************************
 * File:        Hbgs_epid_signature.cpp
 * Description: The Hash-based EPID signature class
 *
 * Author:      Chris Newton
 * Created:     Monday 1 April 2024
 *
 * (C) Copyright 2024, University of Surrey.
 *
 ****************************************************************************/

#include <cstring>
#include "Io_utils.h"
#include "Hbgs_epid_signature.h"

Revocation_signature::Revocation_signature(std::string const &srl_file,
  uint8_t const *str, Lowmc_state_words64_const_ptr sid,
  Lowmc_state_words64_const_ptr sst,
  Lowmc_state_words64_const_ptr r_value) noexcept
  : srl_reference_(srl_file)
{
    std::memcpy(str_, str, Mpc_parameters::nonce_size_bytes_);
    std::memcpy(sid_, sid, Mpc_parameters::lowmc_state_bytes_);
    std::memcpy(cd_.sst_, sst, Mpc_parameters::lowmc_state_bytes_);
    std::memcpy(r_value_, r_value, Mpc_parameters::lowmc_state_bytes_);
}
/*
bool Revocation_signature::print_rs(std::ostream &os) const noexcept
{
    os << reference_prefix << ' ' << srl_reference_ << '\n';
    os << nonce_prefix << ' ' << Mpc_parameters::nonce_size_bytes_ << '\n';
    print_buffer(os, str_, Mpc_parameters::nonce_size_bytes_);
    os << '\n' << state_prefix << '\n';
    print_lowmc_state_words64(os, sid_);
    os << '\n';
    print_lowmc_state_words64(os, r_value_);
    os << '\n' << verification_data_prefix << '\n';
    print_lowmc_state_words64(os, cd_.sst_);
    os << '\n';


    return true;
}

bool Revocation_signature::read_rs(std::istream &is) noexcept
{
    std::string temp;
    std::getline(is, temp);
    // check and retrieve filename (srl_reference_)
    is >> temp;
    if (temp != nonce_prefix) {
        std::cerr << "Revocation_signature::read_rs: error reading the "
                     "parameter set - incorrect prefix\n";
        std::cerr << "Expected: " << nonce_prefix << " Read: " << temp << '\n';
        return false;
    }

    size_t str_size{ 0 };
    is >> str_size;
    if (str_size != Mpc_parameters::nonce_size_bytes_) {
        std::cerr
          << "Revocation_signature::read_rs: incorrect array size for nonce\n";
    }
    is >> std::ws;
    read_hex_bytes(is, str_, Mpc_parameters::nonce_size_bytes_);
    is >> std::ws;
    std::getline(is, temp);
    if (temp != state_prefix) {
        std::cerr << "Revocation_signature::read_rs: error reading the "
                     "lowmc states - incorrect prefix\n";
        std::cerr << "Expected: " << state_prefix << " Read: " << temp << '\n';

        return false;
    }
    read_lowmc_state_words64(is, sid_);
    read_lowmc_state_words64(is, r_value_);
    is >> std::ws;
    std::getline(is, temp);
    if (temp != verification_data_prefix) {
        std::cerr << "Revocation_signature::read_rs: error reading the "
                     "verification data - incorrect prefix\n";
        std::cerr << "Expected: " << verification_data_prefix
                  << " Read: " << temp << '\n';
        return false;
    }
    read_lowmc_state_words64(is, cd_.sst_);


    return true;
}

bool print_revocation_signature(Revocation_signature const &sig,
  std::string const &base_dir, std::string const &sig_name)
{
    std::string filename = make_filename(base_dir, sig_name);
    std::string sig_filename = filename + '.' + signature_file_ext;
    std::ofstream sig_os{ sig_filename };
    if (!sig_os) {
        std::cerr << "Unable to open the file " << sig_filename << '\n';
        return false;
    }

    bool print_ok = sig.print_rs(sig_os);

    sig_os.close();

    return print_ok;
}

bool read_revocation_signature(Revocation_signature &sig,
  std::string const &base_dir, std::string const &sig_name)
{
    std::string filename = make_filename(base_dir, sig_name);
    std::string sig_filename = filename + '.' + signature_file_ext;
    std::ifstream sig_is{ sig_filename };
    if (!sig_is) {
        std::cerr << "Unable to open the file " << sig_filename << '\n';
        return false;
    }

    bool read_ok = sig.read_rs(sig_is);

    sig_is.close();

    return read_ok;
}
*/
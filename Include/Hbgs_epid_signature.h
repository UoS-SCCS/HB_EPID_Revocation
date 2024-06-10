/***************************************************************************
 * File:        Hbgs_epid_signature.h
 * Description: The Hash-based EPID signature class
 *
 * Author:      Chris Newton
 * Created:     Saturday 30 March 2024
 *
 * (C) Copyright 2024, University of Surrey.
 *
 ****************************************************************************/

#ifndef HB_EPID_SIGNATURE_H
#define HB_EPID_SIGNATURE_H

#include <string>
#include <vector>

#include "Hbgs_param.h"
#include "Lowmc64.h"
#include "Hb_epid_revocation_lists.h"

/*
const std::string signature_file_ext{ "sig" };


const std::string reference_prefix{ "SRL:" };
const std::string nonce_prefix{ "Nonce:" };
const std::string state_prefix{ "Lowmc states:" };
const std::string verification_data_prefix{ "Data to verify:" };
const std::string signature_prefix{ "Signature:" };
const size_t signature_line_length{ 64 };
*/

struct Revocation_checking_data
{
    Lowmc_state_words64 sst_{ 0 };
    Epid_a_rl a_j_;
};

class Revocation_signature
{
  public:
    using sig_buffer = std::vector<uint8_t>;
    Revocation_signature() = default;
    Revocation_signature(std::string const &srl_file, uint8_t const *str,
      Lowmc_state_words64_const_ptr sid, Lowmc_state_words64_const_ptr sst,
      Lowmc_state_words64_const_ptr r_value) noexcept;
    Revocation_signature(Revocation_signature const &gle) = delete;
    Revocation_signature &operator=(Revocation_signature const &gle) = delete;
    uint8_t const *str() const { return str_; }
    Lowmc_state_words64_const_ptr sid() const { return sid_; }
    Lowmc_state_words64_const_ptr sst() const { return cd_.sst_; }
    Lowmc_state_words64_const_ptr rv() const { return r_value_; }
    Revocation_checking_data &rev_check() { return cd_; }
    Revocation_checking_data const &rev_ceck() const noexcept { return cd_; }
    size_t siglen() const noexcept { return signature_.size(); }
    sig_buffer &sig_buf() noexcept { return signature_; }
    sig_buffer const &sig_buf() const noexcept { return signature_; }
    uint8_t *sig() noexcept { return signature_.data(); }
    uint8_t const *sig() const noexcept { return signature_.data(); }
    bool print_rs(std::ostream &os) const noexcept;
    bool read_rs(std::istream &is) noexcept;

  private:
    std::string srl_reference_{ "unset" };// For these tests this will be a
                                          // filename (relative to base_dir)
    // Σ = (str, sid, r, sst, ∀j∈[1,J] Aj ,πE).
    uint8_t str_[Mpc_parameters::nonce_size_bytes_]{ 0 };
    Lowmc_state_words64 sid_{ 0 };
    Lowmc_state_words64 r_value_{ 0 };
    Revocation_checking_data cd_;
    // \pi_E
    sig_buffer signature_{};
};

bool print_revocation_signature(Revocation_signature const &sig,
  std::string const &base_dir, std::string const &sig_name);

bool read_revocation_signature(Revocation_signature &sig,
  std::string const &base_dir, std::string const &sig_name);

#endif

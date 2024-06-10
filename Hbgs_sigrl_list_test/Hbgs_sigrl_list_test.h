/***************************************************************************
 * File:        Hbgs_sigrl_list_test.h
 * Description: Code for testing a list of sigRL entries
 *
 * Author:      Chris Newton
 * Created:     Sunday 14 January 2024, updated for new scheme 28 March 2024
 *
 * (C) Copyright 2024, University of Surrey.
 *
 ****************************************************************************/

#ifndef HBGS_SIGRL_LIST_TEST_H
#define HBGS_SIGRL_LIST_TEST_H

#include "Clock_utils.h"

#include "Hbgs_param.h"
#include "Hb_epid_revocation_lists.h"
#include "Mpc_sigrl_entries.h"
#include "Mpc_working_data.h"
#include "Mpc_signature_utils.h"
#include "Mpc_parameters.h"

void usage(std::ostream &os, std::string const &program);

enum mask_values : uint8_t { mask_s = 1, mask_sku = 2 };

class Hbgs_sigrl_list_test
{
  public:
    Hbgs_sigrl_list_test() = delete;
    Hbgs_sigrl_list_test(Lowmc_state_words64_const_ptr sid,
      Lowmc_state_words64_const_ptr r_value, Epid_sig_rl const &srl) noexcept;
    void set_sku(
      Lowmc_state_words64_const_ptr sk_u) noexcept;// Only used for signing
    Tape_offset set_offsets(Tape_offset const &of) noexcept;
    void compute_salt_and_root_seed(uint8_t *salt_and_root, size_t s_and_r_len,
      uint8_t const *nonce) noexcept;
    void compute_aux_tape_sign(randomTape_t *tapes,
      [[maybe_unused]] Mpc_working_data &mpc_wd, size_t t) noexcept;
    void compute_aux_tape_verify(randomTape_t *tapes,
      [[maybe_unused]] Signature_data const &sig_data, size_t t) noexcept;
    void get_aux_bits(
      uint8_t *aux_bits, randomTape_t *tapes, size_t t) noexcept;
    void set_aux_bits(
      randomTape_t *tapes, Signature_data const &sig_data, size_t t) noexcept;
    int mpc_simulate_sign(randomTape_t *tapes, Mpc_working_data &mpc_wd,
      shares_t *tmp_shares, size_t t) noexcept;
    void commit_v_sign(
      Commitment_data2 &c2, Mpc_working_data const &wd, size_t t);
    void commit_v_verify(Commitment_data2 &c2, Signature_data const &sig_data,
      msgs_t const *msgs, size_t t);
    void calculate_hcp(uint8_t *challenge_hash, Signature_data const &sig_data,
      Commitment_data2 &cd2, uint8_t const *message_digest,
      uint8_t const *nonce) noexcept;
    int mpc_simulate_and_verify(randomTape_t *tapes,
      Signature_data const &sig_data, msgs_t *msgs,
      Revocation_checking_data const &cd, shares_t *tmp_shares,
      size_t t) noexcept;
    void save_proof_data(
      Proof2 *proof, Mpc_working_data const &mpc_wd, size_t t) const;
    void reset();

    constexpr static Mpc_param sst_mpc_param_{ Mpc_lowmc64::aux_bits_, 1, 1,
        1 };

    constexpr static Mpc_param single_entry_mpc_param_{
        Mpc_sigrl_entry::aux_bits_, 0, 2, 1
    };

    static Mpc_param mpc_param_;

  private:
    Lowmc_state_words64 sid_{ 0 };
    Lowmc_state_words64 sk_u_{ 0 };
    Lowmc_state_words64 r_value_{ 0 };

    Epid_sig_rl const &srl_;

    paramset_t paramset_;
    Tape_offset sku_mask_offset_{ null_offset };

    std::vector<Tape_offset> sigrl_entry_offsets_{};

    constexpr static Mpc_proof_indices pi_ =
      indices_from_mpc_param(sst_mpc_param_);

    Mpc_lowmc64 sst_lowmc_;
};

struct Mpc_time_point
{
    std::string type_;
    float time_;
};

using Mpc_timings = std::vector<Mpc_time_point>;

struct Mpc_timing_data
{
    F_timer_ms timer_;
    Mpc_timings times_;
};


#endif

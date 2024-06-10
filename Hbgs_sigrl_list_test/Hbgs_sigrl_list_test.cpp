/***************************************************************************
 * File:        Hbgs_sigrl_list_test.cpp
 * Description: Code for testing a list of sigRL entries
 *
 * Author:      Chris Newton
 * Created:     Sunday 14 January 2024, updated for new scheme 28 March 2024
 *
 * (C) Copyright 2024, University of Surrey.
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

#include "picnic.h"
extern "C" {
#include "picnic_types.h"
#include "picnic3_impl.h"
}

#include "Hbgs_param.h"
#include "Picnic_mpc_functions.h"
#include "Mpc_utils.h"
#include "Mpc_parameters.h"
#include "Mpc_signature_utils.h"
#include "Mpc_working_data.h"
#include "Mpc_sign.h"
#include "Mpc_verify.h"
#include "Hb_epid_revocation_lists.h"
#include "Hbgs_epid_signature.h"
#include "Hbgs_sigrl_list_test.h"

//#define DEBUG_VARIABLES
//#define DEBUG_AUX
//#define DEBUG_MPC_INPUTS
//#define DEBUG_OUTPUTS
#define MINIMAL_PRINTING

Mpc_param Hbgs_sigrl_list_test::mpc_param_{
    Hbgs_sigrl_list_test::single_entry_mpc_param_
};

Hbgs_sigrl_list_test::Hbgs_sigrl_list_test(Lowmc_state_words64_const_ptr sid,
  Lowmc_state_words64_const_ptr r_value, Epid_sig_rl const &srl) noexcept
  : srl_(srl)
{
    std::memcpy(sid_, sid, Mpc_parameters::lowmc_state_bytes_);
    std::memcpy(r_value_, r_value, Mpc_parameters::lowmc_state_bytes_);

    mpc_param_ =
      scale_mpc_param(single_entry_mpc_param_, srl_.size()) + sst_mpc_param_;
    sigrl_entry_offsets_.resize(srl_.size());

    get_param_set(get_picnic_parameter_set_id(), &paramset_);
}
void Hbgs_sigrl_list_test::set_sku(
  Lowmc_state_words64_const_ptr sk_u) noexcept// Only used for signing
{
    std::memcpy(sk_u_, sk_u, Mpc_parameters::lowmc_state_bytes_);
}

Tape_offset Hbgs_sigrl_list_test::set_offsets(Tape_offset const &of) noexcept
{

#ifdef DEBUG_VARIABLES
    // Just for testing, this gets called by sign and verify
    std::cout << green << "\n              sku: ";
    print_lowmc_state_words64(std::cout, sk_u_);
    std::cout << '\n';
    std::cout << "            sid: ";
    print_lowmc_state_words64(std::cout, sid_);
    std::cout << normal << '\n';
#endif

    Tape_offset next_offset = of;


    sku_mask_offset_ = next_offset;
    next_offset += paramset_.stateSizeBits;

    next_offset = sst_lowmc_.set_offsets(next_offset);

    Mpc_sigrl_entry entry;
    Tape_offset entry_offset_delta = entry.set_offsets(0);
    auto srl_size = static_cast<uint32_t>(srl_.size());
    sigrl_entry_offsets_[0] = next_offset;
    for (uint32_t e = 1; e < srl_size; ++e) {
        sigrl_entry_offsets_[e] = next_offset + e * entry_offset_delta;
    }

    return next_offset + srl_size * entry_offset_delta;
}

void Hbgs_sigrl_list_test::compute_salt_and_root_seed(
  uint8_t *salt_and_root, size_t s_and_r_len, uint8_t const *nonce) noexcept
{
    HashInstance ctx;

    HashInit(&ctx, &paramset_, HASH_PREFIX_NONE);
    HashUpdate(
      &ctx, reinterpret_cast<uint8_t *>(sid_), paramset_.stateSizeBytes);
    HashUpdate(&ctx, nonce, Mpc_parameters::nonce_size_bytes_);
    HashUpdateIntLE(&ctx, static_cast<uint16_t>(paramset_.stateSizeBits));
    HashFinal(&ctx);
    HashSqueeze(&ctx, salt_and_root, s_and_r_len);
}

void Hbgs_sigrl_list_test::compute_aux_tape_sign(
  randomTape_t *tapes, Mpc_working_data &mpc_wd, size_t t) noexcept
{
    randomTape_t *current_tape_ptr = &tapes[t];

    Lowmc_state_words64 null_mask{ 0 };
    sst_lowmc_.compute_aux_tape(current_tape_ptr, null_mask, null_mask,
      reinterpret_cast<Word *>(mpc_wd.mpc_inputs_[0][t]), &paramset_);


    Lowmc_state_words64 sku_mask = { 0 };
    if (sku_mask_offset_ != null_offset) {
        get_mask_from_tapes(
          sku_mask, current_tape_ptr, sku_mask_offset_, &paramset_);
    }

    xor64(reinterpret_cast<Word *>(mpc_wd.mpc_inputs_[0][t]), sku_mask);

    Mpc_proof_indices cpi = pi_;
    for (size_t e = 0; e < srl_.size(); ++e) {
        size_t mpc_base = cpi.mpc_input_index_;
        Mpc_sigrl_entry mpc_entry;
        mpc_entry.set_offsets(sigrl_entry_offsets_[e]);

        mpc_entry.compute_aux_tape(current_tape_ptr,
          reinterpret_cast<Word *>(mpc_wd.mpc_inputs_[mpc_base][t]),
          reinterpret_cast<Word *>(mpc_wd.mpc_inputs_[mpc_base + 1][t]),
          &paramset_);

        auto *masked_sku =
          reinterpret_cast<Word *>(mpc_wd.mpc_inputs_[mpc_base][t]);
        xor64(masked_sku, sku_mask);

        cpi = indices_add_mpc_param(cpi, single_entry_mpc_param_);
    }
}

void Hbgs_sigrl_list_test::compute_aux_tape_verify(randomTape_t *tapes,
  [[maybe_unused]] Signature_data const &sig_data, size_t t) noexcept
{
    randomTape_t *current_tape_ptr = &tapes[t];

    Lowmc_state_words64 null_mask{ 0 };
    sst_lowmc_.compute_aux_tape(
      current_tape_ptr, null_mask, null_mask, nullptr, &paramset_);


    for (size_t e = 0; e < srl_.size(); ++e) {
        Mpc_sigrl_entry mpc_entry;
        mpc_entry.set_offsets(sigrl_entry_offsets_[e]);
        mpc_entry.compute_aux_tape(
          current_tape_ptr, nullptr, nullptr, &paramset_);
    }
}

void Hbgs_sigrl_list_test::get_aux_bits(
  uint8_t *aux_bits, randomTape_t *tapes, size_t t) noexcept
{
    randomTape_t *current_tape_ptr = &tapes[t];

    uint32_t aux_pos = 0;
    sst_lowmc_.get_aux_bits(aux_bits, aux_pos, current_tape_ptr);

    for (size_t e = 0; e < srl_.size(); ++e) {
        Mpc_sigrl_entry mpc_entry;
        mpc_entry.set_offsets(sigrl_entry_offsets_[e]);
        mpc_entry.get_aux_bits(aux_bits, aux_pos, current_tape_ptr);
    }
#ifdef DEBUG_AUX
    std::cout << "get - Tape: " << t << '\n' << magenta;
    print_buffer(std::cout, aux_bits, mpc_param_.aux_size_bytes_);
    std::cout << normal << '\n';
#endif
}

void Hbgs_sigrl_list_test::set_aux_bits(
  randomTape_t *tapes, Signature_data const &sig_data, size_t t) noexcept
{
    randomTape_t *current_tape_ptr = &tapes[t];
    uint32_t aux_pos = 0;
    sst_lowmc_.set_aux_bits(
      current_tape_ptr, aux_pos, sig_data.proofs_[t]->aux_);

    for (size_t e = 0; e < srl_.size(); ++e) {
        Mpc_sigrl_entry mpc_entry;
        mpc_entry.set_offsets(sigrl_entry_offsets_[e]);
        mpc_entry.set_aux_bits(
          current_tape_ptr, aux_pos, sig_data.proofs_[t]->aux_);
    }
#ifdef DEBUG_AUX
    std::cout << "set - Tape: " << t << '\n' << magenta;
    print_buffer(
      std::cout, sig_data.proofs_[t]->aux_, mpc_param_.aux_size_bytes_);
    std::cout << normal << '\n';
#endif
}

int Hbgs_sigrl_list_test::mpc_simulate_sign(randomTape_t *tapes,
  Mpc_working_data &mpc_wd, shares_t *tmp_shares, size_t t) noexcept
{
    // std::cout << "\nRound " << t;
    randomTape_t *current_tape_ptr = &tapes[t];

    Lowmc_state_words64 sku_mask = { 0 };
    if (sku_mask_offset_ != null_offset) {
        get_mask_from_tapes(
          sku_mask, current_tape_ptr, sku_mask_offset_, &paramset_);
    }

    Lowmc_state_words64 masked_sku = { 0 };
    xor64(masked_sku, sk_u_, sku_mask);

#ifdef DEBUG_MPC_INPUTS
    std::cout << "\n       masked sku: ";
    print_lowmc_state_words64(std::cout, masked_sku);
    std::cout << '\n';
#endif

    xor64(reinterpret_cast<Word *>(mpc_wd.mpc_inputs_[0][t]), masked_sku);
    /*
        auto remasked_sku_input = (Word *)mpc_wd.mpc_inputs_[0][t];
        xor64(remasked_sku_input, masked_sku);
    */
    int rv{ 0 };
    rv = sst_lowmc_.mpc_simulate(
      reinterpret_cast<Word *>(mpc_wd.mpc_inputs_[0][t]), sid_,
      current_tape_ptr, tmp_shares, &mpc_wd.msgs_[t],
      reinterpret_cast<Word *>(mpc_wd.outputs_[0][t]), &paramset_);
#ifdef DEBUG_OUTPUTS
    std::cout << green << "    simulated sst: ";
    print_lowmc_state_words64(std::cout, (Word *)mpc_wd.outputs_[0][t]);
    std::cout << normal << '\n';
#endif

    Mpc_proof_indices cpi = pi_;
    for (size_t e = 0; e < srl_.size(); ++e) {
        size_t mpc_base = cpi.mpc_input_index_;
        size_t output_base = cpi.output_index_;
        Mpc_sigrl_entry mpc_entry;
        mpc_entry.set_offsets(sigrl_entry_offsets_[e]);

        // Re-mask sku and save for verify
        auto *remasked_sku_input =
          reinterpret_cast<Word *>(mpc_wd.mpc_inputs_[mpc_base][t]);
        xor64(remasked_sku_input, masked_sku);

#ifdef DEBUG_MPC_INPUTS
        std::cout << "\n     remasked sku: ";
        print_lowmc_state_words64(std::cout, remasked_sku_input);
        std::cout << '\n';
#endif
        Epid_sigrl_entry const &entry = srl_[e];
        rv = mpc_entry.mpc_simulate(remasked_sku_input, entry.first(),
          reinterpret_cast<Word *>(mpc_wd.mpc_inputs_[mpc_base + 1][t]),
          r_value_, current_tape_ptr, tmp_shares, &mpc_wd.msgs_[t],
          reinterpret_cast<Word *>(mpc_wd.outputs_[output_base][t]),
          &paramset_);

        cpi = indices_add_mpc_param(cpi, single_entry_mpc_param_);

#ifdef DEBUG_OUTPUTS
        std::cout << red << "    simulated a_j: ";
        print_lowmc_state_words64(
          std::cout, reinterpret_cast<Word *>(mpc_wd.outputs_[output_base][t]));
        std::cout << normal << '\n';
#endif
    }
    return rv;
}

int Hbgs_sigrl_list_test::mpc_simulate_and_verify(randomTape_t *tapes,
  Signature_data const &sig_data, msgs_t *msgs,
  Revocation_checking_data const &cd, shares_t *tmp_shares, size_t t) noexcept
{
    // std::cout << "\nRound " << t;
    randomTape_t *current_tape_ptr = &tapes[t];
    int rv{ 0 };

    Lowmc_state_words64 sst{ 0 };
    rv = sst_lowmc_.mpc_simulate(
      reinterpret_cast<Word *>(sig_data.proofs_[t]->mpc_inputs_[0]), sid_,
      current_tape_ptr, tmp_shares, msgs, sst, &paramset_);
    if (rv != 0) {
        std::cerr << "MPC simulation of sst failed for round " << t
                  << ", signature invalid\n";
        return EXIT_FAILURE;
    }

#ifdef DEBUG_OUTPUTS
    std::cout << green << "  simulated sst - v: ";
    print_lowmc_state_words64(std::cout, sst);
    std::cout << normal << '\n';
#endif

    if (memcmp(sig_data.proofs_[t]->outputs_[0], sst, paramset_.stateSizeBytes)
        != 0) {
        std::cerr << "Verification failed - the simulated output for sst "
                     "does not match\n";
        // return EXIT_FAILURE;
    }
    if (memcmp(sst, cd.sst_, paramset_.stateSizeBytes) != 0) {
        std::cerr << "Verification failed - the simulated output for sst "
                     "does not match with the given value for sst\n";
        // return EXIT_FAILURE;
    }


    Epid_arl_entry output{};
    Mpc_proof_indices cpi = pi_;
    for (size_t e = 0; e < srl_.size(); ++e) {
        size_t mpc_base = cpi.mpc_input_index_;
        size_t output_base = cpi.output_index_;
        Mpc_sigrl_entry mpc_entry;
        mpc_entry.set_offsets(sigrl_entry_offsets_[e]);

#ifdef DEBUG_MPC_INPUTS
        std::cout << "\n       masked sku: ";
        print_lowmc_state_words64(
          std::cout, (Word *)sig_data.proofs_[t]->mpc_inputs_[mpc_base]);
        std::cout << '\n';
#endif
        Epid_sigrl_entry const &srl_entry = srl_[e];
        rv = mpc_entry.mpc_simulate(
          reinterpret_cast<Word *>(sig_data.proofs_[t]->mpc_inputs_[mpc_base]),
          srl_entry.first(),
          reinterpret_cast<Word *>(
            sig_data.proofs_[t]->mpc_inputs_[mpc_base + 1]),
          r_value_, current_tape_ptr, tmp_shares, msgs, output.entry(),
          &paramset_);
        if (rv != 0) {
            std::cerr << "MPC simulation failed for round " << t
                      << ", signature invalid\n";
            return EXIT_FAILURE;
        }

#ifdef DEBUG_OUTPUTS
        std::cout << red << "simulated entry - v: ";
        print_lowmc_state_words64(std::cout, output.entry());
        std::cout << normal << '\n';
#endif

        if (memcmp(sig_data.proofs_[t]->outputs_[output_base], output.entry(),
              paramset_.stateSizeBytes)
            != 0) {
            std::cerr << "Verification failed - the simulated outputs for a_j "
                         "do not match\n";
            // return EXIT_FAILURE;
        }

        if (memcmp(cd.a_j_[e].entry(), output.entry(), paramset_.stateSizeBytes)
            != 0) {
            std::cerr
              << "Verification failed - the output for a_j does not match\n";
            return EXIT_FAILURE;
        }

        cpi = indices_add_mpc_param(cpi, single_entry_mpc_param_);
    }
    return EXIT_SUCCESS;
}


void Hbgs_sigrl_list_test::commit_v_sign(
  Commitment_data2 &c2, Mpc_working_data const &mpc_wd, size_t t)
{
    HashInstance ctx;

    msgs_t *msgs = &mpc_wd.msgs_[t];

    HashInit(&ctx, &paramset_, HASH_PREFIX_NONE);
    HashUpdate(&ctx, mpc_wd.mpc_inputs_[0][t], paramset_.stateSizeBytes);
    for (size_t i = 0; i < paramset_.numMPCParties; i++) {
        auto msgs_size =
          static_cast<size_t>(numBytes(static_cast<uint32_t>(msgs->pos)));
        HashUpdate(&ctx, msgs->msgs[i], msgs_size);
    }
    HashFinal(&ctx);
    HashSqueeze(&ctx, c2.Cv.hashes[t], paramset_.digestSizeBytes);
}

void Hbgs_sigrl_list_test::commit_v_verify(Commitment_data2 &c2,
  Signature_data const &sig_data, msgs_t const *msgs, size_t t)
{
    HashInstance ctx;

    HashInit(&ctx, &paramset_, HASH_PREFIX_NONE);
    HashUpdate(
      &ctx, sig_data.proofs_[t]->mpc_inputs_[0], paramset_.stateSizeBytes);
    for (size_t i = 0; i < paramset_.numMPCParties; i++) {
        auto msgs_size =
          static_cast<size_t>(numBytes(static_cast<uint32_t>(msgs->pos)));
        HashUpdate(&ctx, msgs->msgs[i], msgs_size);
    }
    HashFinal(&ctx);
    HashSqueeze(&ctx, c2.Cv.hashes[t], paramset_.digestSizeBytes);
}

void Hbgs_sigrl_list_test::calculate_hcp(uint8_t *challenge_hash,
  Signature_data const &sig_data, Commitment_data2 &cd2,
  uint8_t const *message_digest, uint8_t const *nonce) noexcept
{
    HashInstance ctx;

    HashInit(&ctx, &paramset_, HASH_PREFIX_NONE);
    for (size_t t = 0; t < paramset_.numMPCRounds; t++) {
        HashUpdate(&ctx, cd2.Ch.hashes[t], paramset_.digestSizeBytes);
    }

    HashUpdate(&ctx, cd2.treeCv->nodes[0], Mpc_parameters::digest_size_bytes_);
    HashUpdate(&ctx, sig_data.mpc_pd_.salt_, paramset_.saltSizeBytes);
    HashUpdate(&ctx, message_digest, Mpc_parameters::digest_size_bytes_);
    HashUpdate(&ctx, nonce, Mpc_parameters::nonce_size_bytes_);
    HashFinal(&ctx);
    HashSqueeze(&ctx, challenge_hash, Mpc_parameters::challenge_hash_bytes_);
}

void Hbgs_sigrl_list_test::save_proof_data(
  Proof2 *proof, Mpc_working_data const &mpc_wd, size_t t) const
{
    for (size_t i = 0; i < proof->mpc_inputs_.size(); ++i) {
        memcpy(proof->mpc_inputs_[i],
          mpc_wd.mpc_inputs_[i][t],
          paramset_.stateSizeBytes);
    }
    for (size_t i = 0; i < proof->outputs_.size(); ++i) {
        memcpy(
          proof->outputs_[i], mpc_wd.outputs_[i][t], paramset_.stateSizeBytes);
    }
    for (size_t i = 0; i < proof->inputs_.size(); ++i) {
        memcpy(
          proof->inputs_[i], mpc_wd.inputs_[i][t], paramset_.stateSizeBytes);
    }
}

void Hbgs_sigrl_list_test::reset()
{
    // std::memset(r_value_, 0, paramset_.stateSizeBytes);
    // std::memset(sid_, 0, paramset_.stateSizeBytes);
    std::memset(sk_u_, 0, paramset_.stateSizeBytes);

    sku_mask_offset_ = null_offset;
}

void calculate_sid(Lowmc_state_words64_ptr sid, uint8_t const *str,
  uint8_t const *msg_digest, paramset_t *params) noexcept
{
    HashInstance ctx;

    auto *sid8 = reinterpret_cast<uint8_t *>(sid);

    HashInit(&ctx, params, HASH_PREFIX_NONE);
    HashUpdate(&ctx, str, Mpc_parameters::nonce_size_bytes_);
    HashUpdate(&ctx, msg_digest, Mpc_parameters::digest_size_bytes_);
    HashFinal(&ctx);
    HashSqueeze(&ctx, sid8, Mpc_parameters::lowmc_state_bytes_);

    zeroTrailingBits(sid8, Mpc_parameters::lowmc_state_bits_);
}


//=============================================================================
// Now do the test

int generate_test_data(uint8_t const *str, uint8_t const *msg_digest,
  Lowmc_state_words64_ptr users_sid, Lowmc_state_words64_ptr r_value,
  Lowmc_state_words64_ptr users_sk, Lowmc_state_words64_ptr users_sst,
  paramset_t *params)
{
    // Generate random r_value
    if (picnic_random_bytes(reinterpret_cast<uint8_t *>(r_value),
          Mpc_parameters::lowmc_state_bytes_)
        != 0) {
        std::cerr << "Failed to generate the test input_s\n";
        return -1;
    }
    zeroTrailingBits(
      reinterpret_cast<uint8_t *>(r_value), Mpc_parameters::lowmc_state_bits_);

    // Generate users_sk at random
    if (picnic_random_bytes(reinterpret_cast<uint8_t *>(users_sk),
          Mpc_parameters::lowmc_state_bytes_)
        != 0) {
        std::cerr << "Failed to generate the test users_sk\n";
        return -1;
    }
    zeroTrailingBits(
      reinterpret_cast<uint8_t *>(users_sk), Mpc_parameters::lowmc_state_bits_);

    calculate_sid(users_sid, str, msg_digest, params);

    lowmc64(users_sst, users_sk, users_sid, params);

#ifndef MINIMAL_PRINTING
    std::cout << green << "\n     gtd users sk: ";
    print_lowmc_state_words64(std::cout, users_sk);
    std::cout << "\n          gtd sid: ";
    print_lowmc_state_words64(std::cout, users_sid);
    std::cout << "\n          gtd sst: ";
    print_lowmc_state_words64(std::cout, users_sst);
    std::cout << normal << '\n';
    Lowmc_state_words64 temp{ 0 };
    lowmc64(temp, users_sk, users_sid, params);
    std::cout << blue << "    gtd check sst: ";
    print_lowmc_state_words64(std::cout, temp);
    std::cout << normal << '\n';
#endif


    return EXIT_SUCCESS;
}

bool read_srl_data(
  Epid_sig_rl &sig_rl, std::string const &base_dir, std::string const &srl_name)
{
    std::string filename = make_filename(base_dir, srl_name);
    std::string srl_filename = filename + '.' + sigrl_file_ext;
    std::ifstream srl_is{ srl_filename };
    if (!srl_is) {
        std::cerr << "Unable to open the file " << srl_filename << '\n';
        return false;
    }

    bool read_status_ok = sig_rl.read_rl(srl_is);
    srl_is.close();

    if (!read_status_ok) {
        std::cerr << "Reading the sigRL data file failed\n";
        return false;
    }

#ifndef MINIMAL_PRINTING
    std::cout << sig_rl.list_name() << '\n' << sig_rl.size() << " entries\n";
#endif

    return true;
}

bool check_a_j_and_b_j(
  Epid_sig_rl const &srl, Revocation_signature &rsig, paramset_t *params)
{
    Lowmc_state_words64 test_value{};
    for (size_t e = 0; e < srl.size(); ++e) {
        lowmc64(test_value, srl[e].second(), rsig.rv(), params);
        if (memcmp(test_value, rsig.rev_check().a_j_[e].entry(),
              params->stateSizeBytes)
            == 0) {
            std::cerr << "Revocation check failed for entry " << e << '\n';
            return false;
        }
    }

    return true;
}


int sigrl_list_test(std::string const &base_dir,
  std::string const &srl_filename, bool make_it_fail)
{
    Lowmc_matrices::assign_lowmc_matrices();
    // Vaues for the signer
    Lowmc_state_words64 users_sk{ 0 };
    Lowmc_state_words64 r_value{ 0 };
    Lowmc_state_words64 users_sid{ 0 };
    Lowmc_state_words64 users_sst{ 0 };

    paramset_t paramset;
    get_param_set(get_picnic_parameter_set_id(), &paramset);

    Mpc_timing_data td;

    uint8_t str[Mpc_parameters::nonce_size_bytes_];

    if (picnic_random_bytes(str, Mpc_parameters::nonce_size_bytes_) != 0) {
        std::cerr << "Failed to generate the nonce (str) \n";
        return EXIT_FAILURE;
    }

    // A dummy digest for now
    uint8_t msg_digest[Mpc_parameters::digest_size_bytes_];
    if (picnic_random_bytes(msg_digest, Mpc_parameters::digest_size_bytes_)
        != 0) {
        std::cerr << "Failed to generate a message digest\n";
        return EXIT_FAILURE;
    }

    int ret = generate_test_data(
      str, msg_digest, users_sid, r_value, users_sk, users_sst, &paramset);
    if (ret != 0) {
        std::cerr << "Test data generation failed\n";
        return EXIT_FAILURE;
    }

    Epid_sig_rl srlist{};

    if (!read_srl_data(srlist, base_dir, srl_filename)) { return EXIT_FAILURE; }

    size_t failing_entry = 3;
    if (make_it_fail) {// Fix an entry in the list using users_sk
        Epid_sigrl_entry &entry = srlist[failing_entry];
        lowmc64(entry.second(), users_sk, entry.first(), &paramset);
        std::cout
          << "The revocation list has been configured to fail for entry number "
          << failing_entry << '\n';
    }

    Revocation_signature rsig(srl_filename, str, users_sid, users_sst, r_value);

#ifndef MINIMAL_PRINTING
    std::cout << green << "\n         users sk: ";
    print_lowmc_state_words64(std::cout, users_sk);
    std::cout << "\n              sid: ";
    print_lowmc_state_words64(std::cout, users_sid);
    std::cout << "\n              sst: ";
    print_lowmc_state_words64(std::cout, users_sst);
    std::cout << normal << '\n';
    Lowmc_state_words64 temp{ 0 };
    lowmc64(temp, users_sk, users_sid, &paramset);
    std::cout << blue << "        check sst: ";
    print_lowmc_state_words64(std::cout, temp);
    std::cout << normal << '\n';
#endif

    td.timer_.reset();
    (void)make_it_fail;
    // Now do the actual test
    Epid_a_rl &aj_list = rsig.rev_check().a_j_;
    aj_list.resize(srlist.size());

    for (size_t e = 0; e < srlist.size(); ++e) {
        aj_list[e] =
          calculate_epid_arl_entry(rsig.rv(), users_sk, srlist[e], &paramset);
    }

    uint8_t nonce[Mpc_parameters::nonce_size_bytes_];
    if (picnic_random_bytes(
          reinterpret_cast<uint8_t *>(nonce), Mpc_parameters::nonce_size_bytes_)
        != 0) {
        std::cerr << "Failed to generate the nonce\n";
        return EXIT_FAILURE;
    }

    Hbgs_sigrl_list_test hbgs_sigrl_list_test(users_sid, r_value, srlist);

    hbgs_sigrl_list_test.set_sku(users_sk);

    Signature_data sig_data{ Hbgs_sigrl_list_test::mpc_param_ };
    if (!sig_data.is_initialised_) {
        std::cerr << "Failed to initialise the signature data\n";
        return EXIT_FAILURE;
    }

    ret =
      generate_mpc_signature(hbgs_sigrl_list_test, msg_digest, str, sig_data);
    if (ret != EXIT_SUCCESS) {
        std::cerr << "Failed to create the signature\n ";
        return EXIT_FAILURE;
    }

    size_t max_signature_size =
      signature_size_estimate(Hbgs_sigrl_list_test::mpc_param_, paramset);

#ifndef MINIMAL_PRINTING
    std::cout << "\nMax signature length " << max_signature_size << " bytes\n ";
    std::cout << " Signing a message, with a nonce... " << std::flush;
#endif

    rsig.sig_buf().resize(max_signature_size);

    size_t signature_len =
      sig_data.serialise_signature(rsig.sig_buf().data(), max_signature_size);
    if (signature_len == 0) {
        std::cerr << "Failed to serialize signature\n" << std::flush;
        return EXIT_FAILURE;
    }

    rsig.sig_buf().shrink_to_fit();

    size_t total_signature_size =
      signature_len + Mpc_parameters::nonce_size_bytes_
      + (3 + rsig.rev_check().a_j_.size()) * Mpc_parameters::lowmc_state_bytes_;

#ifndef MINIMAL_PRINTING
    std::cout << " ... success, signature is " << signature_len << " bytes\n ";
#endif

    td.times_.emplace_back(Mpc_time_point{ "generate_srl_signature",
      static_cast<float>(td.timer_.get_duration() + 0.5F) });

    hbgs_sigrl_list_test.reset();

    // Now the verification
#ifndef MINIMAL_PRINTING
    std::cout << "Verifying signature ... \n" << std::flush;
#endif

    td.timer_.reset();

    bool verified_ok = check_a_j_and_b_j(srlist, rsig, &paramset);

    if (verified_ok) {
#ifndef MINIMAL_PRINTING
        std::cout << green << "Comparison of A_j and B_j tested OK\n"
                  << normal << std::flush;
#endif
        ret = verify_mpc_signature(hbgs_sigrl_list_test, rsig.sig_buf().data(),
          signature_len, msg_digest, str, rsig.rev_check());

        verified_ok = (ret == EXIT_SUCCESS);
    }

    td.times_.emplace_back(Mpc_time_point{ "verify_srl_signature",
      static_cast<float>(td.timer_.get_duration() + 0.5F) });


    if (!verified_ok) {
        std::cerr << "\nSignature verification failed\n";
        return EXIT_FAILURE;
    }

    std::cout << base_dir << '\t' << srl_filename << '\t' << srlist.size();
    for (auto const &tp : td.times_) { std::cout << '\t' << tp.time_; }
    std::cout << '\t'
              << static_cast<double>(total_signature_size) / (1024 * 1024)
              << std::endl;


#ifndef MINIMAL_PRINTING
    std::cout << red << " ... test successful\n" << normal << std::flush;
#endif

    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        usage(std::cerr, argv[0]);
        return EXIT_FAILURE;
    }

    std::string base_dir{ argv[1] };
    std::string srl_name{ argv[2] };

    bool make_it_fail{ false };
    char c = argv[3][0];// Just check the first character
    if (c == 'F' || c == 'f') {
        make_it_fail = true;
    } else if (c == 'T' || c == 't') {
        make_it_fail = false;
    } else {
        std::cerr << "pass must be either T or F\n";
        usage(std::cerr, argv[0]);
        return EXIT_FAILURE;
    }

    if (!model_parameters_check_ok()) {
        std::cerr << "The picnic and HBGS parameters are inconsistent.\n";
        print_hbgs_parameters(std::cerr);
        std::cerr << '\n';
        print_picnic_parameters(std::cerr);
        std::cerr << "\nQuitting ..\n";
        return EXIT_FAILURE;
    }

#ifndef MINIMAL_PRINTING
    print_hbgs_parameters(std::cout);
    std::cout << green
              << "\nChecked and consistent with the picnic parameter set used, "
              << picnic_get_param_name(get_picnic_parameter_set_id()) << normal
              << "\n\n";
#endif
    return sigrl_list_test(base_dir, srl_name, make_it_fail);
}

void usage(std::ostream &os, std::string const &program)
{
    os << green << "A test of a list of Sigrl_entries with different masks.\n\n"
       << normal << program << " <base dir> <list name> <pass T/F>\n\n";
}

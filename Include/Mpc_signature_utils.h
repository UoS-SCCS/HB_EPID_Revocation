/***************************************************************************
 * File:        Mpc_signature_utils.h
 * Description: Utilities used for MPC signatures
 *
 * Author:      Chris Newton
 *
 * Created:     Monday 21 February 2022
 *
 * (C) Copyright 2022, University of Surrey.
 *
 ****************************************************************************/

#ifndef MPC_SIGNATURE_UTILS_H
#define MPC_SIGNATURE_UTILS_H

#include <iostream>
#include <cmath>
#include <vector>

#include "picnic.h"
extern "C" {
#include "picnic_types.h"
#include "picnic3_impl.h"
#include "lowmc_constants.h"
}
#include "Mpc_parameters.h"

class Mpc_proof_data
{
  public:
    Mpc_proof_data() noexcept;
    ~Mpc_proof_data();

    bool is_initialised_{ false };
    uint8_t *salt_;
    uint8_t *iSeedInfo_{
        nullptr
    };// Info required to recompute the tree of all initial seeds
    size_t iSeedInfoLen_{ 0 };
    uint8_t *cvInfo_{ nullptr };// Info required to check commitments to views
                                // (reconstruct Merkle tree)
    size_t cvInfoLen_{ 0 };
    uint8_t *challengeHash_{ nullptr };
    uint16_t *challengeC_{ nullptr };
    uint16_t *challengeP_{ nullptr };
};

class Proof2// Derived from picnic proof2_t
{
  public:
    Proof2() = delete;
    Proof2(Mpc_param const &param);
    ~Proof2();

    bool is_initialised_{ false };
    uint8_t *seedInfo_{ nullptr };// Information required to compute the tree
                                  // with seeds of of all opened parties
    size_t seedInfoLen_{ 0 };// Length of seedInfo buffer
    uint8_t *C_{ nullptr };// Commitment to preprocessing step of unopened party
    uint8_t *aux_{
        nullptr
    };// Last party's correction bits; NULL if P[t] == N-1
    uint8_t *msgs_{ nullptr };// Broadcast messages of unopened party P[t]

    std::vector<uint8_t *> inputs_;// Inputs used in online execution
    std::vector<uint8_t *>
      mpc_inputs_;// MPC inputs for LowMC used in online execution
    std::vector<uint8_t *> outputs_;// Outputs from the online
                                    // execution needed for checking
};

enum signature_data_print_mask : uint8_t {
    proof_data = 1,
    proof_param = 2,
    proofs_aux = 4,
    proofs_msgs = 8,
    proofs_inputs = 16,
    proofs_mpc_inputs = 32,
    proofs_outputs = 64,
};

class Signature_data
{
  public:
    Signature_data() = delete;
    Signature_data(Mpc_param const &param) noexcept;
    size_t signature_size() const noexcept;
    size_t serialise_signature(
      uint8_t *signature, size_t signature_len) const noexcept;
    int deserialise_signature(
      const uint8_t *signature, size_t signature_len) noexcept;
    ~Signature_data();

    void print_signature_data(
      std::ostream &os, signature_data_print_mask) const;

    bool is_initialised_{ false };
    Mpc_proof_data mpc_pd_;
    Mpc_param proof_param_{};
    Proof2 **proofs_{ nullptr };
};

size_t signature_size_estimate(
  Mpc_param const &proof_param, paramset_t const &paramset);

#endif

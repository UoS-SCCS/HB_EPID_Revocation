/***************************************************************************
 * File:        Mpc_utils.h
 * Description: Utilities used for MPC
 *
 * Author:      Chris Newton
 *
 * Created:     Monday 17 January 2022
 *
 * (C) Copyright 2022, University of Surrey.
 *
 ****************************************************************************/

#ifndef MPC_UTILS_H
#define MPC_UTILS_H

#include <iostream>
#include <memory>
#include <cmath>

#include "picnic.h"
extern "C" {
#include "picnic_types.h"
#include "picnic3_impl.h"
#include "lowmc_constants.h"
}

#include "Mpc_parameters.h"
#include "Lowmc64.h"

picnic_params_t get_picnic_parameter_set_id();

// From picnic, but not declared in the headers
extern "C" {
int get_param_set(picnic_params_t picnicParams, paramset_t *paramset);
}

bool model_parameters_check_ok();

void print_picnic_parameters(std::ostream &os);

void print_hbgs_parameters(std::ostream &os);

void print_mpc_param(std::ostream &os, Mpc_param const &mpc_param);

void print_proof_indices(std::ostream &os, Mpc_proof_indices const &indices);

void print_random_tapes(
  std::ostream &os, randomTape_t *tapes, size_t tape_size);

void allocate_random_tapes(randomTape_t *tape, size_t tape_size_bytes);

void create_random_tapes(randomTape_t *tapes,
  uint8_t **seeds,
  uint8_t *salt,
  uint16_t t,
  size_t tape_size_bytes,
  paramset_t *params);

void create_random_tapes_times4(randomTape_t *tapes,
  uint8_t **seeds,
  uint8_t const *salt,
  size_t t,
  size_t tape_size_bytes,
  paramset_t *params);

// Utiltity function (not in picnic)
void get_mask_from_tapes(
  uint32_t *mask, randomTape_t *tapes, uint32_t offset, paramset_t *params);

void get_mask_from_tapes(
  Word *mask, randomTape_t *tapes, uint32_t offset, paramset_t *params);

msgs_t *allocate_msgs(size_t msgs_size);

void free_msgs(msgs_t *msgs);

using Msgs_ptr = std::unique_ptr<msgs_t, decltype(&::free_msgs)>;

void calculate_challenge_lists(uint8_t *challengeHash, uint16_t *challengeC,
  uint16_t *challengeP, paramset_t *params);

void calcualte_challenge_lists16(
  uint8_t *challenge_hash, uint16_t *challengeC, uint16_t *challengeP);

#endif

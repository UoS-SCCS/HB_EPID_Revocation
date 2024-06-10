/***************************************************************************
 * File:        Mpc_lowmc.h
 * Description: Utilities used for MPC
 *
 * Author:      Chris Newton
 *
 * Created:     Friday 28 January 2022
 *
 * (C) Copyright 2022, University of Surrey.
 *
 ****************************************************************************/

#ifndef LOWMC32_H
#define LOWMC32_H

#include <iostream>
#include <fstream>
#include <cmath>

extern "C" {
#include "picnic_types.h"
#include "picnic3_impl.h"
#include "lowmc_constants.h"
}

#include "Hbgs_param.h"
#include "Mpc_parameters.h"

using Lowmc_state_words = uint32_t[Mpc_parameters::lowmc_state_words_];
using Lowmc_state_words_ptr = uint32_t *;
using Lowmc_state_words_const_ptr = uint32_t const *;
using Lowmc_state_bytes = uint8_t[Mpc_parameters::lowmc_state_bytes_];
using Lowmc_state_bytes_ptr = uint8_t *;
using Lowmc_state_bytes_const_ptr = uint8_t const *;

void copy_lowmc_state_words32_to_lowmc_state_words32(
  Lowmc_state_words_ptr dest, Lowmc_state_words_const_ptr src);

void print_lowmc_state_bytes(
  std::ostream &os, Lowmc_state_bytes_const_ptr state_ptr) noexcept;

void print_lowmc_state_words(
  std::ostream &os, Lowmc_state_words_const_ptr state_ptr) noexcept;

bool read_lowmc_state_words(std::istream &is, Lowmc_state_words_ptr state_ptr);

bool read_lowmc_state_bytes(std::istream &is, Lowmc_state_bytes_ptr state_ptr);

void hash1a(Lowmc_state_words_ptr hash, Lowmc_state_words_const_ptr a,
  Lowmc_state_words_const_ptr b, paramset_t *params) noexcept;

void hash1b(Lowmc_state_words_ptr hash, Lowmc_state_words_const_ptr a,
  Lowmc_state_words_const_ptr b, Lowmc_state_words_const_ptr c,
  paramset_t *params) noexcept;

#endif

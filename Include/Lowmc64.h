/***************************************************************************
 * File:        Lowmc64.h
 * Description: Calculation of LOWMC using uint64
 *
 * Author:      Chris Newton
 *
 * Created:     Wednesday 31 August 2022
 *
 * (C) Copyright 2022, University of Surrey.
 *
 ****************************************************************************/

#ifndef LOWMC64_H
#define LOWMC64_H

#include <iostream>
#include <fstream>
#include <cmath>

#include "Hbgs_param.h"
#include "Mpc_parameters.h"
#include "Lowmc32.h"

using Word = uint64_t;
inline constexpr uint16_t lowmc_state_words64 =
  (Mpc_parameters::lowmc_state_bytes_ + sizeof(Word) - UINT16_C(1))
  / sizeof(Word);

constexpr uint16_t lowmc_state_words64_bytes =
  lowmc_state_words64 * sizeof(Word);

using Lowmc_state_words64 = Word[lowmc_state_words64];
using Lowmc_state_words64_ptr = Word *;
using Lowmc_state_words64_const_ptr = Word const *;

void print_lowmc_state_words64(
  std::ostream &os, Lowmc_state_words64_const_ptr state_ptr) noexcept;

bool read_lowmc_state_words64(
  std::istream &is, Lowmc_state_words64_ptr state_ptr);

constexpr size_t matrix_rows = Mpc_parameters::lowmc_state_bits_;
constexpr size_t words_per_row = Mpc_parameters::lowmc_state_words_;
constexpr size_t bytes_per_row = words_per_row * sizeof(uint32_t);
constexpr size_t words_per_row64 = lowmc_state_words64;
constexpr size_t bytes_per_row64 = words_per_row64 * sizeof(Word);

struct Matrix_numbers
{
    static constexpr size_t nk_ = Mpc_parameters::lowmc_rounds_ + 1;
    static constexpr size_t nki_ = 1;
    static constexpr size_t nl_ = Mpc_parameters::lowmc_rounds_;
    static constexpr size_t nli_ = Mpc_parameters::lowmc_rounds_;
    static constexpr size_t nr_ = Mpc_parameters::lowmc_rounds_;
};

void copy_lowmc_state_words64_to_lowmc_state_words64(
  Lowmc_state_words64_ptr dest, Lowmc_state_words64_const_ptr src) noexcept;

void copy_lowmc_state_words_to_lowmc_state_words64(
  Lowmc_state_words64_ptr w64, Lowmc_state_words_const_ptr w32) noexcept;

void copy_lowmc_state_words64_to_lowmc_state_words(
  Lowmc_state_words_ptr w32, Lowmc_state_words64_const_ptr w64) noexcept;

using Matrix64 = Lowmc_state_words64[matrix_rows];

class Lowmc_matrices
{
    Lowmc_matrices();

  public:
    static void assign_lowmc_matrices();

    static Matrix64 km_[Matrix_numbers::nk_];
    static Matrix64 kmi_[Matrix_numbers::nki_];
    static Matrix64 lm_[Matrix_numbers::nl_];
    static Matrix64 lmi_[Matrix_numbers::nli_];
    // The row constants
    static Lowmc_state_words64 rc_[Matrix_numbers::nr_];
};

void get_matrix_data(Matrix64 m64, uint32_t const *mdata);

void assign_lowmc_matrices(Lowmc_matrices &lmc, paramset_t *params);

void matrix_mul64(Lowmc_state_words64_ptr output,
  Lowmc_state_words64_const_ptr state,
  Matrix64 const &matrix) noexcept;

void xor64(Lowmc_state_words64_ptr result, Lowmc_state_words64_const_ptr a,
  Lowmc_state_words64_const_ptr b) noexcept;

void xor64(Lowmc_state_words64_ptr a, Lowmc_state_words64_const_ptr b) noexcept;

void and64(Lowmc_state_words64_ptr a, Lowmc_state_words64_const_ptr b) noexcept;

void shl64(Lowmc_state_words64_ptr a, size_t shift) noexcept;

void shr64(Lowmc_state_words64_ptr a, size_t shift) noexcept;

void lowmc64(Lowmc_state_words64_ptr ciphertext,
  Lowmc_state_words64_const_ptr key, Lowmc_state_words64_const_ptr plaintext,
  paramset_t *params) noexcept;

void hash1a64(Lowmc_state_words64_ptr hash, Lowmc_state_words64_const_ptr a,
  Lowmc_state_words64_const_ptr b, paramset_t *params) noexcept;

void hash1b64(Lowmc_state_words64_ptr hash, Lowmc_state_words64_const_ptr a,
  Lowmc_state_words64_const_ptr b, Lowmc_state_words64_const_ptr c,
  paramset_t *params) noexcept;

#endif

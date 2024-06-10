/***************************************************************************
 * File:        Lowmc32.cpp
 * Description: Lowmc state using uint32
 *
 * Author:      Chris Newton
 *
 * Created:     Thursday 15 September 2022
 *
 * (C) Copyright 2022, University of Surrey.
 *
 ****************************************************************************/

#include "Io_utils.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>

#include "Picnic_mpc_functions.h"
#include "Mpc_utils.h"
#include "Lowmc32.h"

//#define DEBUG_LOWMC_IO

void print_lowmc_state_bytes(
  std::ostream &os, Lowmc_state_bytes_const_ptr state_ptr) noexcept
{
    print_buffer(os, state_ptr, Mpc_parameters::lowmc_state_bytes_);
}

void print_lowmc_state_words(
  std::ostream &os, Lowmc_state_words_const_ptr state_ptr) noexcept
{
    print_lowmc_state_bytes(os, reinterpret_cast<uint8_t const *>(state_ptr));
}

bool read_lowmc_state_words(std::istream &is, Lowmc_state_words_ptr state_ptr)
{
    state_ptr[Mpc_parameters::lowmc_state_words_ - 1] = 0;

    return read_lowmc_state_bytes(is, reinterpret_cast<uint8_t *>(state_ptr));
}

bool read_lowmc_state_bytes(std::istream &is, Lowmc_state_bytes_ptr state_ptr)
{
    char c;// NOLINT
    std::string hstr(2, '\0');
    uint8_t b;// NOLINT
    std::memset(state_ptr, 0, Mpc_parameters::lowmc_state_bytes_);
    size_t state_index = 0;

    uint8_t i = 0;
#ifdef DEBUG_LOWMC_IO
    std::cout << "read_lowmc_state_bytes: skipping whitespace\n";
#endif
    is >> std::ws;// Skip whitespace
#ifdef DEBUG_LOWMC_IO
    std::cout << "read_lowmc_state_bytes: reading characters\n";
#endif
    while (is.get(c) && state_index < Mpc_parameters::lowmc_state_bytes_) {
        if (is.eof()) {
            std::cerr << "eof encountered\n";
            break;
        }
#ifdef DEBUG_LOWMC_IO
        std::cout << 0 + c;
#endif
        if (std::isspace(c) != 0) { break; }
        hstr[i++] =
          static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (i == 2) {
            if (std::string::npos
                != hstr.find_first_not_of("0123456789ABCDEFabcdef")) {
                std::cerr << "read_lowmc_state: bad character in input stream";
                std::memset(state_ptr, 0, Mpc_parameters::lowmc_state_bytes_);
                return false;
            }
            b = static_cast<uint8_t>(stoul(hstr, nullptr, 16));
            state_ptr[state_index++] = b;
            i = 0;
#ifdef DEBUG_LOWMC_IO
            std::cout << std::hex << 0 + b;
#endif
        }
    }
#ifdef DEBUG_LOWMC_IO
    std::cout << '\n';
#endif
    if (i != 0 || state_index != Mpc_parameters::lowmc_state_bytes_) {
        std::cerr
          << "read_lowmc_state: not enough characters in input stream\n";
        std::memset(state_ptr, 0, Mpc_parameters::lowmc_state_bytes_);
        return false;
    }

    return true;
}

void hash1a(Lowmc_state_words_ptr hash, Lowmc_state_words_const_ptr a,
  Lowmc_state_words_const_ptr b, paramset_t *params) noexcept
{
    // plaintext, ciphertext, key
    LowMCEnc(b, hash, a, params);
    xor_array(hash, hash, b, params->stateSizeWords);
}

void hash1b(Lowmc_state_words_ptr hash, Lowmc_state_words_const_ptr a,
  Lowmc_state_words_const_ptr b, Lowmc_state_words_const_ptr c,
  paramset_t *params) noexcept
{
    Lowmc_state_words intermediate_state{ 0 };
    hash1a(intermediate_state, a, b, params);
    // hash1a(hash, intermediate_state, c, params);
    LowMCEnc(c, hash, intermediate_state, params);
    xor_array(hash, hash, c, params->stateSizeWords);
}

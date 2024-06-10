/*******************************************************************************
 * File:        Mpc_sig_rl_entries.cpp
 * Description: Functions for testing the SigRL_entries
 *
 * Author:      Chris Newton
 *
 * Created:     Monday 1 January 2024, updated for new schme 28 March 2024
 *
 *
*******************************************************************************/

/*******************************************************************************
*                                                                              *
* (C) Copyright 2020-2021 University of Surrey                                 *
*                                                                              *
* Redistribution and use in source and binary forms, with or without           *
* modification, are permitted provided that the following conditions are met:  *
*                                                                              *
* 1. Redistributions of source code must retain the above copyright notice,    *
* this list of conditions and the following disclaimer.                        *
*                                                                              *
* 2. Redistributions in binary form must reproduce the above copyright notice, *
* this list of conditions and the following disclaimer in the documentation    *
* and/or other materials provided with the distribution.                       *
*                                                                              *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"  *
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE    *
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE   *
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE    *
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR          *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF         *
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS     *
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN      *
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)      *
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE   *
* POSSIBILITY OF SUCH DAMAGE.                                                  *
*                                                                              *
*******************************************************************************/


#include "Io_utils.h"

#include <iostream>
#include <cmath>
#include <cstring>

extern "C" {
#include "picnic_types.h"
#include "picnic3_impl.h"
#include "lowmc_constants.h"
}

#include "Hbgs_param.h"
#include "Lowmc64.h"
#include "Mpc_lowmc64.h"
#include "Mpc_utils.h"
#include "Mpc_sigrl_entries.h"

Epid_arl_entry calculate_epid_arl_entry(Lowmc_state_words64_const_ptr r,
  Lowmc_state_words64_const_ptr sku, Epid_sigrl_entry const &srl,
  paramset_t *params) noexcept
{
    Epid_arl_entry arl_entry;

    Lowmc_state_words64 intermediate_state{ 0 };
    lowmc64(intermediate_state, sku, srl.first(), params);

    lowmc64(arl_entry.entry(), intermediate_state, r, params);

    return arl_entry;
}

Tape_offset Mpc_sigrl_entry::set_offsets(Tape_offset const &of) noexcept
{
    Tape_offset next_offset = of;
    intermediate_mask_offset_ = next_offset;
    next_offset += Mpc_parameters::lowmc_state_bits_;

    next_offset = lowmc_a_.set_offsets(next_offset);

    next_offset = lowmc_a_j_.set_offsets(next_offset);

    assertm(next_offset - of == offset_bits_ + tape_bits_,
      "Mpc_sigrl_entry: inconsistent offsets");

    return next_offset;
}

void Mpc_sigrl_entry::compute_aux_tape(randomTape_t *current_tape_ptr,
  Lowmc_state_words64_ptr adjusted_sku_mask_a,
  // Lowmc_state_words64_const_ptr intermediate_mask,
  Lowmc_state_words64_ptr adjusted_i_mask,
  paramset_t *params) const noexcept
{

    Lowmc_state_words64 null_mask{ 0 };
    // Lowmc_state_words64 a_j_mask{ 0 };
    Lowmc_state_words64 intermediate_mask = { 0 };
    get_mask_from_tapes(
      intermediate_mask, current_tape_ptr, intermediate_mask_offset_, params);

    lowmc_a_.compute_aux_tape(current_tape_ptr, null_mask, intermediate_mask,
      adjusted_sku_mask_a, params);

    lowmc_a_j_.compute_aux_tape(
      current_tape_ptr, null_mask, null_mask, adjusted_i_mask, params);

    if (adjusted_i_mask != nullptr) {
        xor64(adjusted_i_mask, intermediate_mask);
    }
}

int Mpc_sigrl_entry::mpc_simulate(
  Lowmc_state_words64_const_ptr remasked_input_sku,
  Lowmc_state_words64_const_ptr input_sid,
  Lowmc_state_words64_const_ptr i_mask_adjustment,
  Lowmc_state_words64_const_ptr r_value, randomTape_t *current_tape_ptr,
  shares_t *tmp_shares, msgs_t *msgs, Lowmc_state_words64_ptr output_a_j,
  paramset_t *params) const noexcept
{
    Lowmc_state_words64 intermediate_state{ 0 };
    int rv = lowmc_a_.mpc_simulate(remasked_input_sku,
      input_sid,
      current_tape_ptr,
      tmp_shares,
      msgs,
      intermediate_state,
      params);
    if (rv != EXIT_SUCCESS) {
        std::cerr
          << "Mpc_sigrl_entry::mpc_simulate: initial lowmc (a) failed\n";
        return EXIT_FAILURE;
    }

    xor64(intermediate_state, i_mask_adjustment);

    rv = lowmc_a_j_.mpc_simulate(intermediate_state, r_value, current_tape_ptr,
      tmp_shares, msgs, output_a_j, params);
    if (rv != EXIT_SUCCESS) {
        std::cerr
          << "Mpc_sigrl_entry::mpc_simulate: second lowmc (a_j) failed\n";
        return EXIT_FAILURE;
    }

#ifdef DEBUG_INTERMEDIATE
    Lowmc_state_words64 intermediate_mask = { 0 };
    get_mask_from_tapes(
      intermediate_mask, current_tape_ptr, intermediate_mask_offset_, params);
    Lowmc_state_words64 unmasked_intermediate_state{ 0 };
    xor64(unmasked_intermediate_state, intermediate_mask, intermediate_state);
    std::cout << "\n  masked intermed: ";
    print_lowmc_state_words64(std::cout, intermediate_state);
    std::cout << blue << "\t  unmasked interm: ";
    print_lowmc_state_words64(std::cout, unmasked_intermediate_state);
    std::cout << normal << "\n";
#endif
    return rv;
}

void Mpc_sigrl_entry::get_aux_bits(
  uint8_t *aux_bits, uint32_t &pos, randomTape_t *tapes) const noexcept
{
    lowmc_a_.get_aux_bits(aux_bits, pos, tapes);
    lowmc_a_j_.get_aux_bits(aux_bits, pos, tapes);
}

void Mpc_sigrl_entry::set_aux_bits(
  randomTape_t *tapes, uint32_t &pos, uint8_t *aux_bits) const noexcept
{
    lowmc_a_.set_aux_bits(tapes, pos, aux_bits);
    lowmc_a_j_.set_aux_bits(tapes, pos, aux_bits);
}

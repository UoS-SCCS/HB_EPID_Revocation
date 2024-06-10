/*******************************************************************************
 * File:        Mpc_sig_rl_entries.h
 * Description: Functions for testing the SigRL_entries
 *
 * Author:      Chris Newton
 *
 * Created:     Friday 29 December 2023, updated for new schme 28 March 2024
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

#ifndef MPC_SIGRL_ENTRIES_H
#define MPC_SIGRL_ENTRIES_H

#include "Io_utils.h"

#include <iostream>
#include <cmath>

extern "C" {
#include "picnic_types.h"
#include "picnic3_impl.h"
#include "lowmc_constants.h"
}

#include "Hbgs_param.h"
#include "Lowmc64.h"
#include "Mpc_lowmc64.h"
#include "Mpc_utils.h"
#include "Hb_epid_revocation_lists.h"


Epid_arl_entry calculate_epid_arl_entry(Lowmc_state_words64_const_ptr r,
  Lowmc_state_words64_const_ptr sku, Epid_sigrl_entry const &srl,
  paramset_t *params) noexcept;

class Mpc_sigrl_entry
{
  public:
    Mpc_sigrl_entry() = default;
    Tape_offset set_offsets(Tape_offset const &of) noexcept;
    void compute_aux_tape(randomTape_t *tapes,
      Lowmc_state_words64_ptr adjusted_sku_mask_a,
      //      Lowmc_state_words64_const_ptr intermediate_mask,
      Lowmc_state_words64_ptr adjusted_s_mask_a,
      paramset_t *params) const noexcept;
    int mpc_simulate(Lowmc_state_words64_const_ptr remasked_input_sku,
      Lowmc_state_words64_const_ptr input_sid,
      Lowmc_state_words64_const_ptr masked_input_s_a,
      Lowmc_state_words64_const_ptr r_value, randomTape_t *tapes,
      shares_t *tmp_shares, msgs_t *msgs, Lowmc_state_words64_ptr output_a_j,
      paramset_t *params) const noexcept;
    void get_aux_bits(
      uint8_t *output, uint32_t &pos, randomTape_t *tapes) const noexcept;
    void set_aux_bits(
      randomTape_t *tapes, uint32_t &pos, uint8_t *input) const noexcept;

    constexpr static Tape_offset local_offset_bits_ =
      Mpc_parameters::lowmc_state_bits_;// For the intermediate mask
    constexpr static Tape_offset offset_bits_ =
      2 * Mpc_lowmc64::offset_bits_ + local_offset_bits_;
    constexpr static Tape_offset tape_bits_ = 2 * Mpc_lowmc64::tape_bits_;
    constexpr static Tape_offset aux_bits_ = 2 * Mpc_lowmc64::aux_bits_;

  private:
    Tape_offset intermediate_mask_offset_{ null_offset };

    Mpc_lowmc64 lowmc_a_{};// a=lowmc(sku, sid)
    Mpc_lowmc64 lowmc_a_j_{};// lowmc(a,r)
};

#endif

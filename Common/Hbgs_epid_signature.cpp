/***************************************************************************
 * File:        Hbgs_epid_signature.cpp
 * Description: The Hash-based EPID signature class
 *
 * Author:      Chris Newton
 * Created:     Monday 1 April 2024
 *
 * (C) Copyright 2024, University of Surrey.
 *
 ****************************************************************************/

#include <cstring>
#include "Io_utils.h"
#include "Hbgs_epid_signature.h"

Revocation_signature::Revocation_signature(std::string const &srl_file,
  uint8_t const *str, Lowmc_state_words64_const_ptr sid,
  Lowmc_state_words64_const_ptr sst,
  Lowmc_state_words64_const_ptr r_value) noexcept
  : srl_reference_(srl_file)
{
    std::memcpy(str_, str, Mpc_parameters::nonce_size_bytes_);
    std::memcpy(sid_, sid, Mpc_parameters::lowmc_state_bytes_);
    std::memcpy(cd_.sst_, sst, Mpc_parameters::lowmc_state_bytes_);
    std::memcpy(r_value_, r_value, Mpc_parameters::lowmc_state_bytes_);
}

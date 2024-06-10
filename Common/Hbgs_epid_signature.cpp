/*******************************************************************************
 * File:        Hbgs_epid_signature.cpp
 * Description: The Hash-based EPID signature class
 *
 * Author:      Chris Newton
 * Created:     Monday 1 April 2024
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

/***************************************************************************
 * File:        Mpc_working_data.h
 * Description: Working data used in the LowMC MPC proofs derived from the
 *              picnic code
 *
 * Author:      Chris Newton
 *
 * Created:     Tuesday 22 February 2022
 *
 * (C) Copyright 2022, University of Surrey.
 *
 ****************************************************************************/

#ifndef MPC_WORKING_DATA_H
#define MPC_WORKING_DATA_H

#include <iostream>
#include <cmath>
#include <vector>

#include "picnic.h"
extern "C" {
#include "picnic_types.h"
#include "picnic3_impl.h"
#include "lowmc_constants.h"
}
#include "Picnic_mpc_functions.h"
#include "Hbgs_param.h"
#include "Mpc_parameters.h"
#include "Mpc_utils.h"

enum mpc_wd_print_mask : uint8_t {
    aux_bits = 1,
    msgs = 2,
    mpc_inputs = 4,
    inputs = 8,
    outputs = 16
};

class Mpc_working_data
{
  public:
    Mpc_working_data() = delete;
    Mpc_working_data(Mpc_param const &param) noexcept;
    ~Mpc_working_data();

    void print_working_data(std::ostream &os, mpc_wd_print_mask pm);

    bool is_initialised_{ false };
    size_t aux_size_bytes_{ 0 };
    uint8_t *aux_bits_{ nullptr };// saved aux bits for each round
    msgs_t *msgs_{ nullptr };// One set of each party's messages for each round
    std::vector<inputs_t> mpc_inputs_;// One of each input per MPC round
    std::vector<inputs_t> inputs_;// One of each input per MPC round
    std::vector<inputs_t> outputs_;// One of each output per MPC round
};

class Commitment_data1
{
  public:
    Commitment_data1() noexcept;
    ~Commitment_data1();

    bool is_initialised{ false };
    commitments_t *C_{ nullptr };
};

class Commitment_data2
{
  public:
    Commitment_data2() noexcept;
    ~Commitment_data2();

    bool is_initialised{ false };
    commitments_t Ch = { nullptr, 0 };
    commitments_t Cv = { nullptr, 0 };
    tree_t *treeCv = nullptr;
};

#endif

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

#include <iostream>
#include <cmath>
#include "Io_utils.h"

#include "picnic.h"
extern "C" {
#include "picnic_types.h"
#include "picnic3_impl.h"
#include "lowmc_constants.h"
}
#include "Picnic_mpc_functions.h"
#include "Mpc_utils.h"
#include "Mpc_parameters.h"
#include "Lowmc64.h"
#include "Mpc_working_data.h"

inputs_t allocate_inputs64()
{
    auto *slab = static_cast<uint8_t *>(calloc(1,
      Mpc_parameters::mpc_rounds_
        * (lowmc_state_words64_bytes + sizeof(uint8_t *))));
    // Add this test for success allocating the memory
    if (slab == nullptr) { return nullptr; }

    auto inputs = reinterpret_cast<inputs_t>(slab);// inputs_t is uint8_t**

    slab += Mpc_parameters::mpc_rounds_ * sizeof(uint8_t *);

    for (uint32_t i = 0; i < Mpc_parameters::mpc_rounds_; i++) {
        inputs[i] = reinterpret_cast<uint8_t *>(slab);
        slab += lowmc_state_words64_bytes;
    }

    return inputs;
}


Mpc_working_data::Mpc_working_data(Mpc_param const &param) noexcept
{
    aux_size_bytes_ = param.aux_size_bytes_;
    paramset_t paramset;
    [[maybe_unused]] int ret =
      get_param_set(get_picnic_parameter_set_id(), &paramset);
    aux_bits_ = static_cast<uint8_t *>(calloc(aux_size_bytes_, 1));
    if (aux_bits_ == nullptr) { return; }

    // Currently using picnic allocation functions - no status returned
    msgs_ = allocate_msgs(aux_size_bytes_);

    inputs_.resize(param.n_inputs_);
    for (size_t i = 0; i < param.n_inputs_; ++i) {
        inputs_[i] = allocate_inputs64();
    }

    mpc_inputs_.resize(param.n_mpc_inputs_);
    for (size_t i = 0; i < param.n_mpc_inputs_; ++i) {
        mpc_inputs_[i] = allocate_inputs64();
    }

    outputs_.resize(param.n_outputs_);
    for (size_t i = 0; i < param.n_outputs_; ++i) {
        outputs_[i] = allocate_inputs64();
    }


    is_initialised_ = true;
}

Mpc_working_data::~Mpc_working_data()
{
    freeMsgs(msgs_);

    if (aux_bits_ != nullptr) { free(aux_bits_); }

    for (auto &m : inputs_) { freeInputs(m); }

    for (auto &m : mpc_inputs_) { freeInputs(m); }

    for (auto &m : outputs_) { freeInputs(m); }

    is_initialised_ = false;
}

void Mpc_working_data::print_working_data(
  std::ostream &os, mpc_wd_print_mask pm)
{
    os << "Mpc_working_data\n";
    if (!is_initialised_) {
        os << red << "not initialised\n" << normal;
        return;
    }

    if (pm & mpc_wd_print_mask::aux_bits) {
        os << "aux_bits: ";
        print_buffer(os, aux_bits_, aux_size_bytes_);
        os << '\n';
    }

    if (pm & mpc_wd_print_mask::msgs) {
        os << "unopened: " << msgs_->unopened << "\n";
        // for (size_t t = 0; t < Mpc_parameters::mpc_rounds_; ++t) {
        //     os << "round " << t << ":\n";
        for (size_t p = 0; p < Mpc_parameters::mpc_parties_; ++p) {
            os << "party " << p << ": ";
            print_buffer(os, msgs_->msgs[p], aux_size_bytes_);
            os << '\n';
        }
    }

    if (pm & mpc_wd_print_mask::mpc_inputs) {
        size_t n_mpc_inputs = mpc_inputs_.size();
        if (n_mpc_inputs == 0) {
            os << "no mpc inputs\n";
            return;
        }
        os << n_mpc_inputs << " mpc_inputs\n";
        for (size_t t = 0; t < Mpc_parameters::mpc_rounds_; ++t) {
            os << "round " << t << ":\n";
            for (size_t ni = 0; ni < n_mpc_inputs; ++ni) {
                os << ni << '\t';
                print_lowmc_state_words(
                  os, reinterpret_cast<uint32_t *>(mpc_inputs_[ni][t]));
                os << '\n';
            }
        }
    }

    if (pm & mpc_wd_print_mask::inputs) {
        size_t n_inputs = inputs_.size();
        if (n_inputs == 0) {
            os << "no inputs\n";
            return;
        }
        os << n_inputs << " inputs:\n";
        for (size_t t = 0; t < Mpc_parameters::mpc_rounds_; ++t) {
            os << "round " << t << ":\n";
            for (size_t ni = 0; ni < n_inputs; ++ni) {
                os << ni << '\t';
                print_lowmc_state_words(
                  os, reinterpret_cast<uint32_t *>(inputs_[ni][t]));
                os << '\n';
            }
        }
    }


    if (pm & mpc_wd_print_mask::outputs) {
        size_t n_outputs = outputs_.size();
        if (n_outputs == 0) {
            os << "no outputs\n";
            return;
        }
        os << n_outputs << " outputs:\n";
        for (size_t t = 0; t < Mpc_parameters::mpc_rounds_; ++t) {
            os << "round " << t << ":\n";
            for (size_t ni = 0; ni < n_outputs; ++ni) {
                os << ni << '\t';
                print_lowmc_state_words(
                  os, reinterpret_cast<uint32_t *>(outputs_[ni][t]));
                os << '\n';
            }
        }
    }
}


Commitment_data1::Commitment_data1() noexcept
{
    paramset_t paramset;
    get_param_set(get_picnic_parameter_set_id(), &paramset);
    C_ = allocateCommitments(&paramset, 0);
    is_initialised = true;
}

Commitment_data1::~Commitment_data1() { freeCommitments(C_); }

Commitment_data2::Commitment_data2() noexcept
{
    paramset_t paramset;
    get_param_set(get_picnic_parameter_set_id(), &paramset);
    allocateCommitments2(&Ch, &paramset, paramset.numMPCRounds);
    allocateCommitments2(&Cv, &paramset, paramset.numMPCRounds);
    treeCv = createTree(paramset.numMPCRounds, paramset.digestSizeBytes);
    is_initialised = true;
}

Commitment_data2::~Commitment_data2()
{
    freeCommitments2(&Ch);
    freeCommitments2(&Cv);
    freeTree(treeCv);
    is_initialised = false;
}

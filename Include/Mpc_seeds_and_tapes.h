/***************************************************************************
 * File:        Mpc_seeds_and_tapes.h
 * Description: Utilities used for MPC seeds and tapes
 *
 * Author:      Chris Newton
 *
 * Created:     Monday 21 February 2022
 *
 * (C) Copyright 2022, University of Surrey.
 *
 ****************************************************************************/

#ifndef MPC_SEEDS_TAPES_H
#define MPC_SEEDS_TAPES_H

#include <iostream>
#include <cmath>

#include "picnic.h"
extern "C" {
#include "picnic_types.h"
#include "picnic3_impl.h"
#include "tree.h"
}

class Signing_seeds_and_tapes
{
  public:
    Signing_seeds_and_tapes() = delete;
    Signing_seeds_and_tapes(
      size_t tape_size_bytes, uint8_t *salt, tree_t *iSeedsTree) noexcept;
    ~Signing_seeds_and_tapes();

    bool is_initialised{ false };
    tree_t *iSeedsTree_{ nullptr };
    randomTape_t *tapes_{ nullptr };
    tree_t **seeds_{ nullptr };

  private:
    uint8_t **iSeeds_{ nullptr };
};

class Verification_seeds_and_tapes
{
  public:
    Verification_seeds_and_tapes() = delete;
    Verification_seeds_and_tapes(
      size_t tape_size_bytes, Signature_data const &sig_data) noexcept;
    ~Verification_seeds_and_tapes();

    bool is_initialised_{ false };
    tree_t *iSeedsTree_{ nullptr };
    randomTape_t *tapes_{ nullptr };
    tree_t **seeds_{ nullptr };

  private:
    uint8_t **iSeeds_{ nullptr };
};

using Shares_ptr = std::unique_ptr<shares_t, decltype(&::freeShares)>;

using Tree_ptr = std::unique_ptr<tree_t, decltype(&::freeTree)>;

#endif

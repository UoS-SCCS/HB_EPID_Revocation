/***************************************************************************
 * File:        Mpc_seeds_and_tapes.cpp
 * Description: Utilities used for MPC seeds and tapes
 *
 * Author:      Chris Newton
 *
 * Created:     Monday 21 February 2022
 *
 * (C) Copyright 2022, University of Surrey.
 *
 ****************************************************************************/

#include <iostream>

#include "picnic.h"
extern "C" {
#include "picnic_types.h"
#include "picnic3_impl.h"
}
#include "Picnic_mpc_functions.h"
#include "Mpc_parameters.h"
#include "Mpc_utils.h"
#include "Mpc_signature_utils.h"
#include "Mpc_seeds_and_tapes.h"

Signing_seeds_and_tapes::Signing_seeds_and_tapes(
  size_t tape_size_bytes, uint8_t *salt, tree_t *iSeedsTree) noexcept
  : iSeedsTree_(iSeedsTree)
{

    iSeeds_ = getLeaves(iSeedsTree_);

    paramset_t paramset;
    get_param_set(get_picnic_parameter_set_id(), &paramset);
    tapes_ = static_cast<randomTape_t *>(
      malloc(paramset.numMPCRounds * sizeof(randomTape_t)));
    if (tapes_ == nullptr) { return; }
    seeds_ =
      static_cast<tree_t **>(malloc(paramset.numMPCRounds * sizeof(tree_t *)));
    if (seeds_ == nullptr) { return; }
    for (uint32_t t = 0; t < paramset.numMPCRounds; t++) {
        seeds_[t] =
          generateSeeds(paramset.numMPCParties, iSeeds_[t], salt, t, &paramset);
        create_random_tapes_times4(&tapes_[t],
          getLeaves(seeds_[t]),
          salt,
          t,
          tape_size_bytes,
          &paramset);
    }
    is_initialised = true;
}

Signing_seeds_and_tapes::~Signing_seeds_and_tapes()
{
    paramset_t paramset;
    get_param_set(get_picnic_parameter_set_id(), &paramset);
    for (size_t t = 0; t < paramset.numMPCRounds; t++) {
        freeRandomTape(&tapes_[t]);
        freeTree(seeds_[t]);
    }
    free(tapes_);
    free(seeds_);
    freeTree(iSeedsTree_);
}

Verification_seeds_and_tapes::Verification_seeds_and_tapes(
  size_t tape_size_bytes, Signature_data const &sig_data) noexcept
{
    paramset_t paramset;
    get_param_set(get_picnic_parameter_set_id(), &paramset);

    //=========================================================================
    // Build the iSeeds tree
    iSeedsTree_ = createTree(paramset.numMPCRounds, paramset.seedSizeBytes);
    if (iSeedsTree_ == nullptr) {
        std::cerr << "Unable to allocate memory for the iSeedsTree\n";
        return;
    }
    int ret = reconstructSeeds(iSeedsTree_, sig_data.mpc_pd_.challengeC_,
      paramset.numOpenedRounds, sig_data.mpc_pd_.iSeedInfo_,
      sig_data.mpc_pd_.iSeedInfoLen_, sig_data.mpc_pd_.salt_, 0, &paramset);
    if (ret != 0) {
        std::cerr << "Unable to recontruct the seeds\n";
        return;
    }
    //=========================================================================
    // Populate seeds and tapes with values from the signature
    seeds_ =
      static_cast<tree_t **>(calloc(paramset.numMPCRounds, sizeof(tree_t *)));
    if (seeds_ == nullptr) { return; }
    tapes_ = static_cast<randomTape_t *>(
      malloc(paramset.numMPCRounds * sizeof(randomTape_t)));
    if (tapes_ == nullptr) { return; }
    for (uint16_t t = 0; t < paramset.numMPCRounds; t++) {
        if (!contains(
              sig_data.mpc_pd_.challengeC_, paramset.numOpenedRounds, t)) {
            // Expand iSeed[t] to seeds for each parties, using a seed tree
            // These are the opened rounds.
            seeds_[t] = generateSeeds(paramset.numMPCParties,
              getLeaf(iSeedsTree_, t),
              sig_data.mpc_pd_.salt_,
              t,
              &paramset);
        } else {
            // We don't have the initial seed for the round, but instead a
            // seed for each unopened party
            seeds_[t] =
              createTree(paramset.numMPCParties, paramset.seedSizeBytes);
            int P_index = indexOf(
              sig_data.mpc_pd_.challengeC_, paramset.numOpenedRounds, t);
            uint16_t hideList[1];
            hideList[0] = sig_data.mpc_pd_.challengeP_[P_index];
            ret = reconstructSeeds(seeds_[t],
              hideList,
              1,
              sig_data.proofs_[t]->seedInfo_,
              sig_data.proofs_[t]->seedInfoLen_,
              sig_data.mpc_pd_.salt_,
              t,
              &paramset);
            if (ret != 0) {
                std::cerr << "Failed to reconstruct seeds for round " << t
                          << '\n';
                return;
            }
        }
        // Compute random tapes for all parties.  One party for each
        // repitition challengeC will have a bogus seed; but we won't use
        // that party's random tape.
        create_random_tapes(&tapes_[t],
          getLeaves(seeds_[t]),
          sig_data.mpc_pd_.salt_,
          t,
          tape_size_bytes,
          &paramset);
    }

    is_initialised_ = true;
}

Verification_seeds_and_tapes::~Verification_seeds_and_tapes()
{
    paramset_t paramset;
    get_param_set(get_picnic_parameter_set_id(), &paramset);
    for (size_t t = 0; t < paramset.numMPCRounds; t++) {
        freeRandomTape(&tapes_[t]);
        freeTree(seeds_[t]);
    }
    free(tapes_);
    free(seeds_);
    freeTree(iSeedsTree_);
}

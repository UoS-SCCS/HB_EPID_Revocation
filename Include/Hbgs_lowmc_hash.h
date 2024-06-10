/***************************************************************************
 * File:        Hbgs_hash.h
 * Description: sha3 functions used when generating the G-tree (otherwise
 *              the picnic versions are used). Generates a hash of
 *              Lowmc_state_bits at the address given.
 *
 * Author:      Chris Newton
 * Created:     Saturday 7 May 2022
 *
 * (C) Copyright 2022, University of Surrey.
 *
 ****************************************************************************/

#ifndef HBGS_LOWMC_HASH_H
#define HBGS_LOWMC_HASH_H

extern "C" {
#include "picnic3_impl.h"
#include "hash.h"
}

#include "Hbgs_param.h"
#include "Lowmc32.h"
#include "Lowmc64.h"

// Pack the data pointer and its size into a struct.
struct Hash_data
{
    size_t data_size_{ 0 };
    uint8_t *data_{ nullptr };
};

template<typename T> void hash_update(HashInstance *ctx, const T &t)
{
    static_assert(!std::is_same<Lowmc_state_words, T>::value,
      "hash_update only valid for LowMC states");
}

template<> inline void hash_update(HashInstance *ctx, const Hash_data &t)
{
    HashUpdate(ctx, t.data_, t.data_size_);
}

template<>
inline void hash_update(HashInstance *ctx, const Lowmc_state_bytes &t)
{
    HashUpdate(ctx, t, Mpc_parameters::lowmc_state_bytes_);
}

template<>
inline void hash_update(HashInstance *ctx, const Lowmc_state_words &t)
{
    // Still just do the significant bytes - ignore the extra zeros.
    HashUpdate(ctx, (uint8_t *)t, Mpc_parameters::lowmc_state_bytes_);
}

template<>
inline void hash_update(HashInstance *ctx, const Lowmc_state_words64 &t)
{
    // Still just do the significant bytes - ignore the extra zeros.
    HashUpdate(ctx, (uint8_t *)t, Mpc_parameters::lowmc_state_bytes_);
}

// Initialiser list version for the hash
// (A,B) - A is carried out first, then B. The result from B is returned
// (os << t, 0) - writes t to the stream and returns 0 to the <int> initializer
// list
// ... the parameter pack is expanded
template<typename... T>
void lowmc_state_from_hash(uint8_t *hash, paramset_t *params, const T &... t)
{
    HashInstance ctx;

    HashInit(&ctx, params, HASH_PREFIX_NONE);
    (void)std::initializer_list<int>{ (hash_update(&ctx, t), 0)... };
    HashFinal(&ctx);
    HashSqueeze(&ctx, hash, Mpc_parameters::lowmc_state_bytes_);

    zeroTrailingBits(hash, Mpc_parameters::lowmc_state_bits_);
}

#endif

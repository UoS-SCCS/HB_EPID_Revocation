/***************************************************************************
 * File:        Picnic_mpc_functions.c
 * Description: Picnic functions for MPC, adapted from the picnic code
 *
 * Author:      Chris Newton
 *
 * Created:     Monday 17 January 2022
 *
 * (C) Copyright 2022, University of Surrey.
 *
 ****************************************************************************/

#include <string.h>

#include "picnic.h"
#include "picnic_impl.h"
#include "picnic_types.h"

#include "Picnic_mpc_functions.h"


// Minor change to picnic commit function to allow for changing size of aux bits
void commit_c(uint8_t *digest,
  uint8_t *seed,
  uint8_t *aux,
  uint8_t *salt,
  uint16_t t,
  uint16_t j,
  paramset_t *params)
{
    /* Compute C[t][j];  as digest = H(seed||[aux]) aux is optional */
    HashInstance ctx;

    HashInit(&ctx, params, HASH_PREFIX_NONE);
    HashUpdate(&ctx, seed, params->seedSizeBytes);
    if (aux != NULL) { HashUpdate(&ctx, aux, sizeof(aux)); }
    HashUpdate(&ctx, salt, params->saltSizeBytes);
    HashUpdateIntLE(&ctx, t);
    HashUpdateIntLE(&ctx, j);
    HashFinal(&ctx);
    HashSqueeze(&ctx, digest, params->digestSizeBytes);
}

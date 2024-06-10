/***************************************************************************
 * File:        Hb_epid_revocation_lists.cpp
 * Description: Code for the Hash-based EPID revocation list classes
 *
 * Author:      Chris Newton
 * Created:     Monday 6 Novemeber 2023
 *
 * (C) Copyright 2023, University of Surrey.
 *
 ****************************************************************************/


#include <string>
#include <vector>
#include <cstring>// for memcpy
#include "Io_utils.h"
#include "Hbgs_param.h"
#include "Lowmc64.h"
#include "Hb_epid_revocation_lists.h"

template<> const std::string Epid_rl_entry<Sigrl>::list_name{ "sigRL" };

template<> const std::string Epid_list_entry<Rla>::list_name{ "RL_a" };
template<> const std::string Epid_list_entry<Rlk>::list_name{ "keyRL" };

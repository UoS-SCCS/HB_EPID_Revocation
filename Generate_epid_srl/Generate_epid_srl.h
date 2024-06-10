/***************************************************************************
 * File:        Generate_epid_srl.h
 * Description: Code to generate test revocation lists for hash based EPID
 *
 * Author:      Chris Newton
 * Created:     Tuesday 14 November 2023
 *
 * (C) Copyright 2023, University of Surrey.
 *
 ****************************************************************************/

#ifndef GENERATE_EPID_RL_H
#define GENERATE_EPID_RL_H

#include <iostream>
#include <string>
#include "Clock_utils.h"

void usage(std::ostream &os, std::string program);

struct Mpc_time_point
{
    std::string type_;
    float time_;
};

using Mpc_timings = std::vector<Mpc_time_point>;

struct Mpc_timing_data
{
    F_timer_ms timer_;
    Mpc_timings times_;
};

#endif

/***************************************************************************
 * File:        Mem_uint.h
 * Description: Code for setting memory to uints and getting uints from
 *              memory
 *
 * Author:      Chris Newton
 *
 * Created:     Thursday 15 September 2022
 *
 * (C) Copyright 2022, University of Surrey.
 *
 ****************************************************************************/

#ifndef MEM_UINT_H
#define MEM_UINT_H

#include <limits>
#include <cstring>

#include "Hbgs_param.h"


template<typename U, typename = typename std::enable_if<
                       std::numeric_limits<U>::is_integer
                       && !std::numeric_limits<U>::is_signed>::type>
bool uint_to_mem(uint8_t *mem_addr, size_t len, U ui, bool store_big_endian)
{
    size_t ui_size = sizeof(U);
    if (ui_size > len) { return false; }
    std::memset(mem_addr, 0, len);
    size_t index{};
    for (size_t i = 0; i < ui_size; ++i) {
        index = store_big_endian ? ui_size - 1 - i : i;
        *(mem_addr + index) = (ui & 0xff);// NOLINT
        ui >>= 8;// NOLINT
    }

    return true;
}

template<typename U, typename = typename std::enable_if<
                       std::numeric_limits<U>::is_integer
                       && !std::numeric_limits<U>::is_signed>::type>
bool mem_to_uint(
  U &ival, uint8_t *mem_addr, size_t len, bool stored_as_big_endian)
{

    if (len > sizeof(U)) { return false; }
    size_t index{};
    index = stored_as_big_endian ? 0 : len - 1;
    ival = *(mem_addr + index);
    for (size_t i = 1; i < len; ++i) {
        index = stored_as_big_endian ? i : len - 1 - i;
        ival = (ival << 8) + *(mem_addr + index);// NOLINT
    }

    return true;
}


#endif

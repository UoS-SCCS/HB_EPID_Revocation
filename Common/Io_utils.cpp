/**********************************************************************
 * File:        Io_utils.cpp
 * Description: I/O utilities
 *
 * Author:      Chris Newton
 * Created:     Wednesday 1 May 2013
 *
 * (C) Copyright 2013, Hewlett-Packard Ltd, all rights reserved.
 *
 **********************************************************************/

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <sstream>
#include <string>
#include <cctype>
#include <cstdlib>
#include <algorithm>
#include "Io_utils.h"

std::string make_filename(std::string const &baseDir, std::string const &name)
{
    std::string fName = baseDir + dirSep + name;
    // Now fix up the directory seperators as appropriate
    std::string::size_type pos = 0;
    while ((pos = fName.find(altDirSep, pos)) != std::string::npos) {
        fName.replace(pos, 1, 1, dirSep);
    }
    return fName;
}

std::string get_environment_variable(
  std::string const &var, std::string def) noexcept
{
    const char *ret = std::getenv(var.c_str());
    return ret != nullptr ? std::string(ret) : std::move(def);
}

void eat_white(std::istream &is)
{
    // Replace this with std::ws
    //    char c;// NOLINT
    //    while (std::isspace(is.peek()) != 0) {
    //        is.get(c);
    //    }

    is >> std::ws;
}

std::string str_tolower(std::string const &str)
{
    std::string s{ str };
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return s;
}

std::string str_toupper(std::string const &str)
{
    std::string s{ str };
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return std::toupper(c);
    });
    return s;
}

void print_buffer(std::ostream &os, const uint8_t *buf, const size_t len)
{
    if (len == 0) {
        os << "buffer empty";
        return;
    }
    std::ios oldState(nullptr);
    oldState.copyfmt(os);
    os << std::setfill('0') << std::hex;
    for (size_t i = 0; i < len; ++i) {
        os << std::setw(2) << 0 + buf[i];// 0 + used to force conversion to an
                                         // integer for printing
    }

    os.copyfmt(oldState);
}

void print_buffer_as_chars(std::ostream &os,
  const uint8_t *buf,
  size_t len,
  uint8_t non_char_replacement)
{
    uint8_t ch{};
    for (size_t i = 0; i < len; ++i) {
        ch = buf[i];
        if (std::isspace(ch) != 0 || std::isprint(ch) != 0) {
            os << ch;
        } else {
            os << non_char_replacement;
        }
    }
}

void print_hex_byte(std::ostream &os, uint8_t byte)
{
    std::ios oldState(nullptr);
    oldState.copyfmt(os);
    os << std::setfill('0') << std::hex;
    os << std::setw(2) << 0 + byte;
    os.copyfmt(oldState);
}

bool read_hex_byte(std::istream &is, uint8_t *byte)
{
    std::string hstr(2, '\0');
    is.get(hstr[0]);
    is.get(hstr[1]);
    if (std::string::npos != hstr.find_first_not_of("0123456789ABCDEFabcdef")) {
        std::cerr << "read_byte: bad character in input stream\n";
        return false;
    }

    (*byte) = static_cast<uint8_t>(stoul(hstr, nullptr, 16));

    return true;
}

bool read_hex_bytes(std::istream &is, uint8_t *buf, size_t number_to_read)
{
    std::string hstr(2, '\0');

    for (size_t i = 0; i < number_to_read; ++i) {
        is.get(hstr[0]);
        is.get(hstr[1]);
        if (std::string::npos
            != hstr.find_first_not_of("0123456789ABCDEFabcdef")) {
            std::cerr << "read_byte: bad character in input stream\n";
            return false;
        }
        buf[i] = static_cast<uint8_t>(stoul(hstr, nullptr, 16));
    }

    return true;
}


/*
std::istream &operator>>(std::istream &is, Byte_buffer &bb)
{
    char c;// NOLINT
    std::string hstr(2, '\0');
    Byte b;// NOLINT
    bb.byte_buf_.clear();

    uint8_t i = 0;
    is >> std::ws;// Skip whitespace
    while (is.get(c)) {
        if (std::isspace(c) != 0) {
            break;
        }
        hstr[i++] = static_cast<char>(std::tolower(static_cast<unsigned
char>(c))); if (i == 2) { if (std::string::npos !=
hstr.find_first_not_of("0123456789ABCDEFabcdef")) {
                throw(std::runtime_error("Byte_buffer::operator>>: bad character
in input stream"));
            }
            b = static_cast<Byte>(stoul(hstr, nullptr, 16));
            bb.byte_buf_.push_back(b);
            i = 0;
        }
    }
    if (i != 0) {
        throw(std::runtime_error("Byte_buffer::operator>>: odd number of
characters in input stream"));
    }

    return is;
}*/
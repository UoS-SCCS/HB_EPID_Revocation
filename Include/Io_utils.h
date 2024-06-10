/**********************************************************************
 * File:        Io_utils.h
 * Description: I/O utilities
 *
 * Author:      Chris Newton
 * Created:     Wednesday 1 May 2013
 *
 * (C) Copyright 2013
 *
 **********************************************************************/

#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <initializer_list>
#include <iostream>
#include <sstream>
#include <string>

// Define the appropriate directory seperators and their opposites
#ifndef _WIN32
//#define _MAX_FNAME 1024
constexpr char dirSep = '/';
constexpr char altDirSep = '\\';
#else
constexpr char dirSep = '\\';
constexpr char altDirSep = '/';
#endif

// Terminal colours
auto constexpr red = "\33[31m";
auto constexpr green = "\33[32m";
auto constexpr yellow = "\33[33m";
auto constexpr blue = "\33[34m";
auto constexpr magenta = "\33[35m";
auto constexpr cyan = "\33[36m";
auto constexpr white = "\33[37m";
auto constexpr bright_red = "\33[91m";
auto constexpr bright_green = "\33[92m";
auto constexpr bright_yellow = "\33[93m";
auto constexpr bright_blue = "\33[94m";
auto constexpr bright_magenta = "\33[95m";
auto constexpr bright_cyan = "\33[96m";
auto constexpr normal = "\33[0m";

constexpr int maxline = 200;

std::string make_filename(std::string const &baseDir, std::string const &name);

std::string get_environment_variable(
  std::string const &var,
  std::string def) noexcept;

void eat_white(std::istream &is);

std::string str_tolower(std::string const &str);

std::string str_toupper(std::string const &str);

void print_hex_byte(std::ostream &os, uint8_t byte);

void print_buffer(std::ostream &os, const uint8_t *buf, size_t len);

void print_buffer_as_chars(std::ostream &os,
  const uint8_t *buf,
  size_t len,
  uint8_t non_char_replacement);

bool read_hex_byte(std::istream &is, uint8_t *byte);

bool read_hex_bytes(std::istream &is, uint8_t *buf, size_t number_to_read);


// Recursive version of vars_to_string, may cause code to grow unnecessarily
// see https://www.youtube.com/watch?v=CU3VYN6xGzM, C++ Weekly for an
// alternative using initialiser lists
/* template<typename T>
std::string vars_to_string(T const &t)
{
    std::ostringstream os;
    os << t;
    return os.str();
}

template<typename T, typename... Vargs>
std::string vars_to_string(T t, Vargs... args)
{
    std::ostringstream os;
    os << t;
    return os.str() + vars_to_string(args...);
}
*/
// Alternative initialiser list version
// (A,B) - A is carried out first, then B. The result from B is returned
// (os << t, 0) - writes t to the stream and returns 0 to the <int> initializer
// list
// ... the parameter pack is expanded
template<typename... T>
std::string vars_to_string(const T &...t)
{
    std::ostringstream os;
    (void)std::initializer_list<int>{ (os << t, 0)... };
    return os.str();
}

#endif
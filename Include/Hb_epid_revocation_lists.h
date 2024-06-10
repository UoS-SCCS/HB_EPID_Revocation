/***************************************************************************
 * File:        Hb_epid_revocation_lists.h
 * Description: The Hash-based EPID revocation list classes
 *
 * Author:      Chris Newton
 * Created:     Sunday 5 Novemeber 2023
 *
 * (C) Copyright 2023, University of Surrey.
 *
 ****************************************************************************/

#ifndef HB_EPID_REVOCATION_LIST_H
#define HB_EPID_REVOCATION_LIST_H

#include <string>
#include <vector>

#include "Hbgs_param.h"
#include "Lowmc64.h"

const std::string sigrl_file_ext{ "srlist" };
const std::string arl_file_ext{ "alist" };
const std::string keyrl_file_ext{ "krlist" };

class Sigrl// Used with Epid_rl_entry for entries on the SigRL list
// first is sid_j and second is sst_j
{};

template<typename T> class Epid_rl_entry
{
  public:
    Epid_rl_entry() noexcept {}
    Epid_rl_entry(Epid_rl_entry<T> const &sre) noexcept;
    Epid_rl_entry(Lowmc_state_words64_const_ptr first,
      Lowmc_state_words64_const_ptr second) noexcept;
    Epid_rl_entry &operator=(Epid_rl_entry<T> const &sre) noexcept;
    void print_entry(std::ostream &os) const noexcept;
    bool read_entry(std::istream &is) noexcept;
    Lowmc_state_words64_const_ptr first() const noexcept { return first_; }
    Lowmc_state_words64_ptr first() noexcept { return first_; }
    Lowmc_state_words64_const_ptr second() const noexcept { return second_; }
    Lowmc_state_words64_ptr second() noexcept { return second_; }

    static const std::string list_name;// The name for a list of these entries

  private:
    Lowmc_state_words64 first_{ 0 };
    Lowmc_state_words64 second_{ 0 };
};

template<typename T>
Epid_rl_entry<T>::Epid_rl_entry(Epid_rl_entry<T> const &srle) noexcept
{
    std::memcpy(first_, srle.first(), lowmc_state_words64_bytes);
    std::memcpy(second_, srle.second(), lowmc_state_words64_bytes);
}

template<typename T>
Epid_rl_entry<T>::Epid_rl_entry(Lowmc_state_words64_const_ptr first,
  Lowmc_state_words64_const_ptr second) noexcept
{
    std::memcpy(first_, first, lowmc_state_words64_bytes);
    std::memcpy(second_, second, lowmc_state_words64_bytes);
}

template<typename T>
Epid_rl_entry<T> &Epid_rl_entry<T>::operator=(
  Epid_rl_entry const &srle) noexcept
{
    std::memcpy(first_, srle.first(), lowmc_state_words64_bytes);
    std::memcpy(second_, srle.second(), lowmc_state_words64_bytes);
    return *this;
}

template<typename T>
void Epid_rl_entry<T>::print_entry(std::ostream &os) const noexcept
{
    print_lowmc_state_words64(os, first_);
    os << '\t';
    print_lowmc_state_words64(os, second_);
    os << '\n';
}

template<typename T>
bool Epid_rl_entry<T>::read_entry(std::istream &is) noexcept
{
    if (!read_lowmc_state_words64(is, first_)) { return false; }

    if (!read_lowmc_state_words64(is, second_)) { return false; }

    return true;
}

class Rla// Used with Epid_list_entry for A_j entries on a list
{
};

class Rlk// Used with Epid_list_entry for key RL entries on a list
{
};

template<typename T> class Epid_list_entry
{
  public:
    Epid_list_entry() noexcept {}
    Epid_list_entry(Epid_list_entry const &srl) noexcept;
    Epid_list_entry(Lowmc_state_words64_const_ptr sk_u) noexcept;
    Epid_list_entry &operator=(Epid_list_entry const &krle) noexcept;
    void print_entry(std::ostream &os) const noexcept;
    bool read_entry(std::istream &is) noexcept;
    Lowmc_state_words64_const_ptr entry() const noexcept { return entry_; }
    Lowmc_state_words64_ptr entry() noexcept { return entry_; }

    static const std::string list_name;// The name for a list of these entries

  private:
    Lowmc_state_words64 entry_{ 0 };
};

template<typename T>
Epid_list_entry<T>::Epid_list_entry(Epid_list_entry<T> const &le) noexcept
{
    std::memcpy(entry_, le.entry(), lowmc_state_words64_bytes);
}

template<typename T>
Epid_list_entry<T>::Epid_list_entry(
  Lowmc_state_words64_const_ptr entry) noexcept
{
    std::memcpy(entry_, entry, lowmc_state_words64_bytes);
}

template<typename T>
Epid_list_entry<T> &Epid_list_entry<T>::operator=(
  Epid_list_entry<T> const &le) noexcept
{
    std::memcpy(entry_, le.entry(), lowmc_state_words64_bytes);
    return *this;
}

template<typename T>
void Epid_list_entry<T>::print_entry(std::ostream &os) const noexcept
{
    print_lowmc_state_words64(os, entry_);
    os << '\n';
}

template<typename T>
bool Epid_list_entry<T>::read_entry(std::istream &is) noexcept
{
    if (!read_lowmc_state_words64(is, entry_)) { return false; }
    return true;
}

template<typename T> class Hb_epid_rl
{
  public:
    using R_list = std::vector<T>;
    Hb_epid_rl() = default;
    Hb_epid_rl(Hb_epid_rl const &) = default;
    Hb_epid_rl &operator=(Hb_epid_rl const &) = default;
    T &operator[](size_t index) { return rl_[index]; }
    T const &operator[](size_t index) const { return rl_[index]; }
    void add(T const &entry) noexcept;
    void print_rl(std::ostream &os) const;
    bool save_rl(std::string const &filename) const noexcept;
    bool read_rl(std::istream &is) noexcept;
    bool empty() const { return rl_.empty(); }
    size_t size() const { return rl_.size(); }
    void clear() noexcept { rl_.clear(); }
    void resize(size_t sz) { rl_.resize(sz); }
    void shrink_to_fit() { rl_.shrink_to_fit(); }
    R_list const &revocation_list() const { return rl_; }
    std::string const &list_name() const { return T::list_name; };

  private:
    R_list rl_;
};

template<typename T> void Hb_epid_rl<T>::add(T const &entry) noexcept
{
    rl_.push_back(entry);
}

template<typename T> bool Hb_epid_rl<T>::read_rl(std::istream &is) noexcept
{
    std::string rl_name;
    size_t n_entries;
    std::string junk;
    is >> rl_name >> n_entries >> junk;// junk is the "entries" string
    if (rl_name != T::list_name) {
        std::cerr << "Inconsistent list name, should be: " << T::list_name
                  << '\n';
    }
    rl_.resize(n_entries);

    for (size_t entry_no = 0; entry_no < n_entries; ++entry_no) {
        rl_[entry_no].read_entry(is);
    }

    return true;
}

template<typename T> void Hb_epid_rl<T>::print_rl(std::ostream &os) const
{
    os << list_name() << ' ' << rl_.size() << " entries\n";
    for (auto const &rle : rl_) { rle.print_entry(os); }
}

using Epid_sigrl_entry = Epid_rl_entry<Sigrl>;
using Epid_sigrl_entry_ptr = Epid_rl_entry<Sigrl> *;
using Epid_sigrl_entry_const_ptr = Epid_rl_entry<Sigrl> const *;
using Epid_sig_rl = Hb_epid_rl<Epid_sigrl_entry>;
using Epid_sig_rl_ptr = Hb_epid_rl<Epid_sigrl_entry> *;
using Epid_sig_rl_const_ptr = Hb_epid_rl<Epid_sigrl_entry> const *;

using Epid_arl_entry = Epid_list_entry<Rla>;
using Epid_arl_entry_ptr = Epid_list_entry<Rla> *;
using Epid_arl_entry_const_ptr = Epid_list_entry<Rla> const *;

using Epid_a_rl = Hb_epid_rl<Epid_arl_entry>;
using Epid_a_rl_ptr = Hb_epid_rl<Epid_arl_entry> *;
using Epid_a_rl_const_ptr = Hb_epid_rl<Epid_arl_entry> const *;

using Epid_keyRL_entry = Epid_list_entry<Rlk>;
using Epid_keyRL_entry_ptr = Epid_list_entry<Rlk> *;
using Epid_keyRL_entry_const_ptr = Epid_list_entry<Rlk> const *;

using Epid_key_rl = Hb_epid_rl<Epid_keyRL_entry>;
using Epid_key_rl_ptr = Hb_epid_rl<Epid_keyRL_entry> *;
using Epid_key_rl_const_ptr = Hb_epid_rl<Epid_keyRL_entry> const *;


#endif

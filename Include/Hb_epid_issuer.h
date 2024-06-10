/***************************************************************************
 * File:        Hb_epid_issuer.h
 * Description: The Hash-based EPID issuer class
 *
 * Author:      Chris Newton
 * Created:     Friday 3 Novemeber 2023
 *
 * (C) Copyright 2023, University of Surrey.
 *
 ****************************************************************************/

#ifndef HB_EPID_ISSUER_H
#define HB_EPID_ISSUER_H

#include <string>
#include <vector>

#include "Hbgs_param.h"
#include "Lowmc64.h"

const std::string issuer_file_ext{ "issuer" };

class Group_list_entry
{
  public:
    Group_list_entry() noexcept {}
    Group_list_entry(unsigned int user_no, Lowmc_state_words64_const_ptr et_u,
      Lowmc_state_words_const_ptr gr_u,
      std::string const &credential_filename) noexcept;
    Group_list_entry(Group_list_entry const &gle) noexcept;
    Group_list_entry &operator=(Group_list_entry const &gle) noexcept;

    unsigned int user_no_{ 0 };
    Lowmc_state_words64 entry_token_{ 0 };
    Lowmc_state_words64 gr_u_{ 0 };
    std::string credential_filename_;

  private:
    void copy(Group_list_entry &dest, Group_list_entry const &src);
};

class Hb_epid_issuer
{
  public:
    enum Status { uninitialised, initialised, key_set };
    Hb_epid_issuer() noexcept;
    // Initialise from scratch
    Hb_epid_issuer(std::string const &name) noexcept;
    // Initialise from a file
    Hb_epid_issuer(
      std::string const &base_dir, std::string const &basename) noexcept;
    Hb_epid_issuer(Hb_epid_issuer const &) = delete;
    Hb_epid_issuer &operator=(Hb_epid_issuer const &) = delete;
    void print_data(std::ostream &os) const noexcept;
    bool save_data(std::string const &filename) const noexcept;
    bool load_data(std::string const &base_dir, std::string const &basename);
    Status status() const { return status_; }
    Lowmc_state_words64_const_ptr public_key() const noexcept
    {
        return public_key_;
    }
    // For testing only - this is a secret!
    Lowmc_state_words64_const_ptr seed() const noexcept { return master_seed_; }
    Lowmc_state_words64_const_ptr group_id() const noexcept
    {
        return master_seed_;
    }

    Status status_{ Status::uninitialised };

  private:
    paramset_t paramset_;
    std::string name_;
    Lowmc_state_words64 master_seed_{ 0 };
    Lowmc_state_words64 public_key_{ 0 };
    Lowmc_state_words64 gid_{ 0 };
    std::vector<Group_list_entry> group_list_;

    unsigned int next_user{ 0 };

    const static char seed_string_[];
    bool obtain_master_seed_from_seed_string();
    bool calculate_public_key() noexcept;
    void print_group_list(std::ostream &os) const noexcept;
};

#endif

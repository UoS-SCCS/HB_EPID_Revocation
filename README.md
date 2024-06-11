# HB_EPID_Revocation
Source code for the signature revocation component of the Hash-based EPID scheme described in:

    A New Hash-based Enhanced Privacy ID Signature Scheme 

    Liqun Chen, Changyu Dong, Nada El Kassem, Christopher J.P. Newton and Yalan Wang

To be published in the Proceedings of PQCrypto 2024.

## Description
This project is a test implementation of the hash-based EPID signature revocation scheme. The code uses some of the reference picnic3 code from:

    https://github.com/Microsoft/Picnic.

There are separate executables for each value of the number of bits in the LowMC state (n). The executables are
built using cmake, which is set to allow executables for n=129 or n=255 to be built.

The executable are:
    - generate_epid_srl_nnn. this generates a signature revocation list that can then be used for testing. nnn is the number of bits in the LowMC state being used.

    - hbgs_sigrl_list_test_nnn, this generates the proof that the signer's key is not inlcuded in the SRL ($\pi_R$) and then runs the verification check. The output is a set of timings for the different stages together with the signature size.

## Installation and Testing
For installation and testing see README.TXT.

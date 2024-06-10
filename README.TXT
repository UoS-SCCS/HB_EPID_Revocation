## Installation
Install the code files in a directory. Change to this directory.

Change to the HBGS_picnic directory and make the picnic libraries

   cd HBGS_picnic
   make

Return to the code directory and create a build directory:

  mkdir build

Change to this directory and enter

  cmake ..

Then you can use 

  ccmake ..

to select which executable to build and then use make to build the executables.

For a given choice of parameters, the executables that will be built are:

- generate_epid_srl_nnn 

- hbgs_sigrl_list_test_nnn

Where <nnn> is 129, or 255. The executables will be found in <code directory>/bin

## Usage

Start by creating a signature revocation list (SRL), in the code directory enter:

  bin/generate_epid_srl_nnn <base dir>  <revocation file name> <number of srl entries>

Where <base_dir> is the directory where the list is to be stored, <revocation file name> is a name for the file and <number of srl entries> the number of entries to be generated. There are a number of pre-generated SRLs in the RL_data directory. The format of these files is:

rl_<nnn>_<no of entries>.srlist.

To run a signature test enter:

   bin/hbgs_sigrl_list_test_nnn <base dir> <list name> <pass T/F>

The first two parameters give the SRL to use, while the last parameter is used to ensure that the test passes (T), or fails (F). In the failure case (F) an entry in the given SRL is adjusted so that it appears to have been derived from the signer's key and so the test will fail. Note that the filename is given withou the .srlist extension.

When the test passses the output gives:

<filename> <number of entries in the SRL> <time to sign (ms)> <time to verify (ms)> <signature size (Mb)>

For example, one run of :

   bin/hbgs_sigrl_list_test_129 RL_data rl_129_100 T

gave

   RL_data rl_129_100 100 6979.51 3417.02 1.03828.
   
THere are two scripts (runjobs_129 and runjobs_255) that can be used to run a set of tests. The resulting .txt files can be read into a spreadsheet for processing.

## License
The main body of code is licensed under the BSD two clause licence (see LICENSE.TXT).

The Picnic code that is used in this implementation has its own licence (see HBGS_picnic/LICENCE).

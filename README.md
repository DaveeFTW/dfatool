# DFA Tool - Calculate AES key from faulted output

DFA tool supports analysis of AES128 encryption and decryption. It can analyse a single fault in round 9 or round 8, requiring at least 8 r9 faults, or 2 r8 faults.

Reference: [Differential Fault Analysis on A.E.S.][dfa_aes]

## Usage
Execute dfatool with the following arguments:
 * `--encrypt` for AES 128 Encryption
 * `--decrypt` for AES 128 Decryption
 * `--r8` for round 8 fault inputs
 * `--r9` for round 9 fault inputs
 * `file` simple text file containing the hexadecimal representation of the input faults.

Example: `dfatool --encrypt --r8 file.txt` for analysing round 8 faults in AES 128 encryption.

## Building
Requires C++17, and uses cmake. Simply:
```
mkdir build && cd build
cmake ..
make/ninja
```

## License
MIT 👍

  [dfa_aes]: https://ia.cr/2003/010
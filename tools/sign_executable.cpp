// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// syntax: sign_executable data_file private_key_file

#include "config.h"
#include "crypt.h"

std::pair<bool, std::string> sign_executable(
    const std::string& path, const std::string& code_sign_keyfile
    ) {
    int retval = 0;
    R_RSA_PRIVATE_KEY code_sign_key;
    std::tie(retval, code_sign_key) = read_key_file(code_sign_keyfile);
    if (retval) {
        fprintf(stderr, "add: can't read key\n");
        return std::make_pair(false, std::string());
    }
    std::string signature_text =
        sprint_hex_data(sign_file(path, code_sign_key));
    return std::make_pair(true, signature_text);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "syntax: sign_executable data_file private_key_file\n"
            "\n"
            "Writes signature to stdout.\n"
        );
        return 1;
    }

    bool result = false;
    std::string signature_text;
    std::tie(result, signature_text) = sign_executable(argv[1], argv[2]);
    if(!result) {
        return 1;
    }
    printf("%s", signature_text.c_str());

    return 0;
}

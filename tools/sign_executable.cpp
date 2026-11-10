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

using std::make_pair;
using std::pair;
using std::string;
using std::tie;
using std::vector;

pair<int, string> sign_executable(
    const string& path,
    const string& code_sign_keyfile
    ) {
    int retval = 0;
    R_RSA_PRIVATE_KEY code_sign_key;
    tie(retval, code_sign_key) = read_key_file(code_sign_keyfile);
    if (retval) {
        fprintf(stderr, "add: can't read key\n");
        return make_pair(1, string());
    }
    int result = 0;
    vector<uint8_t> data;
    tie(result, data) = sign_file(path, code_sign_key);
    if (result || data.empty()) {
        fprintf(stderr, "add: can't sign file\n");
        return make_pair(1, string());
    }
    return sprint_hex_data(data);
}

int main(
    int argc,
    char** argv
    ) {
    if (argc != 3) {
        fprintf(stderr, "syntax: sign_executable data_file private_key_file\n"
            "\n"
            "Writes signature to stdout.\n"
        );
        return 1;
    }

    int result = 0;
    string signature_text;
    tie(result, signature_text) = sign_executable(argv[1], argv[2]);
    if(result || signature_text.empty()) {
        return 1;
    }
    printf("%s", signature_text.c_str());

    return 0;
}

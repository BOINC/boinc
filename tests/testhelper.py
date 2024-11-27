# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2024 University of California
#
# BOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# BOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

import re

class TestHelper:
    def get_current_version_number(self):
        with open("version.h", "r") as f:
            lines = f.readlines()
            for line in lines:
                res = re.search("#define BOINC_VERSION_STRING \"([\d]+.[\d]+.[\d]+)\"", line)
                if res is not None:
                    return res[1]
        return ""

    def get_version_from_string(self, string):
        res = re.search("([\d]+.[\d]+.[\d]+)", string)
        if res is not None:
            return res[1]
        return ""

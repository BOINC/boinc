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

#include "boinccas_helper.h"
#include "user_group_helper.h"

namespace test_boinccas_CAGrantBOINCAdminsVirtualBoxRights {
    class test_boinccas_CAGrantBOINCAdminsVirtualBoxRights :
        public test_boinccas_TestBase {
    protected:
        test_boinccas_CAGrantBOINCAdminsVirtualBoxRights() :
            test_boinccas_TestBase("GrantBOINCAdminsVirtualBoxRights") {
        }

        void TearDown() override {
            if (localGroupExists("boinc_admins")) {
                deleteLocalGroup("boinc_admins");
            }
        }
    };

#ifndef BOINCCAS_TEST
#endif
}

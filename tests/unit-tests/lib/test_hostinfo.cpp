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

#ifndef _WIN32
#include "gtest/gtest.h"
#endif
#include "hostinfo.h"
#include <initializer_list>
#include <string>
#include <utility>

namespace test_hostinfo {
    class test_hostinfo : public ::testing::Test {
    protected:
        using Ranges = std::initializer_list<std::pair<long long, long long>>;

        std::string mapping(const char* kind, Ranges ranges) {
            std::string result = std::string("    ") + kind + ":\n";
            long long host_id = 100000;
            for (const auto& range: ranges) {
                result += "    - container_id: " + std::to_string(range.first)
                    + "\n      host_id: " + std::to_string(host_id)
                    + "\n      size: " + std::to_string(range.second) + "\n";
                host_id += 100000;
            }
            return result;
        }

        bool accepts(Ranges uids, Ranges gids) {
            std::string info = "host:\n  idMappings:\n" + mapping("gidmap", gids)
                + mapping("uidmap", uids) + "  security:\n    rootless: true\n";
            bool accepted = false;
            EXPECT_TRUE(HOST_INFO::parse_podman_info_subids(info.c_str(), accepted));
            return accepted;
        }
    };

    TEST_F(test_hostinfo, AcceptsSingleAndSplitHostRanges) {
        EXPECT_TRUE(accepts({{0, 1}, {1, 65536}}, {{0, 1}, {1, 65536}}));
        EXPECT_TRUE(accepts({{0, 1}, {1, 32768}, {32769, 32768}},
            {{0, 1}, {1, 32768}, {32769, 32768}}));
    }

    TEST_F(test_hostinfo, AcceptsRangesInEitherOrder) {
        EXPECT_TRUE(accepts({{32769, 32768}, {0, 1}, {1, 32768}},
            {{1, 65536}, {0, 1}}));
    }

    TEST_F(test_hostinfo, RejectsGapsDespiteSufficientTotalSize) {
        EXPECT_FALSE(accepts({{1, 32768}, {32770, 32768}}, {{1, 65536}}));
        EXPECT_FALSE(accepts({{1, 65536}}, {{2, 65536}}));
    }

    TEST_F(test_hostinfo, DoesNotCountOverlapsTwice) {
        EXPECT_FALSE(accepts({{1, 32768}, {1, 32768}}, {{1, 65536}}));
    }

    TEST_F(test_hostinfo, RequiresSubordinateIdsInBothMaps) {
        EXPECT_FALSE(accepts({{0, 1}}, {{1, 65536}}));
        EXPECT_FALSE(accepts({{1, 65536}}, {{0, 1}}));
        EXPECT_FALSE(accepts({{0, 65536}}, {{0, 65536}}));
        EXPECT_FALSE(accepts({{1, 65535}}, {{1, 65536}}));
        EXPECT_FALSE(accepts({{1, 65536}}, {}));
    }

    TEST_F(test_hostinfo, RejectsInvalidOrIrrelevantRanges) {
        EXPECT_FALSE(accepts({{-1, 65537}}, {{1, 65536}}));
        EXPECT_FALSE(accepts({{1, -1}}, {{1, 65536}}));
        EXPECT_FALSE(accepts({{65537, 65536}}, {{1, 65536}}));
        EXPECT_TRUE(accepts({{1, 4294967294LL}}, {{1, 65536}}));
    }

    TEST_F(test_hostinfo, RootfulBypassRequiresExplicitFalse) {
        bool accepted = false;
        EXPECT_TRUE(HOST_INFO::parse_podman_info_subids(
            "host:\n  idMappings:\n    gidmap: []\n    uidmap: []\n  security:\n    rootless: false\n", accepted));
        EXPECT_TRUE(accepted);
        EXPECT_TRUE(HOST_INFO::parse_podman_info_subids(
            "host:\n  idMappings:\n    gidmap: []\n    uidmap: []\n", accepted));
        EXPECT_FALSE(accepted);
    }

    TEST_F(test_hostinfo, UnrecognizedOutputIsNotAConfigurationDiagnosis) {
        bool accepted = true;
        EXPECT_FALSE(HOST_INFO::parse_podman_info_subids("Error: unable to connect\n", accepted));
        EXPECT_FALSE(accepted);
    }
}

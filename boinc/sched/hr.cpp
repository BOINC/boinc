// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

#include "error_numbers.h"
#include "str_util.h"

#include "hr.h"

const int nocpu = 1;
const int Intel = 2;
const int AMD = 3;
const int Macintosh = 4;
const int AMDAthlon = 5;
const int AMDDuron = 6;
const int AMDSempron = 7;
const int AMDOpteron = 8;
const int AMDAthlon64 = 9;
const int AMDAthlonXP = 10;
const int IntelXeon = 11;
const int IntelCeleron = 12;
const int IntelPentium = 13;
const int IntelPentiumII = 14;
const int IntelPentiumIII = 15;
const int IntelPentium4 = 16;
const int IntelPentiumD = 17;
const int IntelPentiumM = 18;
const int AMDAthlonMP = 19;
const int AMDTurion = 20;
const int IntelCore2 = 21;

const int noos = 128;
const int Linux = 256;
const int Windows = 384;
const int Darwin = 512;
const int freebsd = 640;

inline int os(HOST& host){
    if (strcasestr(host.os_name, "Linux")) return Linux;
    else if (strcasestr(host.os_name, "Windows")) return Windows;
    else if (strcasestr(host.os_name, "Darwin")) return Darwin;
    else if (strcasestr(host.os_name, "FreeBSD")) return freebsd;
    else return noos;
};

inline int cpu_coarse(HOST& host){
    if (strcasestr(host.p_vendor, "Intel")) return Intel;
    if (strcasestr(host.p_vendor, "AMD")) return AMD;
    if (strcasestr(host.p_vendor, "Macintosh")) return Macintosh;
    return nocpu;
}

inline int cpu_fine(HOST& host){
    if (strcasestr(host.p_vendor, "Intel")) {
        if (strcasestr(host.p_model, "Xeon")) return IntelXeon;
        if (strcasestr(host.p_model, "Celeron")) {
            if (strcasestr(host.p_model, " M ")) return IntelPentiumM;
            if (strcasestr(host.p_model, " D ")) return IntelPentiumD;
            if (strcasestr(host.p_model, "III"))  return IntelPentiumIII;
            return IntelCeleron;
        }
        if (strcasestr(host.p_model, "Core")) return IntelCore2;
        if (strcasestr(host.p_model, "Pentium")) {
            if (strcasestr(host.p_model, "III"))  return IntelPentiumIII;
            if (strcasestr(host.p_model, "II"))  return IntelPentiumII;
            if (strcasestr(host.p_model, " 4 "))  return IntelPentium4;
            if (strcasestr(host.p_model, " D "))  return IntelPentiumD;
            if (strcasestr(host.p_model, " M "))  return IntelPentiumM;
            return IntelPentium;
        }
        if (strcasestr(host.p_model, "x86")) {
            if (strcasestr(host.p_model, "Family 6 Model 6")) return IntelCeleron;
            if (strcasestr(host.p_model, "Family 6 Model 9")) return IntelPentiumM;
            if (strcasestr(host.p_model, "Family 6 Model 10")) return IntelXeon;
            if (strcasestr(host.p_model, "Family 5 Model 1")) return IntelPentium;
            if (strcasestr(host.p_model, "Family 5 Model 2")) return IntelPentium;
            if (strcasestr(host.p_model, "Family 6 Model 1")) return IntelPentium;
            if (strcasestr(host.p_model, "Family 15 Model 1")) return IntelPentium4;
            if (strcasestr(host.p_model, "Family 15 Model 2")) return IntelPentium4;
            if (strcasestr(host.p_model, "Family 6 Model 7")) return IntelPentiumIII;
            if (strcasestr(host.p_model, "Family 6 Model 8" )) return IntelPentiumIII;
            if (strcasestr(host.p_model, "Family 6 Model 11")) return IntelPentiumIII;
            if (strcasestr(host.p_model, "Family 6 Model 3")) return IntelPentiumII;
            if (strcasestr(host.p_model, "Family 6 Model 5")) return IntelPentiumII;
        }
        return Intel;
    }
    if (strcasestr(host.p_vendor, "AMD")) {
        if (strcasestr(host.p_model, "Duron")) return AMDDuron;
        if (strcasestr(host.p_model, "Opteron")) return AMDOpteron;
        if (strcasestr(host.p_model, "Sempron")) return AMDSempron;
        if (strcasestr(host.p_model, "Turion")) return AMDTurion;
        if (strcasestr(host.p_model, "Athlon")) {
            if (strcasestr(host.p_model, "XP"))  return AMDAthlonXP;
            if (strcasestr(host.p_model, "MP"))  return AMDAthlonMP;
            if (strcasestr(host.p_model, "64"))  return AMDAthlon64;
            return AMDAthlon;
        }
        return AMD;
    }
    if (strcasestr(host.p_vendor, "Macintosh")) return Macintosh;
    return nocpu;
};

// call this ONLY if hr_unknown_class() returns false

int hr_class(HOST& host, int hr_type) {
    switch (hr_type) {
    case 1:
        return os(host) + cpu_fine(host);
    case 2:
        switch (os(host)) {
        case Windows:
        case Linux:
            return os(host);
        case Darwin:
            return os(host) + cpu_coarse(host);
        }
    }
    return 0;
}

bool hr_unknown_class(HOST& host, int hr_type) {
    switch (hr_type) {
    case 1:
        if (os(host) == noos) return true;
        if (cpu_fine(host) == nocpu) return true;
        return false;
    case 2:
        switch (os(host)) {
        case Windows:
        case Linux:
            return false;
        case Darwin:
            switch(cpu_coarse(host)) {
            case Intel:
            case Macintosh:
                return false;
            }
            return true;
        }
        return true;
    }
    return false;
}

const char* hr_names[HR_NTYPES] = {"", "fine", "coarse"};
int hr_nclasses[HR_NTYPES] = {0, 768, 768};

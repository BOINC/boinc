// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef _VALIDATE_UTIL2_
#define _VALIDATE_UTIL2_

#include <vector>

extern int init_result(RESULT&, void*&);
extern int compare_results(RESULT &, void*, RESULT const&, void*, bool&);
extern int cleanup_result(RESULT const&, void*);
extern double compute_granted_credit(WORKUNIT&, std::vector<RESULT>& results);

#endif

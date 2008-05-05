// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#define MAX_CPU_BENCHMARKS_SECONDS	300
#define CPU_BENCHMARKS_RUNNING		0
#define CPU_BENCHMARKS_COMPLETE		1
#define CPU_BENCHMARKS_NOT_RUNNING	2	
#define CPU_BENCHMARKS_ERROR		3

#define BM_TYPE_FP       0
#define BM_TYPE_INT      1

extern int dhrystone(double& vax_mips, double& loops, double& cpu_time, double min_cpu_time);
extern int whetstone(double& flops, double& cpu_time, double min_cpu_time);
extern void benchmark_wait_to_start(int which);
extern bool benchmark_time_to_stop(int which);

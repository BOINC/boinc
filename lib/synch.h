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

#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>

extern int create_semaphore(key_t);
extern int destroy_semaphore(key_t);
extern int lock_semaphore(key_t);
extern int unlock_semaphore(key_t);
extern int get_key(char* path, int id, key_t&);

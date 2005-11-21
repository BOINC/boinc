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
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// Untested and unsupported code for checkpointing multiple files.

#include "config.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdarg>

using namespace std;

// abstract interface - derive classes from this base class
class BoincCheckpointFile
{
public:
    // these functions should set error on f on failure

    // this is only called once each time an application is resumed
    virtual void input(istream& f) = 0;

    // this is called at every checkpoint
    virtual void output(ostream& f) = 0;
};

// checkpoint class good for binary dump of struct as a state file
class BoincRawDataCheckpointFile : public BoincCheckpointFile {
    void* data;
    size_t length;
public:
    BoincRawDataCheckpointFile(void* data_, size_t length_)
        :data(data_), length(length_)
    {
    }

    virtual void input(istream& f)
    {
        f.read((char*) data, length);
        if (!f.eof())
            f.clear(ios::badbit);
    }

    virtual void output(ostream& f)
    {
        f.write((char const*) data, length);
    }
};

// Class that is good for writing or appending (text or binary) data.
// Use standard C++ iostream operators to output, or printf
class BoincStreamCheckpointFile : public BoincCheckpointFile, public stringstream
{
public:
    virtual void input(istream& f)
    {
        // read entire file into memory buffer (which is a stringbuf)
        f >> rdbuf();
    }

    virtual void output(ostream& f)
    {
        // write entire memory buffer to file
        seekg(0);
        f << rdbuf();
    }

    void printf(const char* format, ...)
    {
        va_list ap;
        char buf[20000];

        va_start(ap, format);
        vsprintf(buf, format, ap);
        va_end(ap);
        *this << buf;
    }
};

// AtomicFileSet defines a set of files which are written/read atomically.  If
// the system crashes within a write(), the next read() will read what was
// written by the previous write().  Assumes that writing a single integer is
// atomic - a much safer assumption than just writing an arbitrary number of
// files of arbitrary length.
class AtomicFileSet
{
    struct NamedCheckpointFile {
        const char* filename;
        BoincCheckpointFile* file;
        bool keep;
    };

    int current_checkpoint_number;
    typedef vector<NamedCheckpointFile> FilesList;
    FilesList files;

    // returns true on success
    bool read_checkpoint_number()
    {
        ifstream f("boinc_checkpoint.dat");
        return (bool) (void*) (f >> current_checkpoint_number);
    }

    void write_checkpoint_number() const
    {
        ofstream f("boinc_checkpoint.dat");
        f << current_checkpoint_number;
    }

    string format_checkpointed_filename(const char* filename, int checkpoint_number)
    {
        char buf[1024];
        sprintf(buf, "%s.%d", filename, checkpoint_number);
        return buf;
    }

    void delete_files(int checkpoint_number)
    {
        for (FilesList::const_iterator i = files.begin(); i != files.end(); ++i) {
            string cp_filename =
                format_checkpointed_filename(i->filename, checkpoint_number);
            if (unlink(cp_filename.c_str())) {
                cerr << "Couldn't unlink \"" << cp_filename << "\": " << strerror(errno) << endl;
            }
        }
    }

public:
    AtomicFileSet() : current_checkpoint_number(0) {}

    // add a file to the set of files to atomically write.  if keep, then the
    // file exists as 'filename' after Finish(); else it is deleted after
    // Finish().
    void add(const char* filename,
             BoincCheckpointFile* file,
             bool keep = true)
    {
        NamedCheckpointFile namedfile;
        namedfile.filename = filename;
        namedfile.file = file;
        namedfile.keep = keep;
        files.push_back(namedfile);
    }

    // call when resuming. returns true on success of all files, false on failure.
    bool read()
    {
        if (!read_checkpoint_number()) return false;

        for (FilesList::const_iterator i = files.begin(); i != files.end(); ++i) {
            string cp_filename =
                format_checkpointed_filename(i->filename, current_checkpoint_number);
            ifstream f(cp_filename.c_str(), ios::binary);
            if (!f) {
                cerr << "Error opening for input \"" << cp_filename << "\"\n";
                return false;
            }
            i->file->input(f);
            if (!f) {
                cerr << "Error reading \"" << cp_filename << "\"\n";
                return false;
            }
        }
        // success
        return true;
    }

    void write()
    {
        // strategy:
        //   1. write *.N
        //   2. write N to checkpoint file
        //   3. delete *.(N-1)

        for (FilesList::const_iterator i = files.begin(); i != files.end(); ++i) {
            string cp_filename =
                format_checkpointed_filename(i->filename, current_checkpoint_number);
            ofstream f(cp_filename.c_str(), ios::binary);
            if (!f) {
                cerr << "Couldn't open output \"" << cp_filename << "\"\n";
                exit(101);
            }
            i->file->output(f);
            if (!f) {
                cerr << "Error writing \"" << cp_filename << "\"\n";
                exit(102);
            }
        }

        write_checkpoint_number();

        delete_files(current_checkpoint_number-1);
        ++current_checkpoint_number;
    }

    // NOTE: you must call write() yourself, if there is any data to be written.
    void finish()
    {
        // delete files that are no longer needed, and rename the ones we
        // want to keep.
        int checkpoint_number = current_checkpoint_number-1;
        for (FilesList::const_iterator i = files.begin(); i != files.end(); ++i) {
            string cp_filename =
                format_checkpointed_filename(i->filename, checkpoint_number);
            if (i->keep) {
                if (unlink(cp_filename.c_str())) {
                    cerr << "Warning: Couldn't unlink \"" << cp_filename
                         << "\": " << strerror(errno) << endl;
                }
            } else {
                if (rename(cp_filename.c_str(), i->filename)) {
                    cerr << "Fatal error: Couldn't rename \"" << cp_filename
                         << "\" to \"" << i->filename << "\": "
                         << strerror(errno) << endl;
                    exit(100);
                }
            }
        }
    }
};

/* usage in astropulse:

AtomicFileSet files;
ostream* output;

void init()
{
    files.add("ap_state.dat",
              new BoincRawDataCheckpointFile(client.state, sizeof(client.state)));
    files.add("pulse.out", (output=new BoincStreamCheckpointFile));
    if (files.read()) {
        // resuming
    } else {
        *output << "<astropulse> ...";
    }
}

void output_pulse()
{
    *output << "<pulse> ... </pulse>";
}

void checkpoint()
{
    files.write();
}

*/

const char *BOINC_RCSID_f3d3fb27ca = "$Id$";

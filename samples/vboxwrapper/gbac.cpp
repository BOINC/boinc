
#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#else
#include <stdio.h>
#endif

#include <iostream>
#include <fstream>


#include "filesys.h"
#include "str_replace.h"
#include "util.h"
#include "parse.h"
#include "gbac.h"
#include "boinc_api.h"

#define GBAC_EXEC_LOG  "gbac-exec.log"
#define GBAC_VAUNZIP_STATUS "gbac-va-unzipped.status"

static const char* dc_files[] =
{
    "dc_stdout.txt",
    "dc_stderr.txt",
    "dc_clientlog.txt",
    "dc_ckpt_out",
    NULL
};

static const char* log_files[] =
{
    "shared/guest-tools-exec.log",
    "shared/gbac-app.stdout",
    "shared/gbac-app.stderr",
    "shared/gbac_job.xml",
    "shared/gbac_exit_status",
    "shared/gbac_command_line.xml",
    "shared/stdout.txt",
    "shared/stderr.txt",
    "shared/gbac-execution.stdout",
    "shared/gbac-execution.stderr",
    "shared/shared-dir-contents.log",
    "shared/dc_clientlog.txt",
    "shared/dc_stdout.txt",
    "shared/dc_stderr.txt",
    NULL
};


GBAC gbac;


GBAC::GBAC()
{
    this->environment.clear();
}


GBAC::~GBAC()
{
    free(this->hostdir);
    free(this->dirsep);
}


int GBAC::init(int argc_, char **argv_)
{
    char resolved_buffer[2048];
    char msg_buf[256];

    this->argc = argc_;
    this->argv = argv_;

    this->hostdir = strdup("shared");
    if (this->hostdir == NULL)
        return EXIT_FAILURE;

    #ifdef _WIN32
        this->dirsep = strdup("\\");
    #else
        this->dirsep = strdup("/");
    #endif
    if (this->dirsep == NULL)
        return EXIT_FAILURE;

    // need to create various files expected by DC-API
    // in case the application fails, DC-API still expects them
    FILE* f;
    for (int i=0; dc_files[i] != NULL; i++)
    {
        boinc_resolve_filename(dc_files[i], resolved_buffer,
                               sizeof(resolved_buffer));


        f = fopen(resolved_buffer, "w");
        if (f) {
            fclose(f);
        } else {
            fprintf(stderr,
                    "%s failed to create DC-API file '%s'\n",
                    boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
                    dc_files[i]);
            return EXIT_FAILURE;
        }
    }
   return 0;
}


int GBAC::prepareHostSharedDir()
{
    DIRREF mydir;
    char msg_buf[256];
    char buffer[1024];
    char dest_buffer[2048];
    char resolved_buffer[2048];

    if (!boinc_file_exists(this->hostdir))
    {
        fprintf(stderr,
                "%s creating host shared directory.\n",
                boinc_msg_prefix(msg_buf, sizeof(msg_buf)));
        if (boinc_mkdir(this->hostdir) != 0)
        {
            fprintf(stderr,
                   "%s could not create host shared directory"\
                   ": %s\n",
                   boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
                   this->hostdir);
            return EXIT_FAILURE;
        }
    }
    else if (is_file(this->hostdir))
    {
        fprintf(stderr,
                "%s there is a file with the same name as "\
                "the host share dir in the slot directory.\n",
                boinc_msg_prefix(msg_buf, sizeof(msg_buf)));
        return EXIT_FAILURE;
    }

    fprintf(stderr,
            "%s === copying files to host directory ===\n",
            boinc_msg_prefix(msg_buf, sizeof(msg_buf)));
    mydir = dir_open(".");
    while (dir_scan(buffer, mydir, sizeof(buffer))==0)
    {
        if (strstr(buffer, ".vdi") == NULL &&
            strcmp(buffer, "vbox_job.xml") &&
            strcmp(buffer, "boinc_lockfile") &&
            strcmp(buffer, "stderr.txt") &&
            strcmp(buffer, "boinc_finish_called") &&
//            strcmp(buffer, "gbac_job.xml") &&
            strcmp(buffer, "init_data.xml") &&
            strcmp(buffer, "boinc_task_state.xml") &&
            strcmp(buffer, "vbox_checkpoint.txt") &&
            strcmp(buffer, this->argv[0]) &&
            !is_dir(buffer))
        {
                if (boinc_resolve_filename(buffer,
                                           resolved_buffer,
                                           sizeof(resolved_buffer)) != 0)
                {
                    fprintf(stderr,
                            "%s cannot resolve filename: '%s'",
                            boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
                            buffer);
                    continue;
                }

                snprintf(dest_buffer, sizeof(dest_buffer),
                         "%s%s%s",
                         this->hostdir, this->dirsep, buffer);
                boinc_copy(resolved_buffer, dest_buffer);
                fprintf(stderr,
                        "%s          '%s'  ->  '%s'\n",
                        boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
                        resolved_buffer, dest_buffer);
        }
    }
    dir_close(mydir);

    // create gbac_command_line.xml file
    int i;
    char arg_buffer[4096];
    FILE *cl_file;
    char cl_file_buffer[1024];

    string arguments = "";

    memset(arg_buffer, 0, sizeof(arg_buffer));

    for (i=1; i<this->argc; i++)
    {
        arguments.append(" ");
        arguments.append(this->argv[i]);
    }

    fprintf(stderr,
            "%s command line is: '%s'\n",
            boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
            arguments.c_str());
    snprintf(arg_buffer, sizeof(arg_buffer),
             "<gbac_command_line>\n"\
             "        <command_line>%s</command_line>\n"\
             "</gbac_command_line>\n",
             arguments.c_str());
    snprintf(cl_file_buffer, sizeof(cl_file_buffer), "%s%s%s",
             this->hostdir, this->dirsep, "gbac_command_line.xml");

    cl_file = fopen(cl_file_buffer, "w+");
    if (cl_file == NULL)
    {
        fprintf(stderr,
                "%s cannot create command line file: '%s'\n",
                boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
                cl_file_buffer);
        return EXIT_FAILURE;
    }
    fputs(arg_buffer, cl_file);
    fclose(cl_file);
    return 0;
}


int GBAC::copyOutputFiles()
{
    DIRREF mydir;
    char msg_buf[256];
    char buffer[1024];
    char src_buffer[2048];
    char resolved_buffer[2048];

    if (!boinc_file_exists(this->hostdir))
    {
        fprintf(stderr,
                "%s host shared directory does not exist.\n",
                boinc_msg_prefix(msg_buf, sizeof(msg_buf)));
        return EXIT_FAILURE;
    }
    fprintf(stderr,
            "%s === copying files back to project/slot directory ===\n",
            boinc_msg_prefix(msg_buf, sizeof(msg_buf)));

    mydir = dir_open(this->hostdir);
    while (dir_scan(buffer, mydir, sizeof(buffer))==0)
    {
        if (strcmp(buffer, "boinc_finish_called") &&
            strcmp(buffer, "boinc_lockfile") &&
            strcmp(buffer, "stderr.txt") &&
            strcmp(buffer, "stdout.txt") &&
            strcmp(buffer, "gbac-execution.log") &&
            strcmp(buffer, "gbac-execution.stderr") &&
            strcmp(buffer, "gbac-execution.stdout") &&
            strcmp(buffer, "gbac_command_line.xml") &&
            strcmp(buffer, "gbac_exit_status") &&
            strcmp(buffer, "gbac_job.xml"))
        {
            snprintf(src_buffer, sizeof(src_buffer),
                     "%s%s%s",
                     this->hostdir, this->dirsep, buffer);
            if (is_dir(src_buffer))
                continue;
            if (boinc_resolve_filename(buffer,
                                       resolved_buffer,
                                       sizeof(resolved_buffer)) != 0)
            {
                printf("cannot resolve '%s'\n", buffer);
                continue;
            }
            boinc_copy(src_buffer, resolved_buffer);
            fprintf(stderr,
                    "%s          '%s'  ->  '%s'\n",
                    boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
                    src_buffer, resolved_buffer);
        }

    }
    return 0;
}


int GBAC::copyLogFiles()
{
    ofstream output_file;
    ifstream input_file;
    string line;
    char msg_buf[8192];

    fprintf(stderr,
            "%s Appending all output files to this logfile:\n",
            boinc_msg_prefix(msg_buf, sizeof(msg_buf)));

    for (int i=0; log_files[i] != NULL; i++)
    {
        input_file.open(log_files[i], ios::in | ios::binary);
        if (!input_file.is_open())
        {
            // for gbac_job.xml we give just a notice
            if (!strcmp(log_files[i], "shared/gbac_job.xml"))
            {
                fprintf(stderr,
                        "%s NOTICE: Cannot open output file '%s' "
                        "for reading\n",
                        boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
                        log_files[i]);
            } else {
                fprintf(stderr,
                        "%s Cannot open output file '%s' for reading\n",
                        boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
                        log_files[i]);
            }
            continue;
        }
        fprintf(stderr,
                "%s >>>>>>>>>>  %s starts here  >>>>>>>>>>\n\n",
                boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
                log_files[i]);
        output_file.open("stderr.txt", ios::out | ios::app);
        if (!output_file.is_open())
        {
            fprintf(stderr,
                    "%s Cannot open output file 'stderr.txt' for writing\n",
                    boinc_msg_prefix(msg_buf, sizeof(msg_buf)));
            continue;
        }
        output_file.flush();
        output_file.seekp(0, ios_base::end);

        while (input_file.good())
        {
           getline(input_file, line);
           output_file << "  > " << line << endl;
        }
        input_file.close();
        output_file.flush();
        output_file.close();
        fprintf(stderr,
                "\n%s <<<<<<<<<<  %s ends here  <<<<<<<<<<\n",
                boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
                log_files[i]);
    }

    fprintf(stderr,
            "%s Done appending all output files to this logfile.\n",
            boinc_msg_prefix(msg_buf, sizeof(msg_buf)));

    //
    // check if stderr.txt and stdout.txt are requested as output files, and copy
    // the content of the gbac-app.stdout and gbac-app.stderr local files
    // (in the slot directory) to the output files (in the project directory).
    //
    // These files contain the stdout and stderr of the application run in the VM.
    std::string file_stderr;
    std::string file_stdout;

    boinc_resolve_filename_s(STDERR_FILE, file_stderr);
    boinc_resolve_filename_s(STDOUT_FILE, file_stdout);

    int cmpresult = 0;
    cmpresult = file_stdout.compare(STDOUT_FILE);
    if (cmpresult != 0) {
    	fprintf(stderr, "%s '%s' is requested as output file, copying "
  		    "contents of standard output of the application "
  		    "(gbac-app.stdout) to this file.\n",
		    boinc_msg_prefix(msg_buf, sizeof(msg_buf)), STDOUT_FILE);
        std::ifstream ifs("gbac-app.stdout", std::ios::binary);
        std::ofstream ofs(file_stdout.c_str(), std::ios::binary);
        ofs << ifs.rdbuf();
        ifs.close();
        ofs.close();
    } else {
    	fprintf(stderr, "%s '%s' is NOT requested as output file.\n",
  		    boinc_msg_prefix(msg_buf, sizeof(msg_buf)), STDOUT_FILE);
    	fprintf(stderr, "%s ('%s'.compare('%s') == %d)\n",
  		    boinc_msg_prefix(msg_buf, sizeof(msg_buf)), STDOUT_FILE,
  		    file_stdout.c_str(), cmpresult);
    }
    cmpresult = file_stderr.compare(STDERR_FILE);
    if (cmpresult != 0) {
    	fprintf(stderr, "%s '%s' is requested as output file, copying "
  		    "contents of standard error of the application "
  		    "(gbac-app.stderr) to this file.\n",
  		    boinc_msg_prefix(msg_buf, sizeof(msg_buf)), STDERR_FILE);
      	std::ifstream ifs("gbac-app.stderr", std::ios::binary);
      	std::ofstream ofs(file_stderr.c_str(), std::ios::binary);
      	ofs << ifs.rdbuf();
        ifs.close();
        ofs.close();
    } else {
    	fprintf(stderr, "%s '%s' is NOT requested as output file.\n",
  		    boinc_msg_prefix(msg_buf, sizeof(msg_buf)), STDERR_FILE);
    	fprintf(stderr, "%s ('%s'.compare('%s') == %d)\n",
  		    boinc_msg_prefix(msg_buf, sizeof(msg_buf)), STDERR_FILE,
  		    file_stderr.c_str(), cmpresult);
    }

    return 0;
}


int GBAC::copyDebugLog()
{
    // Copy the contents of the BOINC stderr to a special log file
    // (GBAC_EXEC_LOG) if requested. This serves better debugging
    // when submitting jobs from other DCI's, e.g., gLite.
    //
    // This method should be called right before boinc_finish().

    std::string file_debuglog;
    char msg_buf[8192];

    boinc_resolve_filename_s(GBAC_EXEC_LOG, file_debuglog);
    int cmpresult = 0;
    cmpresult = file_debuglog.compare(GBAC_EXEC_LOG);
    if (cmpresult != 0) {
        fprintf(stderr, "%s '%s' is requested as output file, copying "
  		    "contents of standard error of BOINC (contains all logs) "
  		    "to this file.\n",
  		    boinc_msg_prefix(msg_buf, sizeof(msg_buf)), GBAC_EXEC_LOG);
        fflush(stderr);
      	std::ifstream ifs(STDERR_FILE, std::ios::binary);
      	std::ofstream ofs(file_debuglog.c_str(), std::ios::binary);
      	ofs << ifs.rdbuf();
        ifs.close();
        ofs.close();
    } else {
    	fprintf(stderr, "%s '%s' is NOT requested as output file.\n",
  		    boinc_msg_prefix(msg_buf, sizeof(msg_buf)), STDERR_FILE);
    	fprintf(stderr, "%s ('%s'.compare('%s') == %d)\n",
  		    boinc_msg_prefix(msg_buf, sizeof(msg_buf)), STDERR_FILE,
  		    file_debuglog.c_str(), cmpresult);
    }

    return 0;
}


int GBAC::parse(const char* file)
{
   return 0;
}


int GBAC::getExitStatus(int &status)
{
    ifstream input_file;
    string line;
    char msg_buf[1024];

    input_file.open("shared/gbac_exit_status", ios::in | ios::binary);
    if (input_file.is_open())
    {
        getline(input_file, line);
        fprintf(stderr,
                "%s getExitStatus(): Content of shared/gbac_exit_status "
                "is '%s'.\n",
                boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
                line.c_str());
        input_file.close();
        status = strtol(line.c_str(), 0, 0);
        if (errno == ERANGE)
        {
            fprintf(stderr,
                    "%s getExitStatus(): Cannot convert value '%s'.\n",
                    boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
                    line.c_str());
            return 1;
        }
    } else {
        fprintf(stderr,
                "%s getExitStatus(): Cannot open 'shared/gbac_exit_status' "
                "for reading.\n",
                boinc_msg_prefix(msg_buf, sizeof(msg_buf)));
        return 1;
    }
    return 0;
}

int GBAC::doGunzip(const char* strGZ, const char* strInput, bool bKeep) {
    unsigned char buf[1024];
    char msg_buf[MSG_CHANNEL_SIZE];
    long lRead = 0,
         lWrite = 0;
    int iGZErr = 0;
    // take an input file (strInput) and turn it into a compressed file (strGZ)
    // get rid of the input file after
    //s.quit_request = 0;
    //checkBOINCStatus();
    FILE* fIn = boinc_fopen(strInput, "wb");
    if (!fIn) {
        fprintf(stderr,
                "%s ERROR: doGunzip(): Error opening '%s'.\n",
                boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
                strInput);
        return false; //error
    }
    gzFile fOut = gzopen(strGZ, "rb");
    if (!fOut) {
        fprintf(stderr,
                "%s ERROR: doGunzip(): Error opening '%s'.\n",
                boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
                strGZ);
        return false; //error
    }
    fseek(fIn, 0, SEEK_SET);  // go to the top of the files
    gzseek(fOut, 0, SEEK_SET);
    while (!gzeof(fOut)) { // read 1KB at a time until end of file
        memset(buf, 0x00, 1024);
        lRead = 0;
        lWrite = 0;
        lRead = (long) gzread(fOut, buf, 1024);
        if (lRead < 0) {
            fprintf(stderr,
                    "%s ERROR: doGunzip(): gzread() encountered an error: %s.\n",
                    boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
                    gzerror(fOut, &iGZErr));
            break;
        }
        lWrite = (long) fwrite(buf, 1, lRead, fIn);
        if (lRead != lWrite) { //error -- read bytes != written bytes
            fprintf(stderr,
                    "%s ERROR: doGunzip(): Write error gunzipping '%s': read %ld bytes, "
                    "but written %ld bytes.\n",
                    boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
                    strGZ, lRead, lWrite);
            break;
        }
        //boinc_get_status(&s);
        //if (s.quit_request || s.abort_request || s.no_heartbeat) break;
    }
    gzclose(fOut);
    fclose(fIn);
    //checkBOINCStatus();
    if (lRead != lWrite) {
        return false;
    }
    // if we made it here, it compressed OK, can erase strInput and leave
    if (!bKeep) {
        boinc_delete_file(strGZ);
    }
    return true;
}


int GBAC::prepareVa(std::string &strVaFilename) {
    char msg_buf[MSG_CHANNEL_SIZE];
    std::string strVaUngzippedname;
    std::ofstream outfile;
    if (!access(GBAC_VAUNZIP_STATUS, R_OK)) { // returns zero on success
        fprintf(stderr,
                "%s INFO: prepareVa(): VA '%s' is already uncompressed\n",
                boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
                strVaFilename.c_str());
        return 0;
    }
    if (!hasEnding(strVaFilename, ".gz")) {
        fprintf(stderr,
                "%s INFO: prepareVa(): VA '%s' is propably not compressed. Filename should end with '.gz'.\n",
                boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
                strVaFilename.c_str());
        return 1; // do nothing, filename should end with .gz
    }
    // remove '.gz' from the end
    strVaUngzippedname = strVaFilename.substr(0, strVaFilename.size() - 3);
    if (access(strVaFilename.c_str(), R_OK) == -1) {
        fprintf(stderr,
                "%s ERROR: prepareVa(): Cannot access VA '%s'.\n",
                boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
                strVaFilename.c_str());
        return 1; // error - file access
    }
    fprintf(stderr,
            "%s prepareVa(): Uncompressing VA '%s'.\n",
            boinc_msg_prefix(msg_buf, sizeof(msg_buf)),
            strVaFilename.c_str());
    if (!doGunzip(strVaFilename.c_str(), strVaUngzippedname.c_str(), true)) {
        return 1;
    } else {
        outfile.open(GBAC_VAUNZIP_STATUS, ios::out | ios::trunc);
        outfile << "va unzipped" << std::endl;
        outfile.close();
        strVaFilename = strVaUngzippedname;
        return 0;
    }
}


int GBAC::hasEnding(std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return true;
    }
}

int GBAC::printVersion() {
    char buf[256];

    fprintf(stderr, "%s GBAC %s (build date: %s)\n",
        boinc_msg_prefix(buf, sizeof(buf)), SVNREV, __DATE__);
    return 0;
}

#ifndef _ACCOUNT_
#define _ACCOUNT_

extern int write_account_file(
    char* master_url, char* authenticator, char* project_prefs=0
);
extern int add_new_project();
extern int parse_account_files();

#endif

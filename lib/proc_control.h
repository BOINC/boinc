#ifdef _WIN32

extern HANDLE sandbox_account_token;
extern PSID sandbox_account_sid;
extern void get_sandbox_account_token();

extern int run_program(
    const char* path, const char* cdir, int argc, char *const argv[], double, HANDLE&
);

extern void kill_program(HANDLE);
extern int get_exit_status(HANDLE);
extern bool process_exists(HANDLE);

#else
extern int run_program(
    const char* path, const char* cdir, int argc, char *const argv[], double, int&
);
extern void kill_program(int);
extern int get_exit_status(int);
extern bool process_exists(int);
#endif

extern int wait_client_mutex(const char* dir, double timeout);

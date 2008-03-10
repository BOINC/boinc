extern HANDLE sandbox_account_token;
extern PSID sandbox_account_sid;
extern void get_sandbox_account_token();

extern int run_app_windows(
    const char* path, const char* cdir, int argc, char *const argv[], HANDLE&
);

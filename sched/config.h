// info needed by server-side software
//
class CONFIG {
public:
    char db_name[256];
    char db_passwd[256];
    int shmem_key;
    char key_dir[256];
    char upload_dir[256];
    char user_name[256];

    int parse(FILE*);
    int parse_file();
};

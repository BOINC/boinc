// length of buffer to hold an MD5 hash
#define MD5_LEN 64

extern int md5_file(char* path, char* output, double& nbytes);
extern int md5_block(unsigned char* data, int nbytes, char* output);


#define DC_CFG_OK             0 
#define DC_CFG_FILENOTEXISTS  1

/** Parse the config file and store name=value pairs in memory
 * 
 */
int dc_cfg_parse(const char *cfgfile);

/** Get a value for a given name
 * 
 */
char * dc_cfg_get(char *name);

#define DC_CFG_OK             0 
#define DC_CFG_FILENOTEXISTS  1
#define DC_CFG_OUTOFMEM	      2

/** Parse the config file and store name=value pairs in memory
 * 
 */
int dc_cfg_parse(const char *cfgfile);

/** Get a value for a given name
 * 
 */
char * dc_cfg_get(char *name);

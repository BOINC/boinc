#ifndef __REMOVE_H_
#define __REMOVE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Function: Removes a job from the info system. 
 * Arguments: 
 *   - wuname:  the name of the selected job in the info system*/
	
int remove_wu(char *wuname);

#ifdef __cplusplus
}
#endif

#endif /* __RMOVE_H_ */

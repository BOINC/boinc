/*
     File:       LoginItemAPI.h
 
     Copyright:  © 2000-2003 by Apple Computer, Inc., all rights reserved.
 
     Bugs?:      For bug reports, consult the following page on
                 the World Wide Web:
 
                     http://developer.apple.com/bugreporter/
 
*/

#if defined(__MWERKS__)  //for use with CodeWarrior
#   include <Carbon.h> //making a carbon project so need carbon.h
#	include <SIOUX.h> //this is used for window control in CodeWarrior
#else //for use with project builder
#	import <CoreFoundation/CoreFoundation.h>  //This is the only ProjectBuilder include we really needed
#	import <CoreFoundation/CFPreferences.h>  //This is where all the CFPreference APIs come from.
#	import <stdio.h> //used when printing out to standard output
#endif

//These are used as constants for the second argument to  AddLoginItemWithPropertiesToUser.  Either the application is hidden on launch or shows up this property decides this.  
#define kHideOnLaunch      true
#define kDoNotHideOnLaunch false

//These are used as constants for the third argument to  AddLoginItemWithPropertiesToUser.  You can either set the properties of the current user or all the users on the system (with kAllUsers).
#define kCurrentUser kCFPreferencesCurrentUser
#define kAllUsers    kCFPreferencesAnyUser

//#define kAppStr "loginwindow"  
#define kAppStr "loginwindow"  
//The preference name where LoginItem information is stored.  We are actually writing to the file loginwindow.plist in either /Users/Library/Preferences/ directory (for the kCurrentUser) or the /Library/Preferences folder (for All Users).   Note that if you want to install this for all users on the system then you must be root/admin to do so.  If you need to know how to become root check out the sample code: "AuthSample" on the developer.apple.com website

#define kKeyStr "AutoLaunchedApplicationDictionary" //The required "special" key for accessing LoginItem information.

//defining types which are used as the request types into the function GetListOfLoginItems.  kFullPathInfo causes the routine to return the full path as a character string, kApplicationNameInfo causes just the application or alias name to be returned, kHideInfo returns if the application is hidden on launch or not.

#define kFullPathInfo 		1
#define kApplicationNameInfo 	2
#define kHideInfo 		3

//This is the character array size used by GetListOfLoginItems
#define kLoginItemInfoLength 1025

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************/
// AddLoginItemWithPropertiesToUser
/***************************************************************************************************/
// This function will add a LoginItem to the list of LoginItems when called.  The properties
// given to the new LoginItem are passed when calling the function.  Note that *no* check is made
// when adding the LoginItem to ensure that the path points to a valid application.  Note that the LoginItem
// Is always added to the *end* of the list of LoginItems.
/***************************************************************************************************/
// Input Parameters:
// First Parameter (WhosPreferencesToChange) -> A constant which represents which users preferences
//                we want to change.  In this case there are two alternatives: kCurrentUser which
//                changes the preferences of the current user.  The second alternative is kAllUsers.
//                The kAllUsers LoginItems are launched for all users on the system.  You must be root
//                or admin to use the kAllUsers option.
// Second Parameter (AbsolutePath) -> 	The absolute path of the application to be launched 
//                expressed as a Standard C string.  (example: "/Applications/Clock")
// Third Parameter (HidetheApplicationOnStartup) -> A value representing if you want
//                your application to be hidden at login time. kHideOnLaunch if you want the 
//                application to be hidden after it is launched.  If you want the application to show
//                up normally use be hidden use kDoNotHideOnLaunch
//
// Output Parameters:
// Return -> 	This function returns a boolean value representing if the function was successful.
//              The function returns true if the LoginItem was successfully added.  False if
//              otherwise.  No additional error codes are returned. 
// Other Notes: This code doesn't work properly if more than one application is attempting to write to
//              the LoginItems preference at once. 
//              Also no check is made to ensure that what you are adding to the LoginItem list isn't a 
//              duplicate.  If there is a duplicate however only one instance of the application is launched
/***************************************************************************************************/

Boolean AddLoginItemWithPropertiesToUser(CFStringRef WhosPreferencesToChange, const char* AbsolutePath, Boolean HidetheApplicationOnLaunch);

/***************************************************************************************************/
// ReturnLoginItemPropertyAtIndex
/***************************************************************************************************/
// This function will return the list of LoginItems as a CFArray.  
/***************************************************************************************************/
// Input Parameters:
// First Parameter (WhosPreferencesToList) -> 	A constant representing which users preferences 
//                you want to examine.  The two alternatives are: kCurrentUser which lists the LoginItems
//                which are specific to that user (these are preferences found in /Users/<username>/Library/Preferences)
//            	  and kAnyUsers which lists LoginItems which are common to all users on the system (these
//                preferences are found in /Library/Preferences/).
// Second Parameter (RequestType) -> A constant representing which type of request for information you are making.  
//                There are currently three request types.  kFullPathInfo which causes this routine to return the 
//                absolute path to the LoginItem which is stored as a C String (example return value:
//                "/Applications/Clock.app").  Second alternative is kApplicationNameInfo which causes the routine to 
//                return the name of the application as a C string (example return value: "Clock.app").  Third 
//                alternative is kHideInfo which causes the routine to return the hide current status as a C string.  
//                The two return types possible are the C Strings "Hide" and "DoNotHide" (example return value: "Hide")
// Third Parameter (LoginItemIndex) -> An integer representing the LoginItem index.  Note that the LoginItems are 
//                stored as an array indexed from 0 to N-1 where N is the number of LoginItems.  Note that if an invalid
//                index is passed then you will get a return value of NULL indicating error status.
//
// Output Parameters:
// Return -> 	This function returns a value of NULL if an error has occurred (likely caused by attempting to index an
//              invalid LoginItemIndex).  If no error has occurred then a C String is returned which represents the data
//              requested on the LoginItem at the index given.  The three return types are: 
//              1) For a kFullPathInfo request the absolute path of the LoginItem is returned (example: 
//              "/Applications/TextEdit").  2) For a kApplicationNameInfo request only the LoginItem name is returned
//              with no path (example: "TextEdit").  3) For a kHideInfo request either the C string "Hide" is returned 
//              if LoginItem is hidden on launch or the C string value is "DoNotHide" if the LoginItem isn't hidden on 
//              launch
// Other Notes: The actual indexing of the LoginItem array starts at zero and goes up to N-1 where N is the number of
//              LoginItems.  Also, the order that the LoginItems are listed in the array is consistent with the ordering
//              of the LoginItems within the preference file.  Note that the ordering of the LoginItems in the array
//              *does not* correspond to the order which the LoginItems are launched.  The order which LoginItems are 
//              launched are determined by OSX at Login time and is based on what launch order would be most efficent.
/***************************************************************************************************/

char* ReturnLoginItemPropertyAtIndex(CFStringRef WhosPreferencesToList, int RequestType, int LoginItemIndex);

/***************************************************************************************************/
// GetCountOfLoginItems
/***************************************************************************************************/
// This function will return the number of LoginItems for the user requested.    
/***************************************************************************************************/
// Input Parameters:
// First Parameter (WhosPreferencesToList) -> 	A constant representing which users preferences 
//                you want the number of LoginItems for.  The two alternatives are: kCurrentUser which lists the 
//                LoginItems which are specific to that user (these are preferences found in 
//                /Users/<username>/Library/Preferences) and kAnyUsers which lists LoginItems which are common to all 
//                users on the system (these preferences are found in /Library/Preferences/).
//
// Output Parameters:
// Return -> 	This function returns the number of LoginItems for the user requested as a signed integer. 
/***************************************************************************************************/

int GetCountOfLoginItems(CFStringRef WhosPreferencesToList);

/***************************************************************************************************/
// RemoveLoginItemAtIndex
/***************************************************************************************************/
// This function will remove the LoginItem at the given index for the user specified.    
/***************************************************************************************************/
// Input Parameters:
// First Parameter (WhosPreferencesToList) -> 	A constant representing which users preferences 
//                you want to remove the LoginItem from.  The two alternatives are: kCurrentUser which removes login
//                items for the which are specific to that user (these are preferences found in 
//                /Users/<username>/Library/Preferences) and kAnyUsers which lists LoginItems which are common to all 
//                users on the system (these preferences are found in /Library/Preferences/).
// Second Parameter (LoginItemIndex) -> An integer representing the LoginItem index of the LoginItem you want removed. 
// 		 It is assumed that you will look up the LoginItems using a couple ReturnLoginItemPropertyAtIndex calls
//		 which will allow you to know which LoginItem you want to remove by index.  Note that the LoginItems are
//               stored as an array indexed from 0 to N-1 where N is the number of LoginItems.  Note that if an invalid
//               index is passed then you will get a return value of false indicating error status.
//
// Output Parameters:
// Return -> 	This function a boolean true if the LoginItem was successfully removed.  This function returns false if
//		the LoginItem could not be removed (or if it was not found).  *Note that when you remove a LoginItem 
//		from the list of LoginItems the index of all LoginItems following the removed item will be 
//		deincremented by one.  This means there will never be gaps in the list of LoginItems (i.e. empty slots 
//		in the array of LoginItems). 
/***************************************************************************************************/

Boolean RemoveLoginItemAtIndex(CFStringRef WhosPreferencesToChange, int LoginItemIndex);

/***************************************************************************************************/
// PrintOutLoginItemAtIndex
/***************************************************************************************************/
// This function will print out the LoginItem at the specified index 
/***************************************************************************************************/
// Input Parameters:
// First Parameter (WhosPreferencesToList) -> 	A constant representing which users preferences 
//                you want to print the LoginItem from.  The two alternatives are: kCurrentUser which removes login
//                items for the which are specific to that user (these are preferences found in 
//                /Users/<username>/Library/Preferences) and kAnyUsers which lists LoginItems which are common to all 
//                users on the system (these preferences are found in /Library/Preferences/).
// Second Parameter (LoginItemIndex) -> An integer representing the LoginItem index of the LoginItem you want to 
// 		print. Note that the LoginItems are stored as an array indexed from 0 to N-1 where N is the number of 
// 		LoginItems.  Note that if an invalid index is passed then you will get a return value of false 
//		indicating error status.
//
// Output Parameters:
// Return -> 	This function a boolean true if the LoginItem was successfully printed.  
/***************************************************************************************************/

Boolean PrintOutLoginItemAtIndex(CFStringRef WhosPreferencesToList, int LoginItemIndex);

/***************************************************************************************************/
// PrintOutAllLoginItems
/***************************************************************************************************/
// This function will print out all the LoginItems for the specified user 
/***************************************************************************************************/
// Input Parameters:
// First Parameter (WhosPreferencesToList) -> 	A constant representing which users preferences 
//                you want to print the LoginItems from.  The two alternatives are: kCurrentUser which removes login
//                items for the which are specific to that user (these are preferences found in 
//                /Users/<username>/Library/Preferences) and kAnyUsers which lists LoginItems which are common to all 
//                users on the system (these preferences are found in /Library/Preferences/).
//
// Output Parameters:
// Return -> 	This function a boolean true if the LoginItem was successfully printed.  
/***************************************************************************************************/

Boolean PrintOutAllLoginItems(CFStringRef WhosPreferencesToList);

/***************************************************************************************************/
// Input Parameters:
// First Parameter (WhosPreferencesToList) -> 	A constant representing which users preferences 
//                you want to print the number of LoginItems from.  The two alternatives are: kCurrentUser 
// 		which removes login items for the which are specific to that user (these are preferences found in 
//                /Users/<username>/Library/Preferences) and kAnyUsers which lists LoginItems which are common to all 
//                users on the system (these preferences are found in /Library/Preferences/).
//
// Output Parameters:
// Return -> 	True if number of login items successfully printed. false otherwise.
/***************************************************************************************************/
Boolean PrintNumberOfLoginItems(CFStringRef WhosPreferencesToList);

#ifdef __cplusplus
}
#endif

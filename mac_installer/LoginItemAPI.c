/*
     File:       LoginItemAPI.c
 
     Copyright:  © 2000-2003 by Apple Computer, Inc., all rights reserved.
 
     Bugs?:      For bug reports, consult the following page on
                 the World Wide Web:
 
                     http://developer.apple.com/bugreporter/
 
*/
#import "LoginItemAPI.h"

/***************************************************************************************************/
// AddLoginItemWithPropertiesToUser
//**************************************************************************************************/
// This function will add a LoginItem to the list of LoginItems when called.  The properties
// given to the new LoginItem are passed when calling the function.  Note that *no* check is made
// when adding the LoginItem to ensure that the path points to a valid application.  Note that the LoginItem
// Is always added to the *end* of the list of LoginItems.
//**************************************************************************************************/
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
//**************************************************************************************************/

Boolean AddLoginItemWithPropertiesToUser(CFStringRef WhosPreferencesToChange, const char* AbsolutePath, Boolean HidetheApplicationOnLaunch)
{
    CFStringRef PreferenceName, MainKeyName;
    CFArrayRef	ArrayOfLoginItemsFixed;
    Boolean Success;
    CFMutableArrayRef ArrayOfLoginItemsModifiable;
    CFDictionaryRef EmptyLoginItem;
    CFMutableDictionaryRef NewLoginItem;

    //Generating the Lookup keys.  Lookups are done via keys which are CFStrings.
    CFStringRef HideKey = CFStringCreateWithCString(NULL,"Hide",kCFStringEncodingASCII);
    CFBooleanRef HideValue;
		
    CFStringRef PathKey = CFStringCreateWithCString(NULL,"Path",kCFStringEncodingASCII);
    CFStringRef PathValue;
	
    //Turning the "hide" boolean value passed as kHideOnLaunch, kDoNotHideOnLaunch into the proper type (CFBooleanRef).  
    if (HidetheApplicationOnLaunch == true)
    {
	HideValue = kCFBooleanTrue;
    }
    else
    {
	HideValue = kCFBooleanFalse;
    }

    //doing sanity check on the name of the user passed in.  Note we can use direct compairson because the pointer values and memory values should be identical to the constants.
    if ((WhosPreferencesToChange != kCurrentUser) && (WhosPreferencesToChange != kAllUsers))
    {
        return(false);
    }
    //Converting the Absolute path passed in to the format accepted by the CFPreferences API's (CFString format).  
    //For developers localizing to different languages: You should replace the kCFStringEncodingASCII with kCFStringEncodingUnicode and then what you pass in as the absolute path should be a unicode string.  This will allow you to add LoginItems localized to different languages where the path can contain non-ascii characters.
    PathValue = CFStringCreateWithCString(NULL,AbsolutePath,kCFStringEncodingASCII);
	
    //The name of the preference we are going to edit as a CFString.  In this case it is loginwindow.plist
    PreferenceName = CFSTR(kAppStr); //Note: CFString constants do not need to be released.

    //The name of the main or first key referenced in the preference file.  In this case it is: AutoLaunchedApplicationDictionary
    MainKeyName = CFSTR(kKeyStr); //Note: CFString constants do not need to be released.

    //Getting the array of LoginItems from the LoginItems preference file.  
    //Parameters: 
    //First Parameter: The key used to look up the array information.
    //Second Parameter:Name of the preference Item (loginwindow.plist in this case),
    //Third Parameter: Which user(s) will be effected.  We either want the current user (kCurrentUser) or all users (kAllUsers) 
    //Fourth Parameter:  We set this to anyhost because LoginItems always make use of Anyhost.
    //Note: The return value will be null if the preference currently does not exist.  
    //Note that the structure returned must be released later.
    
    ArrayOfLoginItemsFixed = CFPreferencesCopyValue(MainKeyName, PreferenceName, WhosPreferencesToChange, kCFPreferencesAnyHost);
	
    //If the preference loginwindow.plist doesn't exist (ArrayOfLoginItemsFixed == null) then we will create the preference from scratch.  The first step is to create an empty array.  Afterwards we will insert our LoginItem into the previously empty array of LoginItems.
    if( ArrayOfLoginItemsFixed == NULL)  //handling case where no LoginItems exist yet.
	{
		//There must not be a loginwindow preference.  We will create one by first creating an empty array of LoginItems.  This will serve as our empty list of LoginItems which we will add to later.  
		//Note we must release the returned value.
		ArrayOfLoginItemsFixed = CFArrayCreate(NULL, NULL, 0, NULL);
		
		//If creating the array failed then we give up.  When giving up we always release required data types.
		if (ArrayOfLoginItemsFixed == NULL)
		  {
		     return(false);
		  }
	}

    //We want to get a modifiable version of the LoginItems.  The modifiable version is where we will make all our changes before saving the LoginItems back to disk.
    //First Parameter: using default CFAllocator. 
    //Second Paramater: want a non-bounded array so set this parameter to 0.  This prevents the system from limiting the size of the array we want to use.
    //Third Parameter: copy of the original list of LoginItems (the fixed list from before).
    ArrayOfLoginItemsModifiable = CFArrayCreateMutableCopy(NULL,0,ArrayOfLoginItemsFixed);
	
    //If this operation failed then we release allocated structures and give up
    if (ArrayOfLoginItemsModifiable == NULL)
	{
          CFRelease(ArrayOfLoginItemsFixed);
          return(false);
	}
	
    //Here we are creating an empty LoginItem.  Actually a LoginItem itself is just a CFDictionary
    //Note that the return value must be released later.
    EmptyLoginItem = CFDictionaryCreate(NULL,NULL,NULL, 0,NULL, NULL);
	
    //If this operation failed then we release allocated structures and give up
    if (EmptyLoginItem == NULL)
	{
          CFRelease(ArrayOfLoginItemsFixed);
          CFRelease(ArrayOfLoginItemsModifiable);
          return(false);
	}
	
    //OK here we create an empty modifiable LoginItem from the emptyLoginItem which is fixed.  
    //First Parmater: NULL because we are using current allocator
    //Second Paramater: want a non-bounded array so set this parameter to 0.  This prevents the system from limiting the size of the array we want to use.
    //Third parameter: The empty LoginItem which we are making a copy of.
    //Note that we have to release the new LoginItem at the end.
    NewLoginItem = CFDictionaryCreateMutableCopy(NULL,0, EmptyLoginItem);

    //If this operation failed then we release allocated structures and give up
    if (NewLoginItem == NULL)
	{
          CFRelease(EmptyLoginItem);
          CFRelease(ArrayOfLoginItemsModifiable);
          CFRelease(ArrayOfLoginItemsFixed);
          return(false);
	}

    //This is where we add the new LoginItem to the currently empty list
    //FirstParameter: the LoginItem list we are adding to.
    //Second Parameter: The key/value we are going to set.
    //Third Parameter: The value that the LoginItem will have.
    
    CFDictionaryAddValue(NewLoginItem,HideKey, HideValue); 	
    CFDictionaryAddValue(NewLoginItem,PathKey, PathValue); 	

    //We are appending the new (finished) LoginItem to the list of LoginItems.
    CFArrayAppendValue(ArrayOfLoginItemsModifiable, NewLoginItem);
		
    //Here we write the settings back to the preference stored in memory.
    //Parameters: 
    //First Parameter: The key used to look up the array information.
    //Second Parameter: The LoginItems list to write out to disk.
    //Third Parameter:Name of the preference Item (loginwindow preference in this case),
    //Fourth Parameter: Which user(s) will be effected.  We either want the current user (kCurrentUser) or all users (kAllUsers) 
    //Fifth Parameter:  We set this to anyhost since LoginItems always use anyhost.
    CFPreferencesSetValue(MainKeyName,ArrayOfLoginItemsModifiable,PreferenceName, WhosPreferencesToChange, kCFPreferencesAnyHost);

    //Here we write the preference file in memory back to the hard disk.  
    Success = CFPreferencesSynchronize(PreferenceName, WhosPreferencesToChange, kCFPreferencesAnyHost);

    //If this operation failed then we release allocated structures and give up
    if (Success == false)
	{
	  CFRelease(NewLoginItem);
          CFRelease(EmptyLoginItem);
          CFRelease(ArrayOfLoginItemsModifiable);
          CFRelease(ArrayOfLoginItemsFixed);
          return(false);
	}

    //Releasing all the data structures which need releasing.  
    CFRelease(NewLoginItem);
    CFRelease(EmptyLoginItem);
    CFRelease(ArrayOfLoginItemsModifiable);
    CFRelease(ArrayOfLoginItemsFixed);

    return(true);  //we return true because all operations succeeded which means the LoginItems were successfully updated.
}

/**************************************************************************************************/
// ReturnLoginItemPropertyAtIndex
//**************************************************************************************************/
// This function will return the list of LoginItems as a CFArray.  
//**************************************************************************************************/
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
//**************************************************************************************************/

char ReturnCharacterArray [kLoginItemInfoLength]; //allocating a character array where the return result will be stored.

char* ReturnLoginItemPropertyAtIndex(CFStringRef WhosPreferencesToList, int RequestType, int LoginItemIndex)
{
    CFArrayRef	ArrayOfLoginItemsFixed;
    CFDictionaryRef LoginItemWeAreExaming;
    CFStringRef PreferenceName, MainKeyName;
    CFStringRef LoginPathInfoAsCFString;
    CFStringRef PathKey, HideKey;
    CFBooleanRef HideValue;
	
    char* StringToReturn = NULL;
    char* TempString;
    
    //Doing a sanity check on the request type passed in.  If request type doesn't match something we know we return NULL representing an error
    if ((RequestType != kApplicationNameInfo) && (RequestType != kHideInfo) && (RequestType != kFullPathInfo))
    {
	return(NULL);
    }
    
    //Doing a sanity check on the LoginItem index.  Return error if not valid index.
    if (LoginItemIndex < 0)
    {
	return(NULL);
    }

    //doing sanity check on the name of the user passed in.  Note we can use direct compairson because the pointer values and memory values should be identical to the constants.
    if ((WhosPreferencesToList != kCurrentUser) && (WhosPreferencesToList != kAllUsers))
    {
        return(NULL);
    }

    //Generating the Lookup keys.  Lookups are done via keys which are CFStrings.
    HideKey = CFStringCreateWithCString(NULL,"Hide",kCFStringEncodingASCII);
    //This is the key used when looking up using the kFullPathInfo or kApplicationNameInfo.  
    PathKey = CFStringCreateWithCString(NULL,"Path",kCFStringEncodingASCII);

    //We need the name of the Application to look up in order look up the LoginItem list.  In this the name is loginwindow.plist
    PreferenceName = CFSTR(kAppStr); //Note: CFString constants do not need to be released.

    //The name of the main or first key referenced in the preference file.  In this case it is: AutoLaunchedApplicationDictionary
    MainKeyName = CFSTR(kKeyStr); //Note: CFString constants do not need to be released.

    //Getting the array of LoginItems from the LoginItems preference file.  
    //Parameters: 
    //First Parameter: The key used to look up the array information.
    //Second Parameter:Name of the preference Item (loginwindow.plist in this case),
    //Third Parameter: Which user(s) will be effected.  We either want the current user (kCurrentUser) or all users (kAllUsers) 
    //Fourth Parameter:  We set this to anyhost because LoginItems always make use of Anyhost.
    //Note: The return value will be null if the preference currently does not exist.  
    //Note that the structure returned must be released later.
    
    ArrayOfLoginItemsFixed =  CFPreferencesCopyValue(MainKeyName, PreferenceName, WhosPreferencesToList, kCFPreferencesAnyHost);

    //If the preference loginwindow.plist doesn't exist (ArrayOfLoginItemsFixed == null) then we know the size of the array is zero.  This means that all queries on the database will result in returning NULL because there is no elements to look up.  Thus we return null at this point.  Note that the user of this function should normally call LoginItemsCount first to know how many items there are before asking about info on indiviual items.
        if( ArrayOfLoginItemsFixed == NULL)  //handling case where no LoginItems exist and can't do any lookups.
	{
	    return(NULL);  //returning null since there are no items to return information on 
	}

     //since we got back a vaild array of LoginItems now we will query the specific index of the LoginItem array that we are interested in.
     //We do the lookup using a CFArrayGetValueAtIndex call.
     //First Parameter: The array to query.
     //Second Parameter: The index we want in this case whatever the user of this function passed in.  We have to convert to a CFIndex which in reality is just a signed integer.
    LoginItemWeAreExaming = (CFDictionaryRef) CFArrayGetValueAtIndex(ArrayOfLoginItemsFixed, (CFIndex) LoginItemIndex);
 
    if (LoginItemWeAreExaming == NULL) //if dictionary returned is null (invalid) then we give up.
    {
	return(NULL);  //returning null since the value returned is not valid. 
    }
 
    //the request was for the path info in the full path case or the application name case the code is pretty much the same except we pull out the path at the end for the application name.
    if ((RequestType == kFullPathInfo) || (RequestType == kApplicationNameInfo))    {
	//Here we need to determine if the key we are looking for is present.  If it isn't then the LoginItem is either corrupted or in a format we don't understand.  In either case if the key we are looking for isn't present then we give up.
	//We determine validity using CFDictionaryContainsKey to see if the key we are looking for is there
	//First parameter: The CFdictionary to search in this case the LoginItem we are looking at (LoginItems are actually stored as CFDictionaries
	//Second Paramter: the key we are looking for.
	if (CFDictionaryContainsKey(LoginItemWeAreExaming,PathKey)  == false) 	
	{
	    CFRelease(ArrayOfLoginItemsFixed);
	    return(NULL);  //returning null since LoginItems are corrupted or in a non-reconized format
	}
	
	//Here we get a copy of the path as a CFString.  we use a combination of calls of CFStringCreateCopy and CFDictionaryGetValue.  The CFString call just makes a copy of the return value (like strcpy does) which later needs to be released.  The CFDictionaryGetValue call will return the CFString reprenting the path.
	LoginPathInfoAsCFString = CFStringCreateCopy(NULL, CFDictionaryGetValue(LoginItemWeAreExaming,PathKey));
	
	//now that we have the path as a CFString we will convert to a standard C string for returning later.  We use CFStringGetCStringPtr to do this
	//First Parameter: CFString to be converted to C String
	//Second Parameter: A pointer to the buffer which is where the C string will be placed.
	//Third Parameter: The length of the buffer that we are putting the C string into.  If the CFString is too long for the buffer then the call returns an error (NULL)
	//Fourth Paramter: Encoding Type. We use encoding type of ASCII for simplicity to developers.
	//**Note to engineers localizing this API to a non-english format: The encoding used below kCFStringEncodingASCII is simplistic and should be replaced with kCFStringEncodingUnicode.  The return result will be in unicode format of the path and will handle the case where you want to lookup paths that have non ascii characters in the path.  The reason I don't use unicode by default is for simplicity of use to the majority of developers.  
	CFStringGetCString(LoginPathInfoAsCFString, (char*) (&ReturnCharacterArray), kLoginItemInfoLength, kCFStringEncodingASCII);		    
	
	//Now if the application name is request we will strip off the beginning of the path leaving only the application or alias name.  Note that this assumes the string it is parsing is a ASCII string.  How this works is with multiple calls to strtok.  strtok returns a pointer to the string *after* the next "/" character in the string.  This continues until strtok returns NULL which means that there are no more "/" in the string (we are at the end of the string).  This means that what is left is the filename and this is what we want to return (example: "Clock.app" from "/Applications/Clock.app").
	if (RequestType == kApplicationNameInfo)
	{
	    TempString = (char*)strtok(ReturnCharacterArray, "/");

	    while (TempString != NULL)
	    {
		StringToReturn = TempString;
		TempString = (char*) strtok(NULL, "/"); //successive calls to strtok with the first argument as NULL continues the search for "/" from where we left off.
	    }//endwhile	
	}//endif
	else
	{  //if we only want to return the entire path then the string we want to return is the entire path which is just a pointer to the character array.
	  StringToReturn = (char*) ReturnCharacterArray;
	}//end else
	
	CFRelease(LoginPathInfoAsCFString); //need to release this here since it was allocated.  
    }//end get path info section

    else  //now at the begining of the code which is executed if the Hide info is requested.
    {
	if (CFDictionaryContainsKey(LoginItemWeAreExaming,HideKey)  == false) 	
	{
	    CFRelease(ArrayOfLoginItemsFixed);
	    return(NULL);  //returning null since LoginItems are corrupted or in a non-reconized format
	}
	
	//Here we get a copy of the HideKey as a CFBooleanRef (a CF boolean).  We get the info with a call to CFDictionaryGetValue.
	//First Argument: Dictionary to pull the boolean value from (Note that LoginItems are stored as CFDictionaries)
	//Second Argument: The key to look up in this case the "hide" key.
	HideValue = CFDictionaryGetValue(LoginItemWeAreExaming,HideKey);

	if (HideValue == kCFBooleanTrue)
	{  
	    //we set the returnCharacterArray equal to "true" this will be the return value
	    strcpy(ReturnCharacterArray, "true");
	    StringToReturn = (char*) ReturnCharacterArray;
	}
	else if (HideValue == kCFBooleanFalse)
	{
	    //we set the returnCharacterArray equal to "false" this will be the return value
	    strcpy(ReturnCharacterArray, "false");
	    StringToReturn = (char*) ReturnCharacterArray;
	}
	else //here we have an invalid HideValue.  Thus we give up and return error condition (NULL).
	{
	    CFRelease(ArrayOfLoginItemsFixed);
	    return(NULL);  //returning null since LoginItems are corrupted or in a non-reconized format
	}

    } //end kHideInfo section of the routine.
   
    CFRelease(ArrayOfLoginItemsFixed);
    return(StringToReturn);
}

/***************************************************************************************************/
// GetCountOfLoginItems
//**************************************************************************************************/
// This function will return the number of LoginItems for the user requested.    
//**************************************************************************************************/
// Input Parameters:
// First Parameter (WhosPreferencesToList) -> 	A constant representing which users preferences 
//                you want the number of LoginItems for.  The two alternatives are: kCurrentUser which lists the 
//                LoginItems which are specific to that user (these are preferences found in 
//                /Users/<username>/Library/Preferences) and kAnyUsers which lists LoginItems which are common to all 
//                users on the system (these preferences are found in /Library/Preferences/).
//
// Output Parameters:
// Return -> 	This function returns the number of LoginItems for the user requested as a signed integer. 
//**************************************************************************************************/

int GetCountOfLoginItems(CFStringRef WhosPreferencesToList)
{
    CFArrayRef	ArrayOfLoginItemsFixed;
    CFStringRef PreferenceName, MainKeyName;
    int ValueToReturn = -1;


    //doing sanity check on the name of the user passed in.  Note we can use direct compairson because the pointer values and memory values should be identical to the constants.
    if ((WhosPreferencesToList != kCurrentUser) && (WhosPreferencesToList != kAllUsers))
    {
        return(0);
    }

    //We need the name of the Application to look up in order look up the LoginItem list.  In this the name is loginwindow.plist
    PreferenceName = CFSTR(kAppStr); //Note: CFString constants do not need to be released.

    //The name of the main or first key referenced in the preference file.  In this case it is: AutoLaunchedApplicationDictionary
    MainKeyName = CFSTR(kKeyStr); //Note: CFString constants do not need to be released.

    //Getting the array of LoginItems from the LoginItems preference file.  
    //Parameters: 
    //First Parameter: The key used to look up the array information.
    //Second Parameter:Name of the preference Item (loginwindow.plist in this case),
    //Third Parameter: Which user(s) will be effected.  We either want the current user (kCurrentUser) or all users (kAllUsers) 
    //Fourth Parameter:  We set this to anyhost because LoginItems always make use of Anyhost.
    //Note: The return value will be null if the preference currently does not exist.  
    //Note that the structure returned must be released later.
    
    ArrayOfLoginItemsFixed =  CFPreferencesCopyValue(MainKeyName, PreferenceName, WhosPreferencesToList, kCFPreferencesAnyHost);
    
    //If the preference loginwindow.plist doesn't exist (ArrayOfLoginItemsFixed == null) then we know the size of the array is zero.  
    if( ArrayOfLoginItemsFixed == NULL)  //handling case where no LoginItems exist and can't do any lookups.
	{
	    return(0); //number of LoginItems is zero.   
	}
	
    ValueToReturn = (int) CFArrayGetCount(ArrayOfLoginItemsFixed);

    CFRelease(ArrayOfLoginItemsFixed);
    return(ValueToReturn);    
}

/***************************************************************************************************/
// RemoveLoginItemAtIndex
//**************************************************************************************************/
// This function will remove the LoginItem at the given index for the user specified.    
//**************************************************************************************************/
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
//**************************************************************************************************/

Boolean RemoveLoginItemAtIndex(CFStringRef WhosPreferencesToChange, int LoginItemIndex)
{
    CFArrayRef	ArrayOfLoginItemsFixed;
    CFStringRef PreferenceName, MainKeyName;
    CFMutableArrayRef ArrayOfLoginItemsModifiable;
    Boolean Success;

    //Doing a sanity check on the LoginItem index.  The LoginItem index can't be smaller than zero and can't be larger than the length of the array (Number of LoginItems - 1).
    if ((LoginItemIndex < 0) || (LoginItemIndex > (GetCountOfLoginItems(WhosPreferencesToChange) - 1) ))
    {
	return(false);
    }

    //doing sanity check on the name of the user passed in.  Note we can use direct compairson because the pointer values and memory values should be identical to the constants.
    if ((WhosPreferencesToChange != kCurrentUser) && (WhosPreferencesToChange != kAllUsers))
    {
        return(false);
    }

    //We need the name of the Application to look up in order look up the LoginItem list.  In this the name is loginwindow.plist
    PreferenceName = CFSTR(kAppStr); //Note: CFString constants do not need to be released.

    //The name of the main or first key referenced in the preference file.  In this case it is: AutoLaunchedApplicationDictionary
    MainKeyName = CFSTR(kKeyStr); //Note: CFString constants do not need to be released.

    //Getting the array of LoginItems from the LoginItems preference file.  We will then remove the LoginItem in question from the CFArray and write the preferences back to disk.
    //Parameters: 
    //First Parameter: The key used to look up the array information.
    //Second Parameter:Name of the preference Item (loginwindow.plist in this case),
    //Third Parameter: Which user(s) will be effected.  We either want the current user (kCurrentUser) or all users (kAllUsers) 
    //Fourth Parameter:  We set this to anyhost because LoginItems always make use of Anyhost.
    //Note: The return value will be null if the preference currently does not exist.  
    //Note that the structure returned must be released later.
    
    ArrayOfLoginItemsFixed =  CFPreferencesCopyValue(MainKeyName, PreferenceName, WhosPreferencesToChange, kCFPreferencesAnyHost);

    //If the preference loginwindow.plist doesn't exist (ArrayOfLoginItemsFixed == null) then we know the size of the array is zero.  This means that no LoginItems can be removed.  In this case we fail and give up.
    if( ArrayOfLoginItemsFixed == NULL)  //handling case where no LoginItems exist and can't do any lookups.
	{
	    return(false);  //returning null since there are no items to return information on 
	}

    //We want to get a modifiable version of the LoginItems.  The modifiable version is where we will make all our changes before saving the LoginItems back to disk.
    //First Parameter: using default CFAllocator. 
    //Second Paramater: want a non-bounded array so set this parameter to 0.  This prevents the system from limiting the size of the array we want to use.
    //Third Parameter: copy of the original list of LoginItems (the fixed list from before).
    ArrayOfLoginItemsModifiable = CFArrayCreateMutableCopy(NULL,0,ArrayOfLoginItemsFixed);
	
    //If this operation failed then we release allocated structures and give up
    if (ArrayOfLoginItemsModifiable == NULL)
	{
          CFRelease(ArrayOfLoginItemsFixed);
          return(false);
	}
	
    //We are removing the selected LoginItem to the list of LoginItems.
    //First parameter: CFArray (our LoginItem array) which we want to remove a value.
    //Second paramter: the index of the LoginItem we want to remove.  Note that we convert the integer to a CFindex which is really just a signed integer.
    CFArrayRemoveValueAtIndex(ArrayOfLoginItemsModifiable, (CFIndex) LoginItemIndex);    
    		
    //Here we write the settings back to the preference stored in memory.
    //Parameters: 
    //First Parameter: The key used to look up the array information.
    //Second Parameter: The LoginItems list to write out to disk.
    //Third Parameter:Name of the preference Item (loginwindow preference in this case),
    //Fourth Parameter: Which user(s) will be effected.  We either want the current user (kCurrentUser) or all users (kAllUsers) 
    //Fifth Parameter:  We set this to anyhost since LoginItems always use anyhost.
    CFPreferencesSetValue(MainKeyName,ArrayOfLoginItemsModifiable,PreferenceName, WhosPreferencesToChange, kCFPreferencesAnyHost);

    //Here we write the preference file in memory back to the hard disk.  
    Success = CFPreferencesSynchronize(PreferenceName, WhosPreferencesToChange, kCFPreferencesAnyHost);

    //If this operation failed then we release allocated structures and give up
    if (Success == false)
	{
          CFRelease(ArrayOfLoginItemsModifiable);
          CFRelease(ArrayOfLoginItemsFixed);
          return(false);
	}

    //Releasing all the data structures which need releasing.  
    CFRelease(ArrayOfLoginItemsModifiable);
    CFRelease(ArrayOfLoginItemsFixed);

    return(true);  //we return true because all operations succeeded which means the LoginItems were successfully updated.
    
}

/***************************************************************************************************/
// PrintOutLoginItemAtIndex
//**************************************************************************************************/
// This function will print out the LoginItem at the specified index 
//**************************************************************************************************/
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
//**************************************************************************************************/

Boolean PrintOutLoginItemAtIndex(CFStringRef WhosPreferencesToList, int LoginItemIndex)
{
    
    char* StringToPrint;
    
    //doing sanity check on the name of the user passed in.  Note we can use direct compairson because the pointer values and memory values should be identical to the constants.
    if ((WhosPreferencesToList != kCurrentUser) && (WhosPreferencesToList != kAllUsers))
    {
        return(false);
    }
    
    //doing sanity check on index passed in.  Return false if index isn't valid.
    if ((LoginItemIndex < 0) || (LoginItemIndex > GetCountOfLoginItems(WhosPreferencesToList)))
    {
	return(false);
    }
    
    StringToPrint = ReturnLoginItemPropertyAtIndex(WhosPreferencesToList, kFullPathInfo, LoginItemIndex);

    if (StringToPrint == NULL)
    {
	return(false);
    }
    
    printf("\tFull path to App: %s\n", StringToPrint);

    StringToPrint = ReturnLoginItemPropertyAtIndex(WhosPreferencesToList, kHideInfo, LoginItemIndex);

    if (StringToPrint == NULL)
    {
	return(false);
    }
    
    printf("\tHide Info:        %s\n", StringToPrint);

    StringToPrint = ReturnLoginItemPropertyAtIndex(WhosPreferencesToList, kApplicationNameInfo, LoginItemIndex);

    if (StringToPrint == NULL)
    {
	return(false);
    }
    
    printf("\tExecutable Name:  %s\n", StringToPrint);
    
    return(true);
}

/***************************************************************************************************/
// PrintOutAllLoginItems
//**************************************************************************************************/
// This function will print out all the LoginItems for the specified user 
//**************************************************************************************************/
// Input Parameters:
// First Parameter (WhosPreferencesToList) -> 	A constant representing which users preferences 
//                you want to print the LoginItems from.  The two alternatives are: kCurrentUser which removes login
//                items for the which are specific to that user (these are preferences found in 
//                /Users/<username>/Library/Preferences) and kAnyUsers which lists LoginItems which are common to all 
//                users on the system (these preferences are found in /Library/Preferences/).
//
// Output Parameters:
// Return -> 	This function a boolean true if the LoginItem was successfully printed.  
//**************************************************************************************************/

Boolean PrintOutAllLoginItems(CFStringRef WhosPreferencesToList)
{
    int NumberOfLoginItems, Counter;
    Boolean Success;
    
    //doing sanity check on the name of the user passed in.  Note we can use direct compairson because the pointer values and memory values should be identical to the constants.
    if ((WhosPreferencesToList != kCurrentUser) && (WhosPreferencesToList != kAllUsers))
    {
        return(false);
    }

    NumberOfLoginItems = GetCountOfLoginItems(WhosPreferencesToList);
    
    if (NumberOfLoginItems == 0)
    {
	printf("LoginItems are Empty (i.e. there are no LoginItems)\n");
    }
    
    for (Counter = 0 ; Counter < NumberOfLoginItems ; Counter++)
    {
	printf("LoginItem Index %d:\n", Counter);
	Success = PrintOutLoginItemAtIndex(WhosPreferencesToList, Counter);
	
	if (Success == false) //if couldn't print the specified index then we give up.
	{return(false);}
    }
    
    return(true);
}

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
//**************************************************************************************************/
Boolean PrintNumberOfLoginItems(CFStringRef WhosPreferencesToList)
{

        //doing sanity check on the name of the user passed in.  Note we can use direct compairson because the pointer values and memory values should be identical to the constants.
    if ((WhosPreferencesToList != kCurrentUser) && (WhosPreferencesToList != kAllUsers))
    {
        return(false);
    }

    printf("The number of LoginItems is: %d", GetCountOfLoginItems(WhosPreferencesToList));
    return(true);
}



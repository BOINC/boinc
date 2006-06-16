<?php

/**
 * This class represents a user
 **/

// A list of what kind of special users exist
define('S_MODERATOR', 0); $special_user_bitfield[S_MODERATOR]="Forum moderator";
define('S_ADMIN', 1); $special_user_bitfield[S_ADMIN]="Project administrator";
define('S_DEV', 2); $special_user_bitfield[S_DEV]="Project developer";
define('S_TESTER', 3); $special_user_bitfield[S_TESTER]="Project tester";
define('S_VOLUNTEER', 4); $special_user_bitfield[S_VOLUNTEER]="Volunteer developer";
define('S_VOLUNTEER_TESTER', 5); $special_user_bitfield[S_VOLUNTEER_TESTER]="Volunteer tester";
define('S_SCIENTIST', 6); $special_user_bitfield[S_SCIENTIST]="Project scientist";

class User {
    // Container for the db variables for this user
    var $dbObj;
    // Container for the forum db variables for this user
    var $fprefObj;
    var $transform_options;
    var $prefs_loaded;
    var $dbhandler;

    /**
     * Constructor (don't use this, a user is a singleton)
     * instead use the user creator function: getUser($id)
     **/
    function User($id, $optional_dbobj="", $optional_prefobj=""){
        global $mainFactory;
        $this->dbhandler = $mainFactory->getDatabaseHandler();
        if ($optional_dbobj){
            $this->dbObj = $optional_dbobj;
        } else {
            $this->dbObj = $this->dbhandler->getUser($id);
        }
        if ($optional_prefobj) {$this->fprefObj = $optional_prefobj; $this->prefs_loaded=true;};
        if (!$this->dbObj) error_page("User with id $id created but nothing returned from DB layer");
    }
    
    /**
     * The following functions return various fields for the user or update them.
     **/    
    function getID(){
        return $this->dbObj->id;
    }
    function getCreateTime(){
        return $this->dbObj->create_time;
    }
    function getName(){
        return $this->dbObj->name;
    }
    function getEmail(){
        return $this->dbObj->email_addr;
    }
    function getTotalCredit(){
        return $this->dbObj->total_credit;
    }
    function getExpavgCredit(){
        return $this->dbObj->expavg_credit;
    }
    function hasProfile(){
        return $this->dbObj->has_profile;
    }
    function hasDonated(){
        return $this->dbObj->donated;
    }
    function getAuthenticator(){
        return $this->dbObj->authenticator;
    }
    function getForumSortStyle(){
        $this->ensurePrefsLoaded();
        return $this->fprefObj->forum_sorting;
    }
    function setForumSortStyle($style){
        $this->fprefObj->forum_sorting = $style;
        return $this->updatePrefs("forum_sorting", intval($style));
    }
    function getThreadSortStyle(){
        $this->ensurePrefsLoaded();
        return $this->fprefObj->thread_sorting;
    }
    function setThreadSortStyle($style){
        $this->fprefObj->forum_sorting = $style;
        return $this->updatePrefs("thread_sorting", intval($style));
    }
    function getHighRatingThreshold(){
        $this->ensurePrefsLoaded();
        return $this->fprefObj->high_rating_threshold;
    }
    function setHighRatingThreshold($value){
        $this->updatePrefs("high_rating_threshold", intval($value));
    }

    function getLowRatingThreshold(){
        $this->ensurePrefsLoaded();
        return $this->fprefObj->low_rating_threshold;
    }
    function setLowRatingThreshold($value){
        $this->updatePrefs("low_rating_threshold", intval($value));
    }

    function getMinimumWrapPostcount(){
        $this->ensurePrefsLoaded();
        return $this->fprefObj->minimum_wrap_postcount;
    }
    function setMinimumWrapPostCount($value){
        $value = intval($value);
        if ($value<0) $value=0;
        $this->updatePrefs("minimum_wrap_postcount", $value);
    }
    function getDisplayWrapPostcount(){
        $this->ensurePrefsLoaded();
        return $this->fprefObj->display_wrap_postcount;
    }
    function setDisplayWrapPostCount($value){
        $value = intval($value);
        if ($value<0) $value=0;
        $this->updatePrefs("display_wrap_postcount", $value);
    }
    function getPostcount(){
        $this->ensurePrefsLoaded();
        return $this->fprefObj->posts;
    }
    function setPostcount($value){
        return $this->updatePrefs("posts", $value);
    }
    function getIgnoreList(){
        $this->ensurePrefsLoaded();
        $list = explode("|",$this->fprefObj->ignorelist);
        return $list;
    }
    function addIgnoredUser($user){
        $this->ensurePrefsLoaded();
        $list = $this->getIgnoreList();
        // See if the user exists:
        foreach ($list as $key => $userid){
            if ($userid==$user->getID()){
                $already_present=true;
            }
        }
        if (!$already_present){
            $list[]=$user->getID();
            $this->updatePrefs("ignorelist",implode("|",array_values($list)));
        } else {
            return true;
        }
    }
    function removeIgnoredUser($user){
        $this->ensurePrefsLoaded();
        $list = $this->getIgnoreList();
        // See if the user exists:
        foreach ($list as $key => $userid){
            if ($userid==$user->getID()){
                unset($list[$key]);
            }
        }
        return $this->updatePrefs("ignorelist",implode("|",array_values($list)));
    }
    function getSignature(){
        $this->ensurePrefsLoaded();
        return $this->fprefObj->signature;
    }
    function setSignature($signature){
        return $this->updatePrefs("signature", $signature);
    }
    function getAvatar(){
        $this->ensurePrefsLoaded();
        return $this->fprefObj->avatar;
    }    
    
    /**
     * Sets the avatar to the provided local url relative to the
     * user directory
     **/
    function setAvatar($url){
        $this->updatePrefs("avatar", $url);
    }
    
    /**
     * Gets user's display options and creates an output_options object
     * based upon their options.  (see text_transformations.php)
     **/
    function getTextTransformSettings(){
        $this->ensurePrefsLoaded();
        if (!isset($this->transform_options)) {
            $this->transform_options = new output_options; // Give defaults
        }
        if ($this->hasImagesAsLinks()){
            $this->transform_options->images_as_links = 1;
        }
        if ($this->hasLinkPopup()){
            $this->transform_options->link_popup = 1;
        }
        return $this->transform_options;
    }
    
    /**
     * More standard field updates or retrieval functions
     */
    function hasIgnoreStickyPosts(){
        $this->ensurePrefsLoaded();
        return $this->fprefObj->ignore_sticky_posts;
    }
    function setIgnoreStickyPosts($bool){
        $this->updatePrefs("ignore_sticky_posts", $bool?true:false);
    }
    function hasJumpToUnread(){
        $this->ensurePrefsLoaded();
        return $this->fprefObj->jump_to_unread;
    }
    function setJumpToUnread($bool){
        $this->updatePrefs("jump_to_unread", $bool?true:false);
    }
    function hasImagesAsLinks(){
        $this->ensurePrefsLoaded();
        return $this->fprefObj->images_as_links;
    }
    function setImagesAsLinks($bool){
        $this->updatePrefs("images_as_links", $bool?true:false);
    }
    function hasLinkPopup(){
        $this->ensurePrefsLoaded();
        return $this->fprefObj->link_popup;
    }
    function setLinkPopup($bool){
        $this->updatePrefs("link_popup", $bool?true:false);
    }
    function hasHideSignatures(){
        $this->ensurePrefsLoaded();
        return $this->fprefObj->hide_signatures;
    }
    function setHideSignatures($bool){
        $this->updatePrefs("hide_signatures", $bool?true:false);
    }
    function hasHideAvatars(){
        $this->ensurePrefsLoaded();
        return $this->fprefObj->hide_avatars;
    }
    function setHideAvatars($bool){
        $this->updatePrefs("hide_avatars", $bool?true:false);
    }
    function hasSignatureByDefault(){
        $this->ensurePrefsLoaded();
        return !$this->fprefObj->no_signature_by_default;
    }
    function setSignatureByDefault($bool){
        // Note the inverted use of false/true - the db stores NO signatures by default... for some reason.
        // No need to make it more confusing than neccesary so we present an API without negations in the name.
        $this->updatePrefs("no_signature_by_default", $bool?false:true);
    }
    function hasAvatar(){
        return ($this->getAvatar()!="");
    }
    function getLastPostTimestamp(){
        $this->ensurePrefsLoaded();
        return $this->fprefObj->last_post;
    }
    function getMarkForumReadTimestamp(){
        //STUB: potential future feature - to mark all forums as read
        return 0;
    }
    
    /**
     * Return wether this user is a given special user type
     **/
    function isSpecialUser($type){
        $this->ensurePrefsLoaded();
        return (substr($this->fprefObj->special_user, $type,1)!=false);
    }
    
    function getSpecialUser(){
        $this->ensurePrefsLoaded();
	return $this->fprefObj->special_user;
    }
    function setSpecialUser($bitfield){
	return $this->updatePrefs("special_user", $bitfield);
    }
    
    /** 
     * Increment the number of posts for this user
     * as well as set the last post timestamp
     **/
    function incPostCount(){
        $this->updatePrefs("posts",$this->getPostCount()+1);
        $this->updatePrefs("last_post",time());
    }
    
    /**
     * This function ensures that the preferences for this user
     * has been loaded into the class. If it hasn't it will be
     * loaded as part of this call.
     **/
    function ensurePrefsLoaded(){
        if (!$this->prefs_loaded){
            $this->fprefObj = $this->dbhandler->getUserPrefs($this);
            if (!$this->fprefObj){ // No prefs found
                $this->resetPrefs();
            }
            $this->prefs_loaded=true;
        }
    }

    /**
     * Reset the user's preferences to the defaults
     **/
    function resetPrefs(){
        $this->dbhandler->createUserPrefs($this);
        $this->updatePrefs("thread_sorting",6);
    }
    
    /**
     * This function is factored out code for each of the mutators.
     * It accesses the mainFactory and asks the default databaseHandler to
     * update the specified user $pref for $this user with the given $value.
     * It returns true on success. If it fails it usually returns false or 
     * doesnt return at all (depending on the DB layer).
     **/
    function updatePrefs($pref, $value){
        return $this->dbhandler->updateUserPrefs($this, $pref, $value);
    }
}

// ----------- Class ends here -------------------------

/**
 * Get the logged in user again?
 **/
function re_get_logged_in_user($must_be_logged_in=true) {
    $authenticator = init_session();
    if (!$authenticator) {
        if (isset($_COOKIE['auth'])) $authenticator = $_COOKIE['auth'];
    }
    $authenticator = process_user_text($authenticator);
    $user = get_user_from_auth($authenticator);
    if ($must_be_logged_in) {
        require_login($user);
    }
    if ($user){
        return newUser($user->id);
    } else {
        return false;
    }
}

/**
 * To speed up queries a user is considered unique. ie two
 * instances of the same user cannot exist at the same time
 * unless the ID is different.
 * This function returns the unique user object or creates
 * it if it didn't exist. This is a PHP4 workaround for the
 * singleton array pattern.
 **/
function newUser($id, $optional_dbobj="", $optional_prefobj=""){
    // When we progress to PHP5 this can be rewritten into a static class variable
    // and a constructor that does exactly the same - this way calls to newUser
    // are simply replaced with calls to new User()
    global $singleton_users;
    if (!$singleton_users[$id]){
        $singleton_users[$id] = & new User($id, $optional_dbobj, $optional_prefobj);
    }
    return $singleton_users[$id];
}

/**
 * Construct links to the given user
 **/
function re_user_links($user) {
    $x = '<a href="show_user.php?userid='.$user->getID().'">'.$user->getName().'</a>';
    if ($user->hasProfile()) {
        $x .= ' <a href="view_profile.php?userid='.$user->getID().'"><img border="0" src="head_20.png"></a>';
    }
    # Does this project accept donations?
    # If so, do you want to have a link next to user name as it appears on the web site?
    if ($user->hasDonated()) {
        require_once("../project/donations.inc");
        $x .= DONATION_LINK;
    }
    return $x;
}

?>
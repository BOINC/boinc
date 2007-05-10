<?php

require_once('../inc/profile.inc');

define('UOTD_THRESHOLD', 7);
    // email sysadmin if # of UOTD candidates falls below this

function uotd_thumbnail($profile, $user) {
    if ($profile->has_picture) {
        $ub = URL_BASE;
        return "<a href=$ub/view_profile.php?userid=$user->id><img border=0 vspace=4 hspace=8 align=left src=$ub".IMAGE_URL.$user->id."_sm.jpg></a>";
    } else {
        return "";
    }
}

// show UOTD in a small box
//
function show_uotd() {
    $profile = get_current_uotd();
    if ($profile) {
        $user = lookup_user_id($profile->userid);
        echo "<br><table cellpadding=4 cellspacing=0><tr><td>";
        echo uotd_thumbnail($profile, $user);
        echo "</td><td>";
        echo user_links($user)."<br>";
        echo sub_sentence(output_transform(no_pres(strip_tags($profile->response1))), ' ', 150, true);
        echo "</td></tr></table><br>\n";
    }
}

// return the last UOTD profile, or null
//
function get_current_uotd() {
    $result = mysql_query("SELECT * FROM profile ORDER BY uotd_time DESC LIMIT 1");
    $p = null;
    if (mysql_num_rows($result) > 0) {
        $p = mysql_fetch_object($result);
    }
    mysql_free_result($result);
    return $p;
}

// Select a (possibly new) UOTD
//
function select_uotd() {
    echo date("F d Y", time()).": Starting\n";
    $current_uotd = get_current_uotd();
    if ($current_uotd) {
        $assigned = getdate($current_uotd->uotd_time);
        $now = getdate(time());
        if ($assigned['mday'] == $now['mday']) {
            $user = lookup_user_id($current_uotd->userid);
            echo "Already have UOTD for today\n";
            generate_uotd_gadget($current_uotd, $user);
            exit();
        }
    }

    // get a list of profiles that have been 'approved' for UOTD,
    // using a project-specific query if supplied in project.inc
    //
    if (function_exists('uotd_candidates_query')) {
        $query = uotd_candidates_query();
    } else {
        $query = default_uotd_candidates_query();
    }
    $result = mysql_query($query);

    // If the number of approved profiles dips below a threshold,
    // email the sys admin every time we pick a new one.
    //
    if ($result && mysql_num_rows($result) < UOTD_THRESHOLD) {
        mail(UOTD_ADMIN_EMAIL,
            PROJECT . ": User of the Day pool is running low!",
            "The pool of approved candidates for User of the Day has".
            " reached your assigned threshold: there are now only " . mysql_num_rows($result) . " approved users.\n\n".
            "To approve more candidates for User of the Day,".
            " go to the " . PROJECT . " administration page and click \"Unrated profile\""
        );
    }

    if ($result && mysql_num_rows($result) == 0) {
        // If all verified profiles have been selected as UOTD,
        // reshow the one that was shown least recently.
        //
        $result = mysql_query("SELECT * FROM profile,user WHERE profile.userid=user.id AND verification=1 ORDER BY uotd_time ASC LIMIT 1");
    }

    if (!$result || mysql_num_rows($result) == 0) {
        // No valid users of the day - do something.
        echo "No screened users found\n";
        exit();
    }
    $profile = mysql_fetch_object($result);
    $user = lookup_user_id($profile->userid);

    $sql = "UPDATE profile SET uotd_time = ".time()." WHERE userid=$user->id";
    mysql_query($sql);

    mail($user->email_addr,
        "You're the " . PROJECT . " user of the day!",
        "Congratulations!\n\nYou've been chosen as the "
        . PROJECT . " user of the day!
        Your profile will be featured on the " . PROJECT . " website for the next 24 hours."
    );
    echo "Chose user $user->id as UOTD\n";
    $profile->uotd_time = time();
    generate_uotd_gadget($profile, $user);
}

// This is the default policy for choosing the UOTD on any BOINC project.
// To override this with your own policy, create a similar function in
// your own project.inc called uotd_candidates_query()
function default_uotd_candidates_query(){
    if (profile_screening()) {
        $query = "SELECT * FROM profile,user WHERE profile.userid=user.id ";
        $query .= " AND verification=1 ";
        $query .= " AND expavg_credit>1 ";
        $query .= " AND uotd_time IS NULL ";
        $query .= "ORDER BY RAND()";
    } else {
        $query = "SELECT * FROM profile,user WHERE profile.userid=user.id ";
        $query .= " AND verification=1 ";
        $query .= " AND uotd_time IS NULL ";
        $query .= "ORDER BY RAND()";
    }
    return $query;
}

// get a list of profiles that have been 'approved' for UOTD,
// using a project-specific query if supplied in project.inc
//
function count_uotd_candidates(){
    $n = -1;                    // negative value returned on error
    if (function_exists('uotd_candidates_query')) {
        $query = uotd_candidates_query();
    } else {
        $query = default_uotd_candidates_query();
    }
                                                
    $result = mysql_query($query);
    if($result) {
        $n = mysql_num_rows($result);
    }
    mysql_free_result($result);
                                                                        
    return $n;
}

// iGoogle gadget  - generate the gadget content page
//
function generate_uotd_gadget($profile, $user) {
    $x = "<font size='2'>\n";
    $gadget =  PROFILE_PATH."uotd_gadget.html";
    if( $h = fopen($gadget, "w") ){
        $age = time()-$profile->uotd_time;
        echo "age: $age";
        if($age <= 86400+3600) { // allow for slop
            $x .= uotd_thumbnail($profile, $user);
            $x .= user_links($user);
            $x .= "&nbsp;&nbsp;".
                sub_sentence(strip_tags(output_transform($profile->response1)), ' ',
                             250, true);
        }
        else {
            $x .= "<font color='fuscia'>
                There is no User of the Day today.
                Only volunteers who have created a Profile
                (with a picture), and have recent credit,
                are eligible to be chosen as User of the Day.
                We have run out of these, so there isn't a
                User of the Day.
                </font>";
        }
        $x .= "\n</font>\n";
        fwrite($h, $x);
        fclose($h);
    }
}

?>

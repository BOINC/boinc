<?php

// This is a sample project configuration file.
// It contains all constants that define your project setup.
// Modify it to suit your project.
// Put your version in html/project/project_config.inc
// See http://boinc.berkeley.edu/trac/wiki/WebConfig for details.
// Add tra() around visible strings to make them translatable
// See inc/translation.inc and http://boinc.berkeley.edu/trac/wiki/TranslateProject for details

define("PROJECT", "REPLACE WITH PROJECT NAME");
// define("PROJECT_ROOT", "REPLACE WITH ABSOLUTE PROJECT DIRECTORY PATH"); // optional
define("IMAGE_PATH", "../user_profile/images/");
define("IMAGE_URL", "user_profile/images/");
define("PROFILE_PATH", "../user_profile/");
define("PROFILE_URL", "user_profile/");
define("LANGUAGE_FILE", "languages.txt");
define("STYLESHEET", "white.css");
//define("STYLESHEET2", "xxx.css");
define("FORUM_QA_MERGED_MODE", false); // Set to true to merge Message boards and Q&A section
define("COPYRIGHT_HOLDER", "REPLACE WITH COPYRIGHT HOLDER");
define("SYS_ADMIN_EMAIL", "admin@$master_url");
define("UOTD_ADMIN_EMAIL", "admin@$master_url");
    // who gets user of the day pool running low e-mails?
//define("MEMCACHE_SERVERS", "127.0.0.1:11211");

// Email addresses separated by pipe ( | ) that will receive user reported 
// offensive forum posts.
define("POST_REPORT_EMAILS", "moderator1@$master_url|moderator2@$master_url");

?>

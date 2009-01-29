<?php

// stuff that's particular to the "BOINC user survey"

// miscellaneous field names

$run_boinc="run_boinc";
$factor_other_text = "factor_other_text";
$country = "country";
$lother_text = "lother_text";
$fother_text = "fother_text";
$wother_text = "wother_text";
$nother_text = "nother_text";
$cother_text = "cother_text";
$vother_text = "vother_text";
$improved = "improved_text";

$choice0 = array(
    "text" => tra("Yes - I have been running BOINC on my computer for..."),
    "rname" => "current",
    "options" => array(
        "tweek" => tra("less than a week"),
        "tmonth" => tra("less than a month"),
        "tyear" => tra("less than a year"),
        "tmoreyear" => tra("more than a year"),
    ),
    "radio_name" => "duration",
    "other_name" => null,
);
$choice1 = array(
    "text" => tra("No - I used to run BOINC, but..."),
    "rname" => "lapsed",
    "options" => array(
        "linterest" => tra("I lost interest"),
        "lcomplicated" => tra("it was too complicated"),
        "lstopped" => tra("I stopped it and forgot to start again"),
        "lproblems" => tra("it caused problems on my computer"),
        "lpower" => tra("it used too much electricity"),
        "lnonboinc" => tra("I switched to a non-BOINC computing project"),
    ),
    "radio_name" => false,
    "other_name" => "lother_text",
);
$choice2 = array(
    "text" => tra("No - I tried running BOINC, but..."),
    "rname" => "failed",
    "options" => array(
        "finstall" => tra("the software didn't install correctly"),
        "ffigure" => tra("I couldn't figure out how to use the software"),
        "fnetwork" => tra("I had network communication problems"),
        "fattach" => tra("I couldn't attach to a project"),
        "fwork" => tra("I attached to a project, but never got any work"),
    ),
    "radio_name" => false,
    "other_name" => "fother_text",
);
$choice3 = array(
    "text" => tra("No - I never tried running BOINC, because..."),
    "rname" => "never",
    "options" => array(
        "nsecurity" => tra("I was concerned about security"),
        "nproject" => tra("I wasn't interested in any of the projects"),
        "npermission" => tra("I don't have permission to run it on my computer"),
        "nversion" => tra("No version was available for my computer"),
    ),
    "radio_name" => false,
    "other_name" => "nother_text",
);

$overall_choices = array($choice0, $choice1, $choice2, $choice3);


$comp_items = array(
    array(
        "text"=> tra("What kind of computers do you have running BOINC?"),
        "options" => array(
            "windows"=>"Windows",
            "mac"=>"Macintosh",
            "linux"=>"Linux",
        ),
        "radio_name"=>false,
        "other_name"=>"cother_text",
    ),
    array(
        "text"=> tra("Where are they?"),
        "options" => array(
            "home"=> tra("Home"),
            "work"=> tra("Work"),
            "school"=> tra("School"),
        ),
        "radio_name"=>false,
        "other_name"=>"vother_text",
    ),
    array(
        "radio_name"=>"ncomputers",
        "text"=> tra("How many computers?"),
        "options" => array(
            "n1"=>"1",
            "n2"=>"2-4",
            "n59"=>"5-9",
            "n10"=>"10-99",
            "n100"=>"100+",
        ),
    ),
    array(
        "radio_name"=>"turned_on",
        "text"=> tra("On average, how many hours per day are they powered on?"),
        "options" => array(
            "t08"=>"0-8",
            "t823"=>"8-23",
            "t24"=>"24",
        ),
    ),
);

$you_items = array(
    array(
        "radio_name"=>"age",
        "text"=> tra("Age"),
        "options"=>array(
            "age019"=>"0-19",
            "age2039"=>"20-39",
            "age4059"=>"40-59",
            "age60"=>"60+",
        ),
    ),
    array(
        "radio_name"=>"sex",
        "text"=> tra("Sex"),
        "options"=>array(
            "sexm"=> tra("Male"),
            "sexf"=> tra("Female"),
        ),
    ),
    array(
        "radio_name"=>"level",
        "text"=> tra("Your level of computer expertise"),
        "options"=>array(
            "levb"=> tra("Beginner"),
            "levi"=> tra("Intermediate"),
            "leva"=> tra("Advanced"),
        ),
    ),
);

$project_items = array(
    array(
        "name"=>"where",
        "text"=> tra("Where did you learn about BOINC projects?"),
        "options"=>array(
            "wtv"=> tra("TV/radio/newspaper"),
            "wpers"=> tra("From friends, relatives, or coworkers"),
            "wteam"=> tra("Team message boards or web sites"),
            "wboinc"=> tra("The BOINC web site"),
            "wweb"=> tra("Other web sites"),
        ),
        "radio_name" => false,
        "other_name" => "wother_text",
    ),
    array(
        "text" => tra("Which are the most important factors when you decide whether to participate in a BOINC project?"),
        "options" => array(
            "factor_graphics"=> tra("Nice-looking screensaver graphics"),
            "factor_credit_fair"=> tra("Fair and quick granting of credit for work done"),
            "factor_credit_more"=> tra("Getting more credit from this project than from others"),
            "factor_mb_friendly"=> tra("Helpful and friendly message boards"),
            "factor_mb_staff"=> tra("Participation by project staff on the message boards"),
            "factor_web_site"=> tra("Informative project web site"),
            "factor_science_important"=> tra("The science is important and beneficial"),
            "factor_science_public"=> tra("Non-profit, and results are public"),
            "factor_recognize"=> tra("Personal recognition if my computer finds something"),
            "factor_publish"=> tra("Publication by the project in scientific journals"),
            "factor_email"=> tra("Periodic email newsletters from the project"),
        ),
        "radio_name" => false,
        "other_name" => "factor_other_text",
    ),
    array(
        "radio_name"=>"nprojects",
        "text"=> tra("How many BOINC projects do you participate in?"),
        "options"=>array(
            "p1"=>"1",
            "p25"=>"2-5",
            "p510"=>"5-10",
            "pmore10"=>"11+",
        ),
    ),
    array(
        "radio_name"=>"ssaver",
        "text"=> tra("Do you run BOINC as a screensaver?"),
        "options"=>array(
            "ss_yes"=> tra("Yes"),
            "ss_no"=> tra("No"),
        ),
    ),
    array(
        "radio_name"=>"mboards",
        "text"=> tra("Your usage of project messages boards:"),
        "options"=>array(
            "mboard_r"=> tra("to read information"),
            "mboard_rw"=> tra("to read and post information"),
            "mboard_none"=> tra("None"),
        ),
    ),
    array(
        "text"=> tra("Where do you get information to help resolve problems with BOINC and/or BOINC projects?"),
        "options"=>array(
            "help_pmb"=> tra("The project message boards"),
            "help_bmb"=> tra("The BOINC message boards"),
            "help_bdoc"=> tra("The BOINC web site"),
            "help_bmlist"=> tra("BOINC mailing lists"),
            "help_wiki"=> tra("The Unofficial BOINC Wiki"),
            "help_team"=> tra("Team web sites"),
            "help_google"=> tra("Google or other search engines"),
        ),
        "radio_name"=>false,
        "other_name"=>help_other_text,
    ),
);

function parse_form() {
    global $run_boinc;
    global $overall_choices;
    global $project_items;
    global $comp_items;
    global $you_items;
    global $country;
    global $improved;
    $x = array();
    $x[$run_boinc] = get_str($run_boinc);
    $x = parse_form_choices($x, $overall_choices);
    $x = parse_form_choices($x, $project_items);
    $x = parse_form_choices($x, $comp_items);
    $x = parse_form_choices($x, $you_items);
    $x[$country] = get_str($country);
    $x[$improved] = get_str($improved);
    return $x;
}

function generate_xml($x) {
    global $run_boinc;
    global $overall_choices;
    global $project_items;
    global $comp_items;
    global $you_items;
    global $country;
    global $improved;
    $xml = "";
    $xml .= "<$run_boinc>".$x[$run_boinc]."</$run_boinc>\n";
    $xml .= gen_xml_choices($x, $overall_choices);
    $xml .= gen_xml_choices($x, $project_items);
    $xml .= gen_xml_choices($x, $comp_items);
    $xml .= gen_xml_choices($x, $you_items);
    $xml .= "<$country>".urlencode($x[$country])."</$country>\n";
    $xml .= "<$improved>".urlencode($x[$improved])."</$improved>\n";
    return $xml;
}

?>

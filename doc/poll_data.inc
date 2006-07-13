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
    "text" => "Yes - I have been running BOINC on my computer for...",
    "rname" => "current",
    "options" => array(
        "tweek" => "less than a week",
        "tmonth" => "less than a month",
        "tyear" => "less than a year",
        "tmoreyear" => "more than a year"
    ),
    "radio_name" => "duration",
    "other_name" => null,
);
$choice1 = array(
    "text" => "No - I was running BOINC, but stopped because...",
    "rname" => "lapsed",
    "options" => array(
        "linterest" => "I lost interest",
        "lcomplicated" => "it was too complicated",
        "lstopped" => "I stopped it and forgot to start again",
        "lproblems" => "it caused problems on my computer",
        "lpower" => "it used too much electricity",
        "lnonboinc" => "I switched to a non-BOINC computing project",
    ),
    "radio_name" => false,
    "other_name" => "lother_text",
);
$choice2 = array(
    "text" => "No - I tried running BOINC, but...",
    "rname" => "failed",
    "options" => array(
        "finstall" => "the software didn't install correctly",
        "ffigure" => "I couldn't figure out how to use the software",
        "fnetwork" => "I had network communication problems",
        "fattach" => "I couldn't attach to a project",
        "fwork" => "I attached to a project, but never got any work",
    ),
    "radio_name" => false,
    "other_name" => "fother_text",
);
$choice3 = array(
    "text" => "No - I never tried running BOINC, because...",
    "rname" => "never",
    "options" => array(
        "nsecurity" => "I was concerned about security",
        "nproject" => "I wasn't interested in any of the projects",
        "npermission" => "I don't have permission to run it on my computer",
        "nversion" => "No version was available for my computer",
    ),
    "radio_name" => false,
    "other_name" => "nother_text",
);

$overall_choices = array($choice0, $choice1, $choice2, $choice3);


$comp_items = array(
    array(
        "text"=>"What kind of computers do you have running BOINC?",
        "options" => array(
            "windows"=>"Windows",
            "mac"=>"Macintosh",
            "linux"=>"Linux",
        ),
        "radio_name"=>false,
        "other_name"=>"cother_text",
    ),
    array(
        "text"=>"Where are they?",
        "options" => array(
            "home"=>"Home",
            "work"=>"Work",
            "school"=>"School",
        ),
        "radio_name"=>false,
        "other_name"=>"vother_text",
    ),
    array(
        "radio_name"=>"ncomputers",
        "text"=>"How many computers?",
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
        "text"=>"On average, how many hours per day are they powered on?",
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
        "text"=>"Age",
        "options"=>array(
            "age019"=>"0-19",
            "age2039"=>"20-39",
            "age4059"=>"40-59",
            "age60"=>"60+",
        ),
    ),
    array(
        "radio_name"=>"sex",
        "text"=>"Sex",
        "options"=>array(
            "sexm"=>"Male",
            "sexf"=>"Female",
        ),
    ),
    array(
        "radio_name"=>"level",
        "text"=>"Your level of computer expertise",
        "options"=>array(
            "levb"=>"Beginner",
            "levi"=>"Intermediate",
            "leva"=>"Advanced",
        ),
    ),
);

$project_items = array(
    array(
        "name"=>"where",
        "text"=>"Where did you learn about BOINC projects?",
        "options"=>array(
            "wtv"=>"TV/radio/newspaper",
            "wweb"=>"The web",
            "wpers"=>"From other people",
        ),
        "radio_name" => false,
        "other_name" => "wother_text",
    ),
    array(
        "text" => "Which are the most important factors when you decide whether to participate in a BOINC project?",
        "options" => array(
            "factor_graphics"=>"Nice-looking screensaver graphics",
            "factor_credit_fair"=>"Fair and quick granting of credit for work done",
            "factor_credit_more"=>"Getting more credit from this project than from others",
            "factor_mb_friendly"=>"Helpful and friendly message boards",
            "factor_mb_staff"=>"Participation by project staff on the message boards",
            "factor_web_site"=>"Informative project web site",
            "factor_science_important"=>"The science is important and beneficial",
            "factor_science_public"=>"Non-profit, and results are public",
            "factor_recognize"=>"Personal recognition if my computer finds something",
            "factor_publish"=>"Publication by the project in scientific journals",
            "factor_email"=>"Periodic email newsletters from the project",
        ),
        "radio_name" => false,
        "other_name" => "factor_other_text",
    ),
    array(
        "radio_name"=>"nprojects",
        "text"=>"How many BOINC projects do you participate in?",
        "options"=>array(
            "p1"=>"1",
            "p25"=>"2-5",
            "p510"=>"5-10",
            "pmore10"=>"more than 10",
        ),
    ),
);
shuffle($project_items[0]['options']);

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

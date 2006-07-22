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
    "text" => tr(POLL_YES),
    "rname" => "current",
    "options" => array(
        "tweek" => tr(POLL_TWEEK),
        "tmonth" => tr(POLL_TMONTH),
        "tyear" => tr(POLL_TYEAR),
        "tmoreyear" => tr(POLL_TMOREYEAR),
    ),
    "radio_name" => "duration",
    "other_name" => null,
);
$choice1 = array(
    "text" => tr(POLL_LAPSED),
    "rname" => "lapsed",
    "options" => array(
        "linterest" => tr(POLL_LINTEREST),
        "lcomplicated" => tr(POLL_LCOMPLICATED),
        "lstopped" => tr(POLL_LSTOPPED),
        "lproblems" => tr(POLL_LPROBLEMS),
        "lpower" => tr(POLL_LPOWER),
        "lnonboinc" => tr(POLL_LNONBOINC),
    ),
    "radio_name" => false,
    "other_name" => "lother_text",
);
$choice2 = array(
    "text" => tr(POLL_FAILED),
    "rname" => "failed",
    "options" => array(
        "finstall" => tr(POLL_FINSTALL),
        "ffigure" => tr(POLL_FFIGURE),
        "fnetwork" => tr(POLL_FNETWORK),
        "fattach" => tr(POLL_FATTACH),
        "fwork" => tr(POLL_FWORK),
    ),
    "radio_name" => false,
    "other_name" => "fother_text",
);
$choice3 = array(
    "text" => tr(POLL_NEVER),
    "rname" => "never",
    "options" => array(
        "nsecurity" => tr(POLL_NSECURITY),
        "nproject" => tr(POLL_NPROJECT),
        "npermission" => tr(POLL_NPERMISSION),
        "nversion" => tr(POLL_NVERSION),
    ),
    "radio_name" => false,
    "other_name" => "nother_text",
);

$overall_choices = array($choice0, $choice1, $choice2, $choice3);


$comp_items = array(
    array(
        "text"=>tr(POLL_KIND),
        "options" => array(
            "windows"=>"Windows",
            "mac"=>"Macintosh",
            "linux"=>"Linux",
        ),
        "radio_name"=>false,
        "other_name"=>"cother_text",
    ),
    array(
        "text"=>tr(POLL_WHERE),
        "options" => array(
            "home"=>tr(POLL_HOME),
            "work"=>tr(POLL_WORK),
            "school"=>tr(POLL_SCHOOL),
        ),
        "radio_name"=>false,
        "other_name"=>"vother_text",
    ),
    array(
        "radio_name"=>"ncomputers",
        "text"=>tr(POLL_HOW_MANY),
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
        "text"=>tr(POLL_TURNED_ON),
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
        "text"=>tr(POLL_AGE),
        "options"=>array(
            "age019"=>"0-19",
            "age2039"=>"20-39",
            "age4059"=>"40-59",
            "age60"=>"60+",
        ),
    ),
    array(
        "radio_name"=>"sex",
        "text"=>tr(POLL_SEX),
        "options"=>array(
            "sexm"=>tr(POLL_MALE),
            "sexf"=>tr(POLL_FEMALE),
        ),
    ),
    array(
        "radio_name"=>"level",
        "text"=>tr(POLL_EXPERTISE),
        "options"=>array(
            "levb"=>tr(POLL_LEVB),
            "levi"=>tr(POLL_LEVI),
            "leva"=>tr(POLL_LEVA),
        ),
    ),
);

$project_items = array(
    array(
        "name"=>"where",
        "text"=>tr(POLL_WHERE),
        "options"=>array(
            "wtv"=>tr(POLL_WTV),
            "wpers"=>tr(POLL_WPERS),
            "wteam"=>tr(POLL_WTEAM),
            "wboinc"=>tr(POLL_WBOINC),
            "wweb"=>tr(POLL_WWEB),
        ),
        "radio_name" => false,
        "other_name" => "wother_text",
    ),
    array(
        "text" => tr(POLL_FACTOR),
        "options" => array(
            "factor_graphics"=>tr(POLL_GRAPHICS),
            "factor_credit_fair"=>tr(POLL_CREDIT_FAIR),
            "factor_credit_more"=>tr(POLL_CREDIT_MORE),
            "factor_mb_friendly"=>tr(POLL_MB_FRIENDLY),
            "factor_mb_staff"=>tr(POLL_MB_STAFF),
            "factor_web_site"=>tr(POLL_WEB_SITE),
            "factor_science_important"=>tr(POLL_SCIENCE_IMPORTANT),
            "factor_science_public"=>tr(POLL_SCIENCE_PUBLIC),
            "factor_recognize"=>tr(POLL_RECOGNIZE),
            "factor_publish"=>tr(POLL_PUBLISH),
            "factor_email"=>tr(POLL_EMAIL),
        ),
        "radio_name" => false,
        "other_name" => "factor_other_text",
    ),
    array(
        "radio_name"=>"nprojects",
        "text"=>tr(POLL_NPROJECTS),
        "options"=>array(
            "p1"=>"1",
            "p25"=>"2-5",
            "p510"=>"5-10",
            "pmore10"=>"11+",
        ),
    ),
    array(
        "radio_name"=>"ssaver",
        "text"=>tr(POLL_SSAVER),
        "options"=>array(
            "ss_yes"=>tr(POLL_YES),
            "ss_no"=>tr(POLL_NO),
        ),
    ),
    array(
        "radio_name"=>"mboards",
        "text"=>tr(POLL_MBOARDS),
        "options"=>array(
            "mboard_r"=>tr(POLL_MBR),
            "mboard_rw"=>tr(POLL_MBRW),
            "mboard_none"=>tr(POLL_NONE),
        ),
    ),
    array(
        "text"=>tr(POLL_HELP),
        "options"=>array(
            "help_pmb"=>tr(POLL_HELP_PMB),
            "help_bmb"=>tr(POLL_HELP_BMB),
            "help_bdoc"=>tr(POLL_HELP_BDOC),
            "help_bmlist"=>tr(POLL_HELP_BMLIST),
            "help_wiki"=>tr(POLL_HELP_WIKI),
            "help_team"=>tr(POLL_HELP_TEAM),
            "help_google"=>tr(POLL_HELP_GOOGLE),
        ),
        "radio_name"=>false,
        "other_name"=>help_other_text,
    ),
);
shuffle($project_items[1]['options']);

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

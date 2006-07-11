<?php

$choice0 = array(
    "text" => "I'm currently running BOINC on my computer",
    "rname" => "current",
    "boxes" => array(
        "tweek" => "less than a week",
        "tmonth" => "less than a month",
        "t6months" => "less than six months",
        "tmore" => "more than six months"
    ),
    "radio_name" => "duration"
);
$choice1 = array(
    "text" => "I was running BOINC, but stopped because",
    "rname" => "lapsed",
    "boxes" => array(
        "linterest" => "I lost interest",
        "lcomplicated" => "it was too complicated",
        "lstopped" => "I stopped it and forgot to start again",
        "lproblems" => "it caused problems on my computer",
        "lpower" => "it used too much electricity",
        "lnonboinc" => "I switched to a non-BOINC computing project",
        "lother" => "other: <input name=foo size=30>"
    ),
    "radio_name" => false
);
$choice2 = array(
    "text" => "I tried BOINC, but",
    "rname" => "failed",
    "boxes" => array(
        "finstall" => "the software didn't install correctly",
        "ffigure" => "I couldn't figure out how to use the software",
        "fnetwork" => "I had network communication problems",
        "fattach" => "I couldn't attach to a project",
        "fwork" => "I attached to a project, but never got any work",
        "fother" => "Other: <input name=foo size=30>"
    ),
    "radio_name" => false
);
$choice3 = array(
    "text" => "I never tried BOINC, because",
    "rname" => "never",
    "boxes" => array(
        "nsecurity" => "I was concerned about securty",
        "nheard" => "I never heard about it",
    ),
    "radio_name" => false
);

$factors = array(
    array(
        "text"=>"Nice-looking screensaver graphics",
        "name"=>"factor_graphics"
    ),
    array(
        "text"=>"Fair and quick granting of credit for work done",
        "name"=>"factor_graphics"
    ),
    array(
        "text"=>"Getting more credit from this project than from others",
        "name"=>"factor_graphics"
    ),
    array(
        "text"=>"Helpful and friendly message boards",
        "name"=>"factor_message_boards"
    ),
    array(
        "text"=>"Participation by project staff on the message boards",
        "name"=>"factor_message_boards"
    ),
    array(
        "text"=>"Informative project web site",
        "name"=>"factor_web_site"
    ),
    array(
        "text"=>"Importance of the science",
        "name"=>"factor_science"
    ),
    array(
        "text"=>"Scientific results are public",
        "name"=>"factor_science"
    ),
    array(
        "text"=>"Personal recognition if my computer finds something",
        "name"=>"factor_science"
    ),
    array(
        "text"=>"Publication by the project in refereed journals",
        "name"=>"factor_science"
    ),
    array(
        "text"=>"Periodic email newsletters from the project",
        "name"=>"factor_science"
    ),
);

$comp_items = array(
    array(
        "name"=>"kind",
        "text"=>"What kind of computer do you have?",
        "choices" => array(
            "windows"=>"Windows",
            "mac"=>"Macintosh",
            "linux"=>"Linux",
            "other_unix"=>"Other Unix",
        ),
    ),
    array(
        "name"=>"venue",
        "text"=>"Where is your computer?",
        "choices" => array(
            "home"=>"Home",
            "work"=>"Work",
            "school"=>"School",
        ),
    ),
    array(
        "name"=>"ncomputers",
        "text"=>"How many computers do you have?",
        "choices" => array(
            "n1"=>"1",
            "n2"=>"2-4",
            "n59"=>"5-9",
            "n10"=>"10+",
        ),
    ),
    array(
        "name"=>"turned_on",
        "text"=>"On average, how many hours a day is your computer turned on?",
        "choices" => array(
            "t08"=>"0-8",
            "t823"=>"8-23",
            "t24"=>"24",
        ),
    ),
);

$you_items = array(
    array(
        "name"=>"age",
        "text"=>"Age",
        "choices"=>array(
            "age019"=>"0-19",
            "age2039"=>"20-39",
            "age4059"=>"40-59",
            "age60"=>"60+",
        ),
    ),
    array(
        "name"=>"sex",
        "text"=>"Sex",
        "choices"=>array(
            "sexm"=>"Male",
            "sexf"=>"Female",
        ),
    ),
    array(
        "name"=>"level",
        "text"=>"Your level of computer expertise",
        "choices"=>array(
            "levb"=>"Beginner",
            "levi"=>"Intermediate",
            "leva"=>"Advanced",
        ),
    ),
);

$where_item = array(
    "name"=>"where",
    "text"=>"Where did you learn about your current project?",
    "choices"=>array(
        "wtv"=>"TV/radio/newspaper",
        "wweb"=>"The web",
        "wpers"=>"Someone told me about it",
        "woth"=>"Other: <input name=foo size=30>"
    ),
);

?>

<?php

$team_types = array(
    "None",
    "Unclassified",
    "Company",
    "Primary school",
    "Secondary school",
    "Junior college",
    "University or department",
    "Government agency"
);

function team_type_name($num) {
    global $team_types;
    if ($num>0 && $num<8) {
        return $team_types[$num];
    }
    return "Unknown";
}

function is_valid_team_type($name) {
    global $team_types;
    foreach ($team_types as $type) {
        if ($name == $type) return true;
    }
    return false;
}

?>

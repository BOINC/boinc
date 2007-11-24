<?php

$team_types = array(
    "",
    "None",
    "Company",
    "Primary school",
    "Secondary school",
    "Junior college",
    "University or department",
    "Government agency",
    "Non-profit organization",
    "National",
    "Local/regional",
    "Computer type",
    "Social/political/religious"
);

function team_type_name($num) {
    global $team_types;
    if ($num>0 && $num<count($team_types)) {
        return $team_types[$num];
    }
    return "None";
}

function is_valid_team_type($name) {
    global $team_types;
    foreach ($team_types as $type) {
        if ($name == $type) return true;
    }
    return false;
}

function team_type_num($name) {
    global $team_types;
    for ($i=0; $i<sizeof($team_types); $i++) {
        if ($name == $team_types[$i]) return $i;
    }
    return 0;
}

function team_type_select($selected_type, $allow_none=false){
    global $team_types;

    $types = $team_types;
    if ($allow_none) {
        $types[0] = '---';
        $type = 0;
    }
    return select_from_array('type', $types, $selected_type);
}

?>

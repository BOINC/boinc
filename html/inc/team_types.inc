<?php

$team_types = array(
    "",
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

function team_type_num($name) {
    global $team_types;
    for ($i=0; $i<sizeof($team_types); $i++) {
        if ($name == $team_types[$i]) return $i;
    }
    return 0;
}

function team_type_select($team){
    global $team_types;

    if (!empty($team)) {
        $type = $team->type;
    } else {
        $type = 1;
    }

    return select_from_array('type', $team_types, $type);
}

?>

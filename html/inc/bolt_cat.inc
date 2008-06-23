<?php

// represents a categorization of students
//
class Categorization {
    abstrcat function name();
        // returns descriptive name
    abstract function categories();
        // returns list of categories
    abstract function categorize($user);
        // returns a student's category
}

class CatSex extends Categorization {
    function name() {
        return "Sex";
    }
    function categories() {
        return array ("Male", "Female", "Unknown");
    }
    function categorize($user) {
        switch ($user->bolt->sex) {
        case 1: return "Male";
        case 2: return "Female";
        default: return "Unknown";
        }
    }
}

$x = localtime(time(), true);
$this_year = 1900 + $x['tm_year'];

class CatAge20 extends Categorization {
    function name() {
        return "Age (20-year groups)";
    }
    function categories() {
        return array("0-19", "20-39", "40-59", "60-79", "80+", "Unknown");
    }
    function categorize($user) {
        if (!$user->bolt->birth_year) return "Unknown";
        global $this_year;
        $n = $this_year - $user->bolt->birth_year;
        if ($n < 20) return "0-19";
        if ($n < 40) return "20-39";
        if ($n < 60) return "40-59";
        if ($n < 80) return "60-79";
        return "80+";
    }
}

$categorizations = array("CatSex", "CatAge20");

?>

<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// classes for different kinds of global preferences.  See prefs.inc
// PREF_BOOL: boolean
// PREF_NUM: a number, possibly constrained to a range
// PREF_NUM2: a pair of numbers (e.g. transfer limit)
// PREF_HOUR_RANGE: a range of hours

$venues = array("home", "school", "work");

function check_venue($x) {
    if ($x == "") return;
    if ($x == "home") return;
    if ($x == "work") return;
    if ($x == "school") return;
    error_page(tra("bad venue: %1", $x));
}

function check_subset($x) {
    if ($x == "global") return;
    if ($x == "project") return;
    error_page(tra("bad subset: %1", $x));
}

abstract class PREF {
    public $desc;
    public $tag;    // the pref's primary XML tag
    function __construct($desc, $tag) {
        $this->desc = $desc;
        $this->tag = $tag;
    }

    abstract function show_value($prefs);
    abstract function show_form($prefs, $error);
    abstract function parse_form(&$prefs, &$error);
    abstract function xml_string($prefs);
    abstract function xml_parse(&$prefs, $name, $text);
    abstract function set_default(&$prefs);

    // multi-column display (read only)
    //
    function show_cols($prefs) {
        global $venues;
        echo "<tr><td class=fieldname>$this->desc</td>";
        $tag = $this->tag;
        if (isset($prefs->$tag)) {
            $this->show_value($prefs);
        } else {
            echo "<td>---</td>";
        }
        foreach ($venues as $v) {
            if (isset($prefs->$v) && isset($prefs->$v->$tag)) {
                $this->show_value($prefs->$v);
            } else {
                echo "<td>---</td>";
            }
        }
        echo "</tr>\n";
    }

    // show read-only row
    //
    function show($prefs) {
        echo "<tr><td class=fieldname>$this->desc</td>";
        $tag = $this->tag;
        if (isset($prefs->$tag)) {
            $this->show_value($prefs);
        } else {
            echo "<td>---</td>";
        }
        echo "</tr>\n";
    }

    // show form row
    //
    function show_form_row($prefs, $error) {
        $tag = $this->tag;
        $class = isset($error->$tag)?"fieldname_error":"fieldname";
        echo "<tr><td class=$class>$this->desc</td>";
        $this->show_form($prefs, $error);
        echo "</tr>\n";
    }
}

class PREF_BOOL extends PREF {
    public $default;
    public $invert;     // show to user in opposite sense
    function __construct($desc, $tag, $default, $invert=false) {
        $this->default = $default;
        $this->invert = $invert;
        parent::__construct($desc, $tag);
    }
    function show_value($prefs) {
        $tag = $this->tag;
        $v = $this->invert?!$prefs->$tag:$prefs->$tag;
        $x = $v?tra("yes"):tra("no");
        echo "<td>$x</td>";
    }
    function show_form($prefs, $error) {
        $tag = $this->tag;
        if ($this->invert) {
            $checked = !$prefs->$tag;
        } else {
            $checked = $prefs->$tag;
        }
        echo "<td class=fieldvalue>"
            ."<input type=checkbox name=$this->tag "
            . ($checked?"checked":"")
            ."></td>
        ";
    }
    function parse_form(&$prefs, &$error) {
        $tag = $this->tag;
        $val = array_key_exists($tag, $_GET);
        if ($this->invert) $val = !$val;
        $prefs->$tag = $val;
    }
    function xml_string($prefs) {
        $tag = $this->tag;
        return "<$tag>"
            .($prefs->$tag?"1":"0")
            ."</$tag>\n";
    }
    function xml_parse(&$prefs, $name, $text) {
        $tag = $this->tag;
        if ($name != $tag) return false;
        $val = (trim($text) != '0');
        $prefs->$tag = $val;
        return true;
    }
    function set_default(&$prefs) {
        $tag = $this->tag;
        $prefs->$tag = $this->default;
    }
}

class NUM_SPEC {
    public $suffix;
    public $min;
    public $max;
    public $default;
    public $scale;

    function __construct($suffix, $min, $max, $default, $scale=1) {
        if (substr($suffix, 0, 1) == "%") {
            $this->suffix = $suffix;
        } else {
            $this->suffix = " $suffix";
        }
        $this->min = $min;
        $this->max = $max;
        $this->default = $default;
        $this->scale = $scale;
    }
    function value_str($v) {
        $v /= $this->scale;
        if ($v == 0) {
            $v = "--- ";
        }
        $v .= "$this->suffix ";
        return $v;
    }
    function form_str($tag, $v) {
        if (is_numeric($v)) {
            $v /= $this->scale;
            if ($v == 0) $v = "";
        }
        return "<input size=5 name=$tag value='$v'> $this->suffix ";
    }
    function form_convert($in, &$out, &$error) {
        $error = false;
        if ($in == "") $in = 0;
        if (!is_numeric($in)) {
            $error = true;
            $out = $in;
            return;
        }
        $out = $in*$this->scale;
        if ($out < $this->min || $out > $this->max) {
            $error = true;
        }
    }
}

class PREF_NUM extends PREF {
    public $num_spec;
    function __construct($desc, $tag, $num_spec) {
        $this->num_spec = $num_spec;
        parent::__construct($desc, $tag);
    }
    function show_value($prefs) {
        $tag = $this->tag;
        echo "<td>".$this->num_spec->value_str($prefs->$tag)."</td>";
    }
    function show_form($prefs, $error) {
        $tag = $this->tag;
        $class = isset($error->$tag)?"fieldvalue_error":"fieldvalue";
        echo "<td class=$class>"
            .$this->num_spec->form_str($tag, $prefs->$tag)
            ."</td>
        ";
    }
    function parse_form(&$prefs, &$error) {
        $tag = $this->tag;
        $this->num_spec->form_convert($_GET[$tag], $prefs->$tag, $e);
        if ($e) $error->$tag = true;
    }
    function xml_string($prefs) {
        $tag = $this->tag;
        $v = $prefs->$tag;
        return "<$tag>$v</$tag>\n";
    }
    function xml_parse(&$prefs, $name, $text) {
        $tag = $this->tag;
        if ($name != $tag) return false;
        $prefs->$tag = $text;
        return true;
    }
    function set_default(&$prefs) {
        $tag = $this->tag;
        $prefs->$tag = $this->num_spec->default;
    }
}

class PREF_NUM2 extends PREF {
    public $tag2;
    public $num_spec1, $num_spec2;
    function __construct($desc, $tag1, $tag2, $num_spec1, $num_spec2) {
        $this->tag2 = $tag2;
        $this->num_spec1 = $num_spec1;
        $this->num_spec2 = $num_spec2;
        parent::__construct($desc, $tag1);
    }
    function show_value($prefs) {
        $tag = $this->tag;
        $tag2 = $this->tag2;
        echo "<td>"
            .$this->num_spec1->value_str($prefs->$tag)
            .$this->num_spec2->value_str($prefs->$tag2)
            ."</td>";
    }
    function show_form($prefs, $error) {
        $tag = $this->tag;
        $tag2 = $this->tag2;
        $class = (isset($error->$tag) || isset($error->$tag2))?"fieldvalue_error":"fieldvalue";
        echo "<td class=$class>"
            .$this->num_spec1->form_str($tag, $prefs->$tag)
            .$this->num_spec2->form_str($tag2, $prefs->$tag2)
            ."</td>
        ";
    }
    function parse_form(&$prefs, &$error) {
        $tag = $this->tag;
        $tag2 = $this->tag2;
        $this->num_spec1->form_convert($_GET[$tag], $prefs->$tag, $e);
        if ($e) $error->$tag = true;
        $this->num_spec2->form_convert($_GET[$tag2], $prefs->$tag2, $e);
        if ($e) $error->$tag2 = $e;
    }
    function xml_string($prefs) {
        $tag = $this->tag;
        $tag2 = $this->tag2;
        return "<$tag>".$prefs->$tag."</$tag>\n<$tag2>".$prefs->$tag2."</$tag2>\n";
    }
    function xml_parse(&$prefs, $name, $text) {
        $tag = $this->tag;
        $tag2 = $this->tag2;
        if ($name == $tag) {
            $prefs->$tag = $text;
        } else if ($name == $tag2) {
            $prefs->$tag2 = $text;
        }
        return false;
    }
    function set_default(&$prefs) {
        $tag = $this->tag;
        $tag2 = $this->tag2;
        $prefs->$tag = $this->num_spec1->default;
        $prefs->$tag2 = $this->num_spec2->default;
    }
}

function hour_select($x, $name) {
    $s = "";
    $s = $s. "<select name=$name>\n";
    for ($i=0; $i<24; $i++) {
        $sel = ($x == $i)?"selected":"";
        $s = $s."<option value=$i $sel> $i:00";
    }
    $s = $s."</select>\n";
    return $s;
}

class PREF_HOUR_RANGE extends PREF {
    public $tag2;
    function __construct($desc, $tag, $tag2) {
        $this->tag2 = $tag2;
        parent::__construct($desc, $tag);
    }
    function show_value($prefs) {
        $tag = $this->tag;
        $tag2 = $this->tag2;
        $h1 = $prefs->$tag;
        $h2 = $prefs->$tag2;
        if ($h1 == $h2) {
            $v = "---";
        } else {
            $v = "$h1:00 ".tra("and")." $h2:00";
        }
        echo "<td>$v</td>";
    }
    function show_form($prefs, $error) {
        $tag = $this->tag;
        $tag2 = $this->tag2;
        echo "<td class=fieldvalue>"
            .hour_select($prefs->$tag, $tag)
            ." "
            .tra("and")
            ." "
            .hour_select($prefs->$tag2, $tag2)
            ."
        ";
    }
    function parse_form(&$prefs, &$error) {
        $tag = $this->tag;
        $tag2 = $this->tag2;
        $prefs->$tag = $_GET[$tag];
        $prefs->$tag2 = $_GET[$tag2];
    }
    function xml_string($prefs) {
        $tag = $this->tag;
        $tag2 = $this->tag2;
        $h1 = $prefs->$tag;
        $h2 = $prefs->$tag2;
        if ($h1 == $h2) return "";
        return "<$tag>$h1</$tag>\n<$tag2>$h2</$tag2>\n";
    }
    function xml_parse(&$prefs, $name, $text) {
        $tag = $this->tag;
        $tag2 = $this->tag2;
        if ($name == $tag) {
            $prefs->$tag = $text;
            return true;
        } else if ($name == $tag2) {
            $prefs->$tag2 = $text;
            return true;
        }
        return false;
    }
    function set_default(&$prefs) {
        $tag = $this->tag;
        $tag2 = $this->tag2;
        $prefs->$tag = 0;
        $prefs->$tag2 = 0;
    }
}

////////////////////////////////////////////
//
// display preference subsets as Columns
//
function row_top($x, $ncols=6, $class="heading") {
    echo "<tr><td class=$class width=35%>$x</td>";
    echo "<td class=$class width=10%><b>".tra("Default")."</b></td>
        <td class=$class width=10%><b>".tra("Home")."</b></td>
        <td class=$class width=10%><b>".tra("School")."</b></td>
        <td class=$class width=10%><b>".tra("Work")."</b></td>
    ";
    echo "<td width=15%><br></td></tr>\n";
}

//
// row_defs - Display a value for all 4 venues in one row
//
function row_defs($pre, $item, $post, $type, $prefs) {
    $gen = $prefs->$item;
    $hom  = (isset($prefs->home) && isset($prefs->home->$item)) ? $prefs->home->$item : "--";
    $schl = (isset($prefs->school) && isset($prefs->school->$item)) ? $prefs->school->$item : "--";
    $wrk  = (isset($prefs->work) && isset($prefs->work->$item)) ? $prefs->work->$item : "--";

    echo "<tr><td class=fieldname>$pre</td>";
    row_field($gen,  $type);
    row_field($hom,  $type);
    row_field($schl, $type);
    row_field($wrk,  $type);
    echo "<td align=left>$post</td></tr>\n";
}

//
// row_field - Display each field value, with selectable display modes
//
function row_field($value, $type) {
    echo "<td class=f_val valign=top>";
    $type = $value === "--" ? "--" : $type;
    switch($type) {
    case "yesno":
        echo $value ?tra("yes"):tra("no");
        break;
    case "noyes":
        echo $value ?tra("no"):tra("yes");
        break;
    case "limit":
        $x = max_bytes_display_mode($value);
        $y = "$x " . BYTE_ABBR;
        echo $x ? "$y" : tra("no limit");
        break;
    case "minutes":
        if ($value) {
            echo $value;
        } else {
            echo '--';
        }
        break;
    default:
        echo $value;
        break;
    }
    echo "</td>";
}

//
// row_links - Display Edit/Add/Remove links for all venues in 1 row
//
function row_links($subset, $prefs) {
    global $g_logged_in_user;
    $tokens = url_tokens($g_logged_in_user->authenticator);
    $pre_add  = "<a href=add_venue.php?venue=";
    $pre_edit  = "<a href=prefs_edit.php?venue=";
    $pre_remove = "<a href=prefs_remove.php?venue=";
    $post_add = "&subset=$subset&cols=1$tokens>".tra("Add")."</a>";
    $post_edit = "&subset=$subset&cols=1$tokens>".tra("Edit")."</a>";
    $post_remove = "&subset=$subset&cols=1$tokens>".tra("Remove")."</a>";
    $gen = "<a href=prefs_edit.php?subset=$subset&cols=1$tokens>".tra("Edit")."</a>";

    $hom  = isset($prefs->home) ? $pre_edit."home".$post_edit : $pre_add."home".$post_add;
    $schl = isset($prefs->school) ? $pre_edit."school".$post_edit : $pre_add."school".$post_add;
    $wrk  = isset($prefs->work) ? $pre_edit."work".$post_edit : $pre_add."work".$post_add;

    echo "<tr><td class=fieldname> </td>";
    echo "<td>$gen</td>";
    echo "<td>$hom</td>";
    echo "<td>$schl</td>";
    echo "<td>$wrk</td>";
    echo "<td><br></td></tr>\n";

    $hom  = isset($prefs->home) ? $pre_remove."home".$post_remove : "<br>";
    $schl = isset($prefs->school) ? $pre_remove."school".$post_remove : "<br>";
    $wrk  = isset($prefs->work) ? $pre_remove."work".$post_remove : "<br>";

    echo "<tr><td class=fieldname> </td>";
    echo "<td> </td>";
    echo "<td>$hom</td>";
    echo "<td>$schl</td>";
    echo "<td>$wrk</td>";
    echo "<td><br></td></tr>\n";
}

?>

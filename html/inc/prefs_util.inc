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
    public $desc;       // short description
    public $tooltip;    // longer description, shown as tooltip
    public $tag;        // the pref's primary XML tag
    function __construct($desc, $tooltip, $tag) {
        $this->desc = $desc;
        $this->tooltip = $tooltip;
        $this->tag = $tag;
    }

    abstract function show_value($prefs);
    abstract function show_form($prefs, $error);
    abstract function parse_form(&$prefs, &$error);
    abstract function xml_string($prefs);
    abstract function xml_parse(&$prefs, $name, $text);
    abstract function set_default(&$prefs);

    function tooltip_tr() {
        if ($this->tooltip) {
            echo "<tr title=\"$this->tooltip\">";
        } else {
            echo "<tr>";
        }
    }

    // multi-column display (read only)
    //
    function show_cols($prefs) {
        global $venues;
        $this->tooltip_tr();
        echo "<td class=fieldname>$this->desc</td>";
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
        $this->tooltip_tr();
        echo "<td class=fieldname>$this->desc</td>";
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
        $this->tooltip_tr();
        echo "<td class=$class>$this->desc</td>";
        $this->show_form($prefs, $error);
        echo "</tr>\n";
    }
}

function readonly_checkbox($checked) {
    if ($checked) {
        return "<input type=checkbox onclick=\"return false\" checked>";
    } else {
        return "<input type=checkbox onclick=\"return false\">";
    }
}

class PREF_BOOL extends PREF {
    public $default;
    public $invert;     // show to user in opposite sense
    function __construct($desc, $tooltip, $tag, $default, $invert=false) {
        $this->default = $default;
        $this->invert = $invert;
        parent::__construct($desc, $tooltip, $tag);
    }
    function show_value($prefs) {
        $tag = $this->tag;
        $v = $this->invert?!$prefs->$tag:$prefs->$tag;
        echo "<td>".readonly_checkbox($v)."</td>";
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
    public $default2;
        // for optional prefs where the default is zero (ignored)
        // this is the value if they check the box
    public $scale;

    function __construct($suffix, $min, $max, $default, $scale=1, $default2=0) {
        if (substr($suffix, 0, 1) == "%") {
            $this->suffix = $suffix;
        } else {
            $this->suffix = " $suffix";
        }
        $this->min = $min;
        $this->max = $max;
        $this->default = $default;
        $this->default2 = $default2;
        $this->scale = $scale;
    }
    function value_str($v) {
        $v /= $this->scale;
        if ($v < $this->min || $v > $this->max) {
            $v = $this->default;
        }
        if ($v == 0) {
            $v = "--- ";
        }
        $v .= "$this->suffix ";
        return $v;
    }
    function form_str($tag, $v, $had_error, $disabled=false, $id=null) {
        if (is_numeric($v)) {
            $v /= $this->scale;
            if (!$had_error && ($v < $this->min || $v > $this->max)) {
                $v = $this->default;
            }
        }
        if ($disabled) $v = "";
        $i = $id?"id=$id":"";
        return "<input type=text size=5 name=$tag value='$v' $disabled $i>$this->suffix ";
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
    function get_default() {
        if ($this->default) return $this->default;
        return $this->default2;
    }
}

// a numeric item
//
class PREF_NUM extends PREF {
    public $num_spec;
    function __construct($desc, $tooltip, $tag, $num_spec) {
        $this->num_spec = $num_spec;
        parent::__construct($desc, $tooltip, $tag);
    }
    function show_value($prefs) {
        $tag = $this->tag;
        echo "<td>".$this->num_spec->value_str($prefs->$tag)."</td>";
    }
    function show_form($prefs, $error) {
        $tag = $this->tag;
        $had_error = isset($error->$tag);
        $class = $had_error ?"fieldvalue_error":"fieldvalue";
        echo "<td class=$class>"
            .$this->num_spec->form_str($tag, $prefs->$tag, $had_error)
            ."</td>
        ";
    }
    function parse_form(&$prefs, &$error) {
        $tag = $this->tag;
        $this->num_spec->form_convert(get_str($tag, true), $prefs->$tag, $e);
        if ($e) $error->$tag = true;
    }
    function xml_string($prefs) {
        $tag = $this->tag;
        $v = $prefs->$tag;
        if (!$v) $v = 0;
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

function checkbox_clicked_js() {
    echo "
        <script type=text/javascript>
        function checkbox_clicked(id, tid, d) {
            e = document.getElementById(id);
            t = document.getElementById(tid);
            if (e.checked) {
                t.disabled = false;
                t.value = d;
            } else {
                t.disabled = true;
                t.value = '';
            }
        }
        function checkbox_clicked2(id, tid1, tid2, d1, d2) {
            e = document.getElementById(id);
            t1 = document.getElementById(tid1);
            t2 = document.getElementById(tid2);
            if (e.checked) {
                t1.disabled = false;
                t1.value = d1;
                t2.disabled = false;
                t2.value = d2;
            } else {
                t1.disabled = true;
                t1.value = '';
                t2.disabled = true;
                t2.value = '';
            }
        }
        </script>
    ";
}

// an optional numeric item where 0 means not specified
//
class PREF_OPT_NUM extends PREF {
    public $num_spec;
    function __construct($desc, $tooltip, $tag, $num_spec) {
        $this->num_spec = $num_spec;
        parent::__construct($desc, $tooltip, $tag);
    }
    function show_value($prefs) {
        $tag = $this->tag;
        $x = $prefs->$tag;
        echo "<td>";
        echo $this->num_spec->value_str($x);
        echo "</td>";
    }
    function show_form($prefs, $error) {
        $tag = $this->tag;
        $had_error = isset($error->$tag);
        $class = $had_error ?"fieldvalue_error":"fieldvalue";
        $checkbox_id = $this->tag."_cb";
        $text_id = $this->tag;
        $default = $this->num_spec->get_default();
        $val = $prefs->$tag;
        $c = $val?"checked":"";
        $d = $val?"":"disabled";
        echo "<td class=$class>"
            ."<input type=checkbox id=$checkbox_id onClick=\"checkbox_clicked('$checkbox_id', '$text_id', $default)\" $c>"
            .$this->num_spec->form_str($tag, $prefs->$tag, $had_error, $d, $text_id)
            ."</td>
        ";
    }
    function parse_form(&$prefs, &$error) {
        $tag = $this->tag;
        $this->num_spec->form_convert(get_str($tag, true), $prefs->$tag, $e);
        if ($e) $error->$tag = true;
    }
    function xml_string($prefs) {
        $tag = $this->tag;
        $v = $prefs->$tag;
        if (!$v) $v = 0;
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

// optional pair of numbers
// for "max X MB in Y days"
//
class PREF_NUM2 extends PREF {
    public $tag2;
    public $num_spec1, $num_spec2;
    function __construct($desc, $tooltip, $tag1, $tag2, $num_spec1, $num_spec2) {
        $this->tag2 = $tag2;
        $this->num_spec1 = $num_spec1;
        $this->num_spec2 = $num_spec2;
        parent::__construct($desc, $tooltip, $tag1);
    }
    function show_value($prefs) {
        $tag = $this->tag;
        $tag2 = $this->tag2;
        $v1 = $prefs->$tag;
        $v2 = $prefs->$tag2;
        echo "<td>"
            .$this->num_spec1->value_str($v1)
            .$this->num_spec2->value_str($v2)
            ."</td>
        ";
    }
    function show_form($prefs, $error) {
        $tag = $this->tag;
        $tag2 = $this->tag2;
        $had_error = isset($error->$tag) || isset($error->$tag2);
        $class = $had_error?"fieldvalue_error":"fieldvalue";
        $checkbox_id = $this->tag."_cb";
        $t1_id = $this->tag."_t1";
        $t2_id = $this->tag."_t2";
        $v1 = $prefs->$tag;
        $v2 = $prefs->$tag2;
        if ($v1 && $v2) {
            $c = "checked";
            $d = "";
        } else {
            $c = "";
            $d = "disabled";
        }
        $def1 = $this->num_spec1->get_default();
        $def2 = $this->num_spec2->get_default();
        echo "<td class=$class>"
            ."<input type=checkbox id=$checkbox_id onClick=\"checkbox_clicked2('$checkbox_id', '$t1_id', '$t2_id', $def1, $def2)\" $c>"
            .$this->num_spec1->form_str($tag, $prefs->$tag, $had_error, $d, $t1_id)
            .$this->num_spec2->form_str($tag2, $prefs->$tag2, $had_error, $d, $t2_id)
            ."</td>
        ";
    }
    function parse_form(&$prefs, &$error) {
        $tag = $this->tag;
        $tag2 = $this->tag2;
        $this->num_spec1->form_convert(get_str($tag, true), $prefs->$tag, $e);
        if ($e) $error->$tag = true;
        $this->num_spec2->form_convert(get_str($tag2, true), $prefs->$tag2, $e);
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

function hour_select($x, $name, $id, $d) {
    $s = "";
    $s = $s. "<select name=$name id=$id $d>\n";
    for ($i=0; $i<24; $i++) {
        $sel = ($x == $i)?"selected":"";
        $s = $s."<option value=$i $sel> $i:00";
    }
    $s = $s."</select>\n";
    return $s;
}

// optional hour range
//
class PREF_HOUR_RANGE extends PREF {
    public $tag2;
    function __construct($desc, $tooltip, $tag, $tag2) {
        $this->tag2 = $tag2;
        parent::__construct($desc, $tooltip, $tag);
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
        $h1 = $prefs->$tag;
        $h2 = $prefs->$tag2;
        $checkbox_id = $this->tag."_cb";
        $t1_id = $this->tag."_t1";
        $t2_id = $this->tag."_t2";
        if ($h1 == $h2) {
            $c = "";
            $d = "disabled";
        } else {
            $c = "checked";
            $d = "";
        }
        echo "<td class=fieldvalue>"
            ."<input type=checkbox id=$checkbox_id onClick=\"checkbox_clicked2('$checkbox_id', '$t1_id', '$t2_id', 0, 23)\" $c>"

            .hour_select($prefs->$tag, $tag, $t1_id, $d)
            ." "
            .tra("and")
            ." "
            .hour_select($prefs->$tag2, $tag2, $t2_id, $d)
            ."
        ";
    }
    function parse_form(&$prefs, &$error) {
        $tag = $this->tag;
        $tag2 = $this->tag2;
        $prefs->$tag = get_str($tag, true);
        $prefs->$tag2 = get_str($tag2, true);
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
    echo "<tr><th class=$class width=35%>$x</th>";
    echo "<th class=$class width=10%><b>".tra("Default")."</b></th>
        <th class=$class width=10%><b>".tra("Home")."</b></th>
        <th class=$class width=10%><b>".tra("School")."</b></th>
        <th class=$class width=10%><b>".tra("Work")."</b></th>
    ";
    //echo "<th width=15%><br></th>
    echo "</tr>\n";
}

//
// row_defs - Display a value for all 4 venues in one row
//
function row_defs($pre, $item, $post, $type, $prefs, $tooltip=null) {
    $gen = $prefs->$item;
    $hom  = (isset($prefs->home) && isset($prefs->home->$item)) ? $prefs->home->$item : "--";
    $schl = (isset($prefs->school) && isset($prefs->school->$item)) ? $prefs->school->$item : "--";
    $wrk  = (isset($prefs->work) && isset($prefs->work->$item)) ? $prefs->work->$item : "--";

    if ($tooltip) {
        echo "<tr title=\"$tooltip\">";
    } else {
        echo "<tr>";
    }
    echo "<td class=fieldname>$pre</td>";
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

// see if we have any beta apps or app versions
//
function project_has_beta() {
    $apps = BoincApp::enum("deprecated=0 and beta>0");
    if (count($apps)) return true;
    $avs = BoincAppVersion::enum("deprecated=0 and beta>0");
    if (count($avs)) return true;
    return false;
}

?>

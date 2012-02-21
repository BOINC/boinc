<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

require_once("../inc/bossa.inc");

function job_show($job, $inst, $user) {
    $output = $inst->get_opaque_data();
    $features = $output->features;
    $input = $job->get_opaque_data();
    $path = $input->path;

    show_style();
    echo "
        <script>
            function image_clicked() {
                if (document.forms.main.type.value == 'none') {
                    alert('Please select a feature type');
                    return false;
                } else {
                    return true;
                }
                return false;
            }
        </script>
        <form name=main onsubmit=\"javascript: return image_clicked()\" action=bossa_example4.php>
        <div style=\"position:relative; top:60; \">
        <input type=hidden name=action value=add>
        <input type=hidden name=bji value=\"$inst->id\">
        <input type=image name=pic src=$path>
    ";
    $size = 100;
    $size2 = 50;
    $i = 0;
    foreach ($features as $f) {
        $cx = $f->x - $size2;
        $cy = $f->y - $size2;
        echo "
            <span style=\"width:".$size."px;height:".$size."px;position:absolute;top:".$cy.";left:".$cx.";color:white;border-style:solid;border-width:1px\">
        ";
        $c = $f->comment?"($f->comment)":"";
        echo "
            <span class=link>
            <a href=\"javascript: void(0)\">
            <img border=0 align = left src=example4_images/info_icon.png>
            <span>
            $f->type $c
            </span>
            </a>
            </span>
        ";
        echo "
            <a href=bossa_example4.php?bji=$inst->id&action=delete&index=$i><img border=0 align=right src=example4_images/close_button.jpg></a>
            </span>
        ";
        $i++;
    }
    echo "
        </div>
    ";
    control_bar($features, $inst);
    echo "
        </form>
    ";
}

function job_issued($job, $inst, $user) {
    $job->set_priority(0);
}

function job_finished($job, $inst) {
}

function job_timed_out($job, $inst, $user) {
    $job->set_priority(1);
}

function job_summary($job) {
    $info = $job->get_opaque_data();
    return "<a href=".URL_BASE."$info->path>View image</a>";
}

function instance_summary($opaque_data) {
    $features = $opaque_data->features;
    $x = "";
    foreach ($features as $f) {
        $c = $f->comment?"($f->comment)":"";
        $x .= "$f->type $c <br>";
    }
    return $x;
}

function user_summary($user) {
    return "";
}

function show_style() {
    echo "
        <style type=\"text/css\">

        body {
            font: 15px \"Trebuchet MS\", sans-serif;
        }

        span.link {
            position: relative;
        }

        span.link a span {
            display: none;
        }

        span.link a:hover {
            font-size: 99%;
            font-color: #000000;
        }

        span.link a:hover span {
            display: block;
            position: absolute;
            margin-top: 10px;
            margin-left: -10px;
            width: 175px; padding: 5px;
            z-index: 100;
            color: #000000;
            background: #ffffcc;
            font: 12px \"Arial\", sans-serif;
            text-align: left;
            text-decoration: none;
        }
        </style>
    ";
}

function select_type() {
    echo "
        <select name=type>
        <option value=none> select feature type
        <option> Tooth
        <option> Skull
        <option> Other
        </select>
    ";
}

function control_bar($features, $inst) {
    if (count($features)) {
        $str1 = "any more";
        $str2 = "another";
    } else {
        $str1 = "any";
        $str2 = "a";
    }
    echo "
        <div style=\"position:absolute; top:0px; left:0px; padding-top: 10px; padding-left: 20px; width:100%; background-color:white\">
        <input type=hidden name=bji value=\"$inst->id\">
        If you see $str2 feature:
    ";
    select_type();
    echo "
        <script>
            function clear_comment() {
                document.forms.main.comment.value = '';
            }
        </script>
        <input name=comment onfocus=\"javascript: clear_comment()\" size=30 value=\"enter comment (optional)\">
        and click on its center.
        <br>
        If you don't see $str1 features, click
        <a href=bossa_job_finished.php?bji=$inst->id>DONE</a>
        </div>
    ";
}

?>

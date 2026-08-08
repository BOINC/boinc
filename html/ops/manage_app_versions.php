<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2017 University of California
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

// web interfance for managing app versions

require_once('../inc/util_ops.inc');

function update() {
    $id = post_int("id");
    $av = BoincAppVersion::lookup_id($id);
    if (!$av) admin_error_page("no such app version");

    $n = post_str("beta", true) ? 1 : 0;
    $av->update("beta=$n");

    $n = post_str("deprecated", true) ? 1 : 0;
    $av->update("deprecated=$n");

    $n = post_int("min_core_version");
    $av->update("min_core_version=$n");

    $n = post_int("max_core_version");
    $av->update("max_core_version=$n");

    $n = post_str("plan_class");
    $av->update("plan_class='$n'");

    echo "<b>Updated app version $id.  This change will take effect when you restart the project.</b><p>";
}

function show_form($all) {
    if ($all) {
        echo "<a href=manage_app_versions.php>Don't show deprecated app versions</a>\n";
    } else {
        echo "<a href=manage_app_versions.php?all=1>Show deprecated app versions</a>\n";
    }
    $_platforms = BoincPlatform::enum("");
    foreach ($_platforms as $platform) {
        $platforms[$platform->id] = $platform;
    }

    $_apps = BoincApp::enum("");
    foreach ($_apps as $app) {
        $apps[$app->id] = $app;
    }

    start_table("");
    table_header(
        "ID #<br><small>click for details</small>",
      "Application<br><small>click for details</small>",
      "Version",
      "Platform",
      "Plan class",
      "minimum<br>client version",
      "maximum<br>client version",
      "beta?",
      "deprecated?",
      ""
    );
    $clause = $all?"true":"deprecated = 0";
    $avs = BoincAppVersion::enum(
        "$clause order by appid, platformid, plan_class, version_num"
    );
    $i = 0;
    foreach ($avs as $av) {
        // grey out deprecated versions
        //
        $f1=$f2='';
        if ($av->deprecated == 1) {
            $f1="<font color='GREY'>";
            $f2="</font>";
        }

        $all_value = $all?1:0;
        $app = $apps[$av->appid];
        // ignore app versions of deprecated apps by default
        if ($all_value == 0 && $app->deprecated == 1) {
            continue;
        }
        echo "<tr class=row$i><form action=manage_app_versions.php?all=$all_value#av_$av->id method=POST>\n";
        $i = 1-$i;
        echo "<input type=hidden name=id value=$av->id>";
        echo "  <TD>$f1 <a id='av_$av->id' href=db_action.php?table=app_version&id=$av->id>$av->id</a> $f2</TD>\n";

        $app = $apps[$av->appid];
        echo "  <TD>$f1 <a href=app_details.php?appid=$app->id>$app->name</a> $f2</TD>\n";

        echo "  <TD>$f1 $av->version_num $f2</TD>\n";

        $platform = $platforms[$av->platformid];
        echo "  <TD>$f1 $platform->name $f2</TD>\n";

        echo "  <td><input type=text name=plan_class size=12 value='$av->plan_class'></td>\n";

        $v = $av->min_core_version;
        echo "  <TD><input type='text' size='4' name=min_core_version value='$v'></TD>\n";

        $v=$av->max_core_version;
        echo "  <TD><input type='text' size='4' name=max_core_version value='$v'></TD>\n";

        $v='';
        if ($av->beta) $v=' CHECKED ';
        echo "  <TD> <input name=beta type='checkbox' $v></TD>\n";

        $v='';
        if ($av->deprecated) $v=' CHECKED ';
        echo "  <TD> <input name=deprecated type='checkbox' $v></TD>\n";

        if (!in_rops()) {
            echo "<td><input class=\"btn btn-default\" name=submit type=submit value=Update>";
        } else {
            echo "<td>&nbsp;</td>";
        }

        echo "</tr></form>";
    }
    end_table();
}


admin_page_head("Manage application versions");

if (post_str("submit", true)) {
    update();
}
$all = get_str("all", true);
show_form($all);
admin_page_tail();
?>

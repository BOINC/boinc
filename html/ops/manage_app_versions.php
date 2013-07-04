<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2013 University of California
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
    if (!$av) error_page("no such app version");

    $n = post_str("deprecated", true) ? 1 : 0;
    $av->update("deprecated=$n");

    $n = post_int("min_core_version");
    $av->update("min_core_version=$n");

    $n = post_int("max_core_version");
    $av->update("max_core_version=$n");

    echo "<b>Updated app version $id.  This change will take effect when you restart the project.</b><p>";
}

function show_form() {
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
        "ID #",
      "Application",
      "Version",
      "Platform",
      "Plan Class",
      "minimum<br>client version",
      "maximum<br>client version",
      "deprecated?",
      ""
    );
    $avs = BoincAppVersion::enum(
        "true order by appid, platformid, plan_class, version_num"
    );
    foreach ($avs as $av) {
        // grey out deprecated versions 
        //
        $f1=$f2='';
        if ($av->deprecated) {
            $f1="<font color='GREY'>";
            $f2="</font>";
        }

        echo "<tr><form action=manage_app_versions.php method=POST>\n";
        echo "<input type=hidden name=id value=$av->id>";
        echo "  <TD>$f1 $av->id $f2</TD>\n";

        $app = $apps[$av->appid];
        echo "  <TD>$f1 $app->name $f2</TD>\n";

        echo "  <TD>$f1 $av->version_num $f2</TD>\n";

        $platform = $platforms[$av->platformid];
        echo "  <TD>$f1 $platform->name $f2</TD>\n";

        echo "  <td>$f1 $av->plan_class $f2</td>\n";

        $v=$av->min_core_version;
        echo "  <TD><input type='text' size='4' name=min_core_version value='$v'></TD>\n";

        $v=$av->max_core_version;
        echo "  <TD><input type='text' size='4' name=max_core_version value='$v'></TD>\n";

        $v='';
        if($av->deprecated) $v=' CHECKED ';
        echo "  <TD> <input name=deprecated type='checkbox' $v></TD>\n";
        echo "<td><input name=submit type=submit value=Update>";

        echo "</tr></form>"; 
    }
    end_table();
}


admin_page_head("Manage application versions");

if (post_str("submit", true)) {
    update();
}
show_form();
admin_page_tail();
?>

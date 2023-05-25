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

// web interface for administering badges

require_once('../inc/util_ops.inc');

function show_form() {
    start_table('table-striped');
    table_header(
        "ID",
        "name",
        "title",
        "image URL",
        "type<br><small>0=user<br>1=team<br>optional</small>",
        "description<br><small>optional</small>",
        "level<br><small>optional</small>",
        "tags<br><small>optional</small>",
        "SQL rule<br><small>optional</small>",
        "", ""
    );

    $badges = BoincBadge::enum("");
    $i = 0;
    foreach ($badges as $badge) {
        echo "<tr class=row$i valign=top><form action=badge_admin.php method=POST>";
        $i = 1-$i;
        echo "<td>$badge->id</td>\n";
        echo "<input type=hidden name=id value=$badge->id>";
        $nu = BoincBadgeUser::count("badge_id=$badge->id");
        $nt = BoincBadgeTeam::count("badge_id=$badge->id");
        $x = "<br><small>Assigned to $nu users<br>Assigned to $nt teams</small>";
        echo "<td><input name=\"name\" value=\"$badge->name\">$x</td>\n";
        echo "<td><input name=\"title\" value=\"$badge->title\"></td>\n";
        $x = "";
        if ($badge->image_url) {
            if (strstr($badge->image_url, "http") == $badge->image_url) {
                $y = $badge->image_url;
            } else {
                $y = url_base().$badge->image_url;
            }
            $x = " <img align=right height=64 src=\"$y\">";
        }
        echo "<td><input name=\"image_url\" value=\"$badge->image_url\">$x</td>\n";
        echo "<td><input name=\"type\" size=4 value=\"$badge->type\"></td>\n";
        echo "<td><input name=\"description\" value=\"$badge->description\"></td>\n";
        echo "<td><input name=\"level\" value=\"$badge->level\"></td>\n";
        echo "<td><input name=\"tags\" value=\"$badge->tags\"></td>\n";
        echo "<td><input name=\"sql_rule\" value=\"$badge->sql_rule\"></td>\n";
        echo "<td><input class=\"btn btn-default\" type=submit name=\"update\" value=Update>\n";
        echo "<td><input class=\"btn btn-danger\" type=submit name=\"delete\" value=Delete>\n";
        echo "</form></tr>\n";
    }

    echo "<tr><form action=badge_admin.php method=POST>";
    echo "<td><br></td>\n";
    echo "<td><input name=\"name\"></td>\n";
    echo "<td><input name=\"title\"></td>\n";
    echo "<td><input name=\"image_url\"></td>\n";
    echo "<td><input name=\"type\" size=4></td>\n";
    echo "<td><input name=\"description\"></td>\n";
    echo "<td><input name=\"level\"></td>\n";
    echo "<td><input name=\"tags\"></td>\n";
    echo "<td><input name=\"sql_rule\"></td>\n";
    echo "<td colspan=2><input class=\"btn btn-primary\" type=submit name=\"add_badge\" value=\"Create badge\"></td>\n";
    echo "</form></tr>\n";

    end_table();
}

function add_badge() {
    $name = BoincDb::escape_string(post_str("name"));
    $type = post_int("type");
    $title = BoincDb::escape_string(post_str("title"));
    $description = BoincDb::escape_string(post_str("description"));
    $image_url = BoincDb::escape_string(post_str("image_url"));
    $level = BoincDb::escape_string(post_str("level"));
    $tags = BoincDb::escape_string(post_str("tags"));
    $sql_rule = BoincDb::escape_string(post_str("sql_rule"));
    $now = time();
    $id = BoincBadge::insert("(create_time, name, type, title, description, image_url, level, tags, sql_rule) values ($now, '$name', $type, '$title', '$description', '$image_url', '$level', '$tags', '$sql_rule')");
    if (!$id) {
        admin_error_page("Insert failed");
    }
}

function update_badge() {
    $id = post_int("id");
    $badge = BoincBadge::lookup_id($id);
    if (!$badge) {
        admin_error_page("no such badge");
    }
    $name = BoincDb::escape_string(post_str("name"));
    $type = post_int("type");
    $title = BoincDb::escape_string(post_str("title"));
    $description = BoincDb::escape_string(post_str("description"));
    $image_url = BoincDb::escape_string(post_str("image_url"));
    $level = BoincDb::escape_string(post_str("level"));
    $tags = BoincDb::escape_string(post_str("tags"));
    $sql_rule = BoincDb::escape_string(post_str("sql_rule"));
    $retval = $badge->update("name='$name', type=$type, title='$title', description='$description', image_url='$image_url', level='$level', tags='$tags', sql_rule='$sql_rule'");
    if (!$retval) {
        admin_error_page("update failed");
    }
}

function delete_badge() {
    $id = post_int("id");
    $badge = BoincBadge::lookup_id($id);
    if (!$badge) {
        admin_error_page("no such badge");
    }
    BoincBadgeUser::delete("badge_id=$id");
    BoincBadgeTeam::delete("badge_id=$id");
    $badge->delete();
}

if (post_str('add_badge', true)) {
    add_badge();
} else if (post_str('update', true)) {
    update_badge();
} else if (post_str('delete', true)) {
    delete_badge();
}
admin_page_head("Manage badges");
echo "
    Manage the set of badges issued by your project.
    <p>
    Badges are assigned using a PHP script;
    see
    <a href=https://github.com/BOINC/boinc/wiki/BadgeDoc>
    https://github.com/BOINC/boinc/wiki/BadgeDoc
    </a>
    <p>
    Fields marked 'optional' are not used by the default script.
";

show_form();
admin_page_tail();
?>

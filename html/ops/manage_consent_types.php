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

// web interfance for managing consent_type table

require_once('../inc/util_ops.inc');

// This function deletes a row from consent_type table.
function mct_update() {
    $cid = post_int("consent_id");
    $consent_type = BoincConsentType::lookup("consent_id = $cid");
    if ($consent_type) {
        $myname = $consent_type->shortname;
        $consent_type->delete_aux("consent_id = $cid");
        echo "<h2>Consent Type ${myname} deleted.</h2>";
    }
}

// This function adds a row to consent_type table.
function add_consenttype() {
    $shortname = BoincDb::escape_string(post_str('add_name'));
    $description = BoincDb::escape_string(post_str('add_description'));

    if (empty($shortname)) {
        admin_error_page("The new consent type must contain a short name.</font></p>");
    }
    if (empty($description)) {
        admin_error_page("The new consent type must contain a description.</font></p>");
    }

    BoincConsentType::insert(
        "(shortname, description) VALUES ('$shortname', '$description')"
    );

    echo "<h2>Consent Type added.</h2>";
}

function mct_show_form() {
    $_consenttypes = BoincConsentType::enum(null, "ORDER BY protected DESC");

    if (!in_rops()) {
        echo "<b>'Protected' consent types are defined by BOINC. You may add project-specific consent types using this form. (Unprotected consent types are defined here by this project.)</b>";
    }
    start_table("");
    table_header(
        "Name",
        "Description",
        "Protected",
        ""
    );

    foreach ($_consenttypes as $ct) {
        echo "<tr><form action=manage_consent_types.php method=POST>\n";

        echo "<input type=hidden name=consent_id value=$ct->consent_id>";
        echo "  <td>$ct->shortname</td>";

        echo "  <td>$ct->description</td>";

        echo "  <td>$ct->protected</td>";

        if (!in_rops() and !($ct->protected)) {
            echo "<td><input class=\"btn btn-default\" name=delete type=submit value=Delete>";
        } else {
            echo "<td>&nbsp;</td>";
        }

        echo "</form></tr>";
    }
    end_table();

    // Entry form to create a new consent type
    if (in_rops()) {
        return;
    }

    echo"<P>
        <h2>Add consent type</h2>
        <p>
        <form action=manage_consent_types.php method=POST>
    ";

    start_table("align='center' ");

    table_header("Name", "Description", "&nbsp;");

    echo "<TR>
            <TD> <input type='text' size='10' name='add_name' value=''> </TD>
            <TD> <input type='text' size='35' name='add_description' value=''> </TD>
            <TD align='center' >
                 <input type='submit' name='add_consenttype' value='Add Consent Type'></TD>
          </TR>\n";

    end_table();
    echo "</form><p>\n";

}


admin_page_head("Manage consent types");

if (post_str("add_consenttype", true)) {
    add_consenttype();
}
else if (post_str("delete", true)) {
    mct_update();
}

// Main display function - shows the form with consent types.
mct_show_form();

admin_page_tail();
?>

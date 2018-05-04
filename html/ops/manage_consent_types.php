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
function update() {
    $cid = post_int("consent_id");
    $consent_type = BoincConsentType::lookup("consent_id = $cid");
    $consent_type->delete_aux("consent_id = $cid");
    echo "<h2>Consent Type $cid deleted (dry-run).</h2>";
}

// This function adds a row to consent_type table.
function add_consenttype() {
    $description = BoincDb::escape_string(post_str('add_description'));

    if (empty($description)) {
        admin_error_page("The new consent type must contain a description.</font></p>");
    }

    BoincConsentType::insert(
        "(description) VALUES ('$description')"
    );

    echo "<h2>Consent Type added.</h2>";
}

function show_form() {
    $_consenttypes = BoincConsentType::enum("");

    if (!in_rops()) {
        echo "<b>You may not delete the first record (id=1) of the consent_type table.</b>";
    }
    start_table("");
    table_header(
        "ID",
        "Description",
        ""
    );

    $rowi=1;
    foreach ($_consenttypes as $ct) {
        echo "<tr class=row$rowi><form action=manage_consent_types.php method=POST>\n";

        echo "<input type=hidden name=consent_id value=$ct->consent_id>";
        echo "  <td>$ct->consent_id</td>";

        echo "  <td>$ct->description</td>";

        if (!in_rops() and ($rowi!=1)) {
            echo "<td><input class=\"btn btn-default\" name=delete type=submit value=Delete>";
        } else {
            echo "<td>&nbsp;</td>";
        }

        echo "</form></tr>";
        $rowi+=1;
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

    table_header("id", "Description", "&nbsp;");

    echo "<TR>
            <TD>(auto-incremented)</TD>
            <TD> <input type='text' size='35' name='add_description' value=''></TD>
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
    update();
}

// Main display function - shows the form with consent types.
show_form();

admin_page_tail();
?>

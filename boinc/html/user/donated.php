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

require_once("../inc/util.inc");

check_get_args(array("st"));

$st = get_str("st", true);

page_head(tra("PayPal - Transaction Completed"));

if ($st == "Completed") {
    echo "<div>".tra("Thank you for donating!")."<br>\n";
    echo tra("Your donation for has been completed.")."<br>\n";
    echo tra("Your donation will be added to the progress bar after confirmation by PayPal.")."</div>";
} else {
    echo "<strong>".tra("You have canceled your donation.")."</strong>";
}

page_tail();

?>

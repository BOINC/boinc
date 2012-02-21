<?PHP
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

// BOINC WAP functions

function wap_begin() {
    header("Content-type: text/vnd.wap.wml");
    header("Expires: Thu, 01 Jan 1970 00:00:00 GMT");

    echo "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>"
     . "<!DOCTYPE wml PUBLIC \"-//WAPFORUM//DTD WML 1.2//EN\" "
     . "\"http://www.wapforum.org/DTD/wml_1.2.xml\">"
     . "<wml><card id=\"cd1\"><p>";
}

function wap_end() {
    echo "</p></card></wml>";
}

function wap_timestamp() {
  return gmdate('j M Y G:i:s', time()) . " UTC";
}
?>

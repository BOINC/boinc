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

// utility functions for download links

function regular_link() {
}

function concierge_link($concierge, $download_url) {
    return sprintf(
        '<form action="https://boinc.berkeley.edu/concierge.php" method="post">
        <input type=hidden name=master_url value="%s">
        <input type=hidden name=user_name value="%s">
        <input type=hidden name=auth value="%s">
        <input type=hidden name=download_url value="%s">
        ',
        $concierge->master_url,
        $concierge->user_name,
        $concierge->auth,
        $download_url
    );
}

// Show download link(s) for the given platform,
// showing version and download size information.
// Show both VBox and non-VBox links if available.
// If via concierge, show links as form submit buttons
//
function download_link(
    $client_info,
    $pname,
    $button=false,
        // show as button (else table row)
    $vbox_req=false,
        // if vbox version is available, show only it
    $concierge = null
        // download via concierge w/ info given by this object
) {
    global $platforms;
    global $url_base;
    $p = $platforms[$pname];
    $v = latest_version($p);
    $file = $v['file'];
    if (array_key_exists('vbox_file', $v)) {
        $vbox_file = $v['vbox_file'];
        $vbox_version = $v['vbox_version'];
        $vbox_url = $url_base.$vbox_file;
        $vbox_path = "dl/$vbox_file";
        $vbox_size = number_format(filesize($vbox_path)/1000000, 2);
    } else {
        $vbox_file = null;
    }
    if (strstr($client_info, 'Windows NT 4') || strstr($client_info, 'Windows NT 5')) {
        $vbox_file = null;
    }
    $long_name = $p['name'];
    $num = $v['num'];
    $path = "dl/$file";
    $url = $url_base.$file;
    $dlink = "<a href=\"$url\">$file</a>";
    $s = number_format(filesize($path)/1000000, 2);

    if ($concierge) {
        $url = concierge_url($concierge, $url);
        if ($vbox_file) {
            $vbox_url = concierge_url($concierge, $vbox_url);
        }
    }

    if ($button) {
        if ($vbox_file && !$vbox_req) {
            echo tra("We recommend that you also install %1 VirtualBox %2, so your computer can work on science projects that require it.", "<a href=https://www.virtualbox.org/>", "</a>");
            echo " <a href=http://boinc.berkeley.edu/wiki/VirtualBox>";
            echo tra("Learn more about VirtualBox.");
            echo "</a>";
        }

        echo "<table width=100%><tr valign=top align=center><td>\n";

        if ($vbox_file) {
            // the current BOINC VBox doesn't work on Win 10,
            // so don't show the VBox button.
            // TODO: remove this when we make a new release
            //
            if (!strstr($client_info, "Windows NT 10")) {
                echo "
                    <a href=\"$vbox_url\" class=\"btn btn-success\"><font size=+1><u>"
                    .tra("Download BOINC + VirtualBox")
                    ."</u></font>
                    <br>"
                    // "for %s" identifies the operating system, e.g. "for Windows"
                    .sprintf(tra("for %s"), $long_name)
                    ." ($vbox_size MB)"
                    ."<br><small>"
                    .sprintf(tra("BOINC %s"), $num)
                    .", "
                    .sprintf(tra("VirtualBox %s"), $vbox_version)
                    ."</small></a>
                    &nbsp;
                    &nbsp;
                ";
            }
        }
        if (!$vbox_file || !$vbox_req) {
            echo "
                <a href=\"$url\" class=\"btn btn-info\"><font size=2><u>".tra("Download BOINC")."</u></font>
                <br>"
                .sprintf(tra("for %s"), $long_name)
                ." ($s MB)"
                ."<br><small>"
                .sprintf(tra("BOINC %s"), $num)
                ."</small></a>
            ";
        }
        echo "</td></tr></table>\n";
        if ($pname == 'linux'||$pname == 'linuxx64') {
            echo "<p>", linux_info();
        }
    } else {
        if ($vbox_file) {
            echo "<tr>
                <td class=rowlineleft>$long_name</td>
                <td class=rowline> $num (with Virtualbox $vbox_version)</td>
                <td class=rowlineright><a href=$vbox_url>Download</a> ($vbox_size MB)</td>
                </tr>
            ";
        }
        echo "<tr>
            <td class=rowlineleft>$long_name</td>
            <td class=rowline> $num</td>
            <td class=rowlineright><a href=$url>Download</a> ($s MB)</td>
            </tr>
        ";
    }
}

// take the client info string reported by web browser,
// and return best guess for platform
//
function client_info_to_platform($client_info) {
    if (strstr($client_info, 'Windows')) {
        if (strstr($client_info, 'Win64')||strstr($client_info, 'WOW64')) {
            return 'winx64';
        } else {
            return 'win';
        }
    } else if (strstr($client_info, 'Mac')) {
        if (strstr($client_info, 'PPC Mac OS X')) {
            return 'macppc';
        } else {
            return 'mac';
        }
    } else if (strstr($client_info, 'Android')) {
        // Check for Android before Linux,
        // since Android contains the Linux kernel and the
        // web browser user agent string lists Linux too.
        //
        return 'android';
    } else if (strstr($client_info, 'Linux')) {
        if (strstr($client_info, 'x86_64')) {
            return 'linuxx64';
        } else {
            return 'linux';
        }
    } else {
        return null;
    }
}

?>

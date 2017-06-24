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

// show a download link as a button or table row
//
function download_link($pname, $button=false) {
    global $platforms;
    global $url_base;
    global $client_info;
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

    if ($button) {
        if ($vbox_file) {
            echo tra("We recommend that you also install %1VirtualBox%2, so your computer can work on science projects that require it.", "<a href=https://www.virtualbox.org/>", "</a>");
            echo " <a href=http://boinc.berkeley.edu/wiki/VirtualBox>";
            echo tra("Learn more about VirtualBox.");
            echo "</a>";

            echo "<table><tr valign=top><td>\n";

            // the current BOINC VBox doesn't work on Win 10
            // remove this when we make a new release
            //
            if (!strstr($client_info, "Windows NT 10")) {
                echo "
                    <table cellpadding=10><tr valign=top><td class=button_green>
                    <a href=\"$vbox_url\"><font size=5em><u>"
                    .tra("Download BOINC + VirtualBox")
                    ."</u></font></a>
                    <br>"
                    // "for %s" identifies the operating system, e.g. "for Windows"
                    .sprintf(tra("for %s"), $long_name)
                    ." ($vbox_size MB)"
                    ."<br><span class=note>"
                    .sprintf(tra("BOINC %s"), $num)
                    .", "
                    .sprintf(tra("VirtualBox %s"), $vbox_version)
                    ."</span></td></tr>
                    </table>
                ";
            }
            echo "</td><td>\n";
        }
        echo "
            <table cellpadding=10><tr valign=top><td class=button>
            <a href=\"$url\"><font size=4><u>".tra("Download BOINC")."</u></font></a>
            <br>"
            .sprintf(tra("for %s"), $long_name)
            ." ($s MB)"
            ."<br><span class=note>"
            .sprintf(tra("BOINC %s"), $num)
            ."</span></td></tr>
            </table>
        ";
        if ($vbox_file) {
            echo "</td></tr></table>\n";
        }
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

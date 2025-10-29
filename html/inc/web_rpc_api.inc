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

// PHP interfaces to some of BOINC's Web RPCs
//
// This file is free-standing; it doesn't include other BOINC PHP files

// my PHP currently doesn't support file_get_contents(https://...)
// so do it with Curl
//
function fetch_url($url) {
    if (0) {
        return file_get_contents($url);
    } else {
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_HEADER, 0);

        $x = curl_exec($ch);
        curl_close($ch);
        //echo "curl return: $x\n";
        return $x;
    }
}

function lookup_account(
    $project_url,
    $email_addr,
    $passwd_hash
) {
    $url = $project_url."/lookup_account.php?email_addr=".urlencode($email_addr)."&passwd_hash=$passwd_hash";
    $reply = fetch_url($url);
    if (!$reply) {
        return array(null, -1, "HTTP error to $url");
    }
    $r = @simplexml_load_string($reply);
    if (!$r) {
        return array(null, -1, "Can't parse reply XML:\n$reply");
    }
    $auth = (string)$r->authenticator;
    if ($auth) {
        return array($auth, 0, null);
    } else {
        return array(null, (int)$r->error_num, (string)$r->error_msg);
    }
}

// return (authenticator, errno, errmsg)
//
function create_account(
    $project_url,
    $email_addr,
    $passwd_hash,
    $user_name
) {
    $url = $project_url."/create_account.php?email_addr=".urlencode($email_addr)."&passwd_hash=$passwd_hash&user_name=".urlencode($user_name);

    $reply = fetch_url($url);
    if (!$reply) return array(null, -1, "HTTP error to $url");

    $r = @simplexml_load_string($reply);
    if (!$r) {
        // old server code returns PHP warnings, which break XML.
        // do ad-hoc parsing instead
        //
        $auth = parse_element($reply, "<authenticator>");
        if ($auth) {
            return array($auth, 0, null);
        }
        $error_num = parse_element($reply, "<error_num>");
        $error_msg = parse_element($reply, "<error_msg>");
        if ($error_num) {
            return array(null, $error_num, $error_msg);
        }
        return array(null, -1, "Can't parse reply XML:\n$reply");
    }
    $auth = (string)$r->authenticator;
    if ($auth) {
        return array($auth, 0, null);
    } else {
        return array(null, (int)$r->error_num, (string)$r->error_msg);
    }
}

// Get info needed to show Download buttons
// that auto-attach to a particular project or account manager.
//
// I DON'T THINK THIS IS USED - OK TO DELETE
//
// args:
//      $project_url: the URL of the project or account manager
//      $rpc_key: a security token supplied by the above
//      $auth: the authenticator of the account to auto-attach to
//      $user_agent: the user agent string supplied by user's browser
//          This indicates what kind of computer they're on.
//
// return: (parsed XML reply, errno, errmsg)
//
function download_software($project_url, $rpc_key, $auth, $user_agent) {
    $url = sprintf(
        '%s/download_software.php?action=get_info&rpc_key=%s&auth=%s&user_agent=%s',
        $project_url, $rpc_key, $auth, urlencode($user_agent)
    );
    $reply = fetch_url($url);
    if (!$reply) return array(null, -1, "HTTP error to $url");
    $r = @simplexml_load_string($reply);
    if (!$r) {
        return array(null, -1, "Can't parse reply XML:\n$reply");
    }
    if ((int)$r->error_num) {
        return array(null, (int)$r->error_num, (string)$r->error_msg);
    }
    return array($r, 0, null);
}

if (0) {
    // Example usage of download_software().

    // Show a download button, which when clicked will download a
    // BOINC client that auto-attaches to a particular account
    //
    // Adapt to your web site. The following assumes Bootstrap.
    //
    function show_download_button($info, $is_vbox) {
        $desc = $is_vbox?$info->boinc_vbox:$info->boinc;
        echo sprintf('
<form action="https://boinc.berkeley.edu/concierge.php" method="post">
    <input type=hidden name=project_id value="%d">
    <input type=hidden name=token value="%s">
    <input type=hidden name=user_id value="%d">
    <input type=hidden name=filename value="%s">
    <button class="btn">
        <font size=2><u>Download BOINC%s</u></font> <br>for %s (%s MB)
',
            (int)$info->project_id,
            (string)$info->token,
            (int)$info->user_id,
            (string)$desc->filename,
            $is_vbox?" + VirtualBox":"",
            (string)$info->platform,
            (string)$desc->size_mb
        );
        if ($is_vbox) {
            echo sprintf('
    <br><small>BOINC %s, VirtualBox %s</small>
</button>
',
                (string)$desc->boinc_version,
                (string)$desc->vbox_version
            );
        } else {
            echo sprintf('
        <br><small>BOINC %s</small>
    </button>
',
                (string)$desc->boinc_version
            );
        }
        echo '</form>
';
    }

    echo "If BOINC is already installed on this computer,
         go <a href=X>here</a>; otherwise:
    ";

    list($info, $errno, $errmsg) = download_software(
        "https://boinc.berkeley.edu/test",
        "xxx",
        "xxx",
        "Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:47.0)"
    );
    if ($errno) {
        echo "Go <a href=https://boinc.berkeley.edu/download.php>here</a>
            to download BOINC, then attach to target_project
        ";
    } else {
        if ($info->boinc_vbox) {
            show_download_button($info, true);
        }
        show_download_button($info, false);
    }
}

if (0) {
    $x = create_account(
        "http://foo.bar.com/test/",
        "john@a.b.c",
        "12345678901234567890123456789012",
        "John Doe"
    );
    print_r($x);
}

if (0) {
    $x = lookup_account(
        "http://foo.bar.com/test/",
        "john@a.b.c",
        "xxx"
    );
    print_r($x);
}
?>

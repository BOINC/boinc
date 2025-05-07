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

function akismet_check($user, $post) {
    $master_url = master_url();
    $config = get_config();
    $key = parse_config($config, "<akismet_key>");
    if (!$key) {
        return true;
    }

    $master_url_enc = urlencode($master_url);
    $response = akismet_request("key=$key&blog=$master_url_enc", "rest.akismet.com", "/1.1/verify-key");
    if ("valid" == $response[1] ) {
        $post = urlencode($post);
        $ip = urlencode($_SERVER['REMOTE_ADDR']);
        $referrer = urlencode($_SERVER['HTTP_REFERER']);
        $author = urlencode($user->name);
        $useragent = urlencode($_SERVER['HTTP_USER_AGENT']);

        $request  = "blog=$master_url_enc";
        $request .= "&user_ip=$ip";
        $request .= "&user_agent=$useragent";
        $request .= "&referrer=$referrer";
        $request .= "&comment_author=$author";
        $request .= "&comment_content=$post";

        $response = akismet_request($request, "$key.rest.akismet.com", "/1.1/comment-check");

        if ("true" == $response[1]) { // Akismet says it's spam
            return false;
        } else {
            return true;
        }
    } else {
        return true; // invalid key
    }
}

function akismet_request($request, $host, $path, $port = 80) {
    $http_request  = "POST $path HTTP/1.0\r\n";
    $http_request .= "Host: $host\r\n";
    $http_request .= "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n";
    $http_request .= "Content-Length: " . strlen($request) . "\r\n";
    $http_request .= "User-Agent: BOINC | Akismet 1.1\r\n";
    $http_request .= "\r\n";
    $http_request .= $request;

    $response = '';
    if( false !== ( $fs = @fsockopen($host, $port, $errno, $errstr, 3) ) ) {
        fwrite($fs, $http_request);
        while ( !feof($fs) )
            $response .= fgets($fs, 1160); // One TCP-IP packet
        fclose($fs);
        $response = explode("\r\n\r\n", $response, 2);
    }
    return $response;
}

?>

<?php

function akismet_check($user, $post) {
    $config = get_config();
    $master_url = parse_config($config, "<master_url>");
    $key = parse_config($config, "<akismet_key>");
    if ($key == null) {
        return true; // unable to get key from config
    }
    $master_url = urlencode($master_url);
    $response = akismet_request("key=$key&blog=$master_url", "rest.akismet.com", "/1.1/verify-key");
    if ("valid" == $response[1] ) {
        $post = urlencode($post);
        $ip = urlencode($_SERVER['REMOTE_ADDR']);
        $referrer = urlencode($_SERVER['HTTP_REFERER']);
        $author = urlencode($user->getName());
        $useragent = urlencode($_SERVER['HTTP_USER_AGENT']);

        $request  = "blog=$master_url";
        $request .= "&user_ip=$ip";
        $request .= "&user_agent=$useragent";
        $request .= "&referrer=$referrer";
        $request .= "&comment_author=$author";
        $request .= "&comment_content=$post";

        $response = akismet_request($request, "$key.rest.akismet.com", "/1.1/comment-check");

        if ("true" == $response[1]) { // Akismet says it's spam
            error_page("Your post has been marked as spam by akismet.net anti-spam system. If you feel that this is wrong, please try editing your message.");
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
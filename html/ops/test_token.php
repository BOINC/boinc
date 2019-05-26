#! /usr/bin/env php
<?php
require_once("../inc/util.inc");
require_once("../inc/token.inc");
require_once("../inc/db_ops.inc");

$token = random_string();

$now = time();

BoincToken::insert("(token,userid,type,create_time, expire_time) values ('$token', 0, 'T', $now, $now+3600)");

$boincTokens = BoincToken::enum("userid=0");
foreach($boincTokens as $boincToken) {
    echo $boincToken->token . "\n";
    echo $boincToken->userid . "\n";
    echo $boincToken->type . "\n";
    echo $boincToken->create_time . "\n";
    echo $boincToken->expire_time . "\n";
}

echo "---------------\n";
$boincToken = BoincToken::lookup("userid=0");
echo $boincToken->token . "\n";
echo $boincToken->userid . "\n";
echo $boincToken->type . "\n";
echo $boincToken->create_time . "\n";
echo $boincToken->expire_time . "\n";

echo "---------------\n";
$boincToken = BoincToken::lookup_valid_token(0, $token, 'T');
if ( $boincToken != null ) {
    echo "Found valid token\n";
}

echo "---------------\n";
$boincToken = BoincToken::lookup_valid_token(0, 'notrealtoken', 'T');
if ( $boincToken == null ) {
    echo "Successfully didn't find invalid token\n";
}

echo "---------------\n";
$user = new BoincUser();
$user->id=0;
$token = create_token($user->id, TOKEN_TYPE_DELETE_ACCOUNT, TOKEN_DURATION_ONE_DAY);
if ( is_valid_token($user->id, $token, TOKEN_TYPE_DELETE_ACCOUNT) ) {
    echo "Successfully created and validated delete account token";
}

?>

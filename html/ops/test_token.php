#! /usr/bin/env php
<?php
require_once("../inc/util.inc");
require_once("../inc/db_ops.inc");

$token = random_string();

BoincToken::insert("(token,userid,type,expire_time) values ('$token', 0, 'T', unix_timestamp()+3600)");

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


?>

#!/usr/local/bin/php
<?php

require_once('../inc/util.inc');

$address = 'jbkirby@ssl.berkeley.edu';
$authenticator = random_string();
$munged = munge_email_addr($address, $authenticator);

print "Munged email address: $munged\n";

split_munged_email_addr($munged, $authenticator, $unmunged);
print "Un-munged email address: $unmunged\n";

?>

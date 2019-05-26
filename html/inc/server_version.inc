<?php

// the server code release number (major/minor/release).
// Update this when you do a server release.

// public releases have even minor #,
// and are assumed to have a branch on github.
// dev versions (e.g. master) have an odd minor #

$server_version = array(1, 1, 0);

$server_version_str = sprintf("%d.%d.%d",
    $server_version[0], $server_version[1], $server_version[2]
);

?>

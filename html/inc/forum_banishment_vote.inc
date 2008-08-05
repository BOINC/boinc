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

// TODO: convert to new DB interface; use caching to avoid
// repeated lookup for same userid

// TODO: merge vote_yes() and vote_no()  (??????)

require_once("../inc/forum_email.inc");

db_init();

function current_tally($voteid) {
    $query="select sum(yes) as ayes,count(id)-sum(yes) as nays from banishment_votes where voteid=".$voteid;
    $result = mysql_query($query);
    $foobar = mysql_fetch_object($result);
    echo "<b>Current Tally</b>    Ayes: ".$foobar->ayes." Nays: ".$foobar->nays."<p>";
    return true;
}

function vote_is_in_progress($userid) {
    // check whether a vote is already ongoing
    $now=time();
    $query="select count(id) as count from banishment_vote where userid=".$userid." and end_time>".$now;
    $result = mysql_query($query);
    if (!$result) {
        echo "Database error attempting to read banishment_vote table 1.<p>";
        echo "query: $query";
        return -1;
    }
    $foobar = mysql_fetch_object($result);
    if (!$foobar) {
        echo "Database error attempting to read banishment_vote table 2.<p>";
        return -1;
    }
    return $foobar->count;
}

function start_vote($config,$logged_in_user,$user,$category,$reason) {
    $now=time();
    $fin=$now+21600;


    if ( vote_is_in_progress($user->id) !=0 ) {
        echo "A banishment vote is already underway for this user.<p>";
        return 0;
    }
    $query="insert into banishment_vote (userid,modid,start_time,end_time) values (".$user->id.",".$logged_in_user->id.",".$now.",".$fin.")";
    $result = mysql_query($query);
    if (!$result) {
        echo "Database error attempting to insert to banishment_vote table.<p>";
        return 0;
    }

    $voteid=mysql_insert_id();
    $query="insert into banishment_votes (voteid,modid,time,yes) values (". $voteid .",". $logged_in_user->id .",". $now .",1)";
    $result = mysql_query($query);
    if (!$result) {
        echo "Database error attempting to insert to banishment_votes table.<p>";
        return 0;
    }

    $query="update forum_preferences set banished_until=".$fin." where userid=".$user->id;
    $result = mysql_query($query);

    echo "Banishment vote started.<p><p>";
    current_tally($voteid);
    return send_banish_vote_email($user, 86400*14, $reason, $now+21600);
}    

function vote_yes($config,$logged_in_user,$user) {
    $now=time();
    // Check that a vote is underway.
    if (vote_is_in_progress($user->id)<1) {
        echo "No banishment vote is underway for this user.<p><p>";
        return 0;
    }
    // Find the voteid
    $query="select id as voteid from banishment_vote where userid=".$user->id." and end_time>".$now;
    $result = mysql_query($query);
    $foobar = mysql_fetch_object($result);
    if (!$foobar) {
        echo "Database error attempting to read banishment_vote table.<p>";
        return 0;
    }
    $voteid=$foobar->voteid;
    // Check whether mod has voted already.
    $query="select count(id) as count from banishment_votes where voteid=".$voteid." and modid=".$logged_in_user->id;
    $result = mysql_query($query);
    $foobar = mysql_fetch_object($result);
    if (!$foobar) {
        echo "Database error attempting to read banishment_vote table.<p>";
        return 0;
    }
    if ($foobar->count > 0) {
        echo "You have already voted in this election.<p><p>";
        current_tally($voteid);
        return 0;
    }
    // insert the vote
    $query="insert into banishment_votes (voteid,modid,time,yes) values (" . $voteid .",". $logged_in_user->id .",". $now .",1)";
    $result = mysql_query($query);
    if (!$result) {
        echo "Database error attempting to insert to banishment_votes table.<p>";
        return 0;
    }
    echo "Vote recorded: Aye<p><p>";
    current_tally($voteid);
    return 1;
}

function vote_no($config,$logged_in_user,$user) {
    // Check that a vote is underway.
    $now=time();
    if (vote_is_in_progress($user->id)<1) {
        echo "No banishment vote is underway for this user.<p>";
        return 0;
    }
    // Find the voteid
    $query="select id as voteid from banishment_vote where userid=".$user->id." and end_time>".$now;
    $result = mysql_query($query);
    $foobar = mysql_fetch_object($result);
    if (!$foobar) {
        echo "Database error attempting to read banishment_vote table.<p>";
        return 0;
    }
    $voteid=$foobar->voteid;
    // Check whether mod has voted already.
    $query="select count(id) as count from banishment_votes where voteid=".$voteid ." and modid=".$logged_in_user->id;
    $result = mysql_query($query);
    $foobar = mysql_fetch_object($result);
    if (!$foobar) {
        echo "Database error attempting to read banishment_vote table.<p>";
        return 0;
    }
    if ($foobar->count > 0) {
        echo "You have already voted in this election.<p><p>";
        current_tally($voteid);
        return 0;
    }
    // insert the vote
    $query="insert into banishment_votes (voteid,modid,time,yes) values (" . $voteid .",". $logged_in_user->id .",". $now .",0)";
    $result = mysql_query($query);
    if (!$result) {
        echo "Database error attempting to insert to banishment_votes table.<p>";
        return 0;
    }
    echo "Vote recorded: Nay<p><p>";
    current_tally($voteid);
    return 1;
}

?>

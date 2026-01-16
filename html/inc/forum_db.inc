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

require_once("../inc/boinc_db.inc");

#[AllowDynamicProperties]
class BoincCategory {
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'category', 'BoincCategory');
    }
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('category', 'BoincCategory', $clause);
    }
    static function enum($clause=null) {
        $db = BoincDb::get();
        return $db->enum('category', 'BoincCategory', $clause);
    }
}

#[AllowDynamicProperties]
class BoincForum {
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('forum', $clause);
        if (!$ret) return null;
        return $db->insert_id();
    }
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'forum', 'BoincForum');
    }
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('forum', 'BoincForum', $clause);
    }
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('forum', 'BoincForum', $clause);
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'forum', $clause);
    }
    function delete() {
        $db = BoincDb::get();
        return $db->delete($this, 'forum');
    }
}

#[AllowDynamicProperties]
class BoincThread {
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('thread', $clause);
        if (!$ret) return null;
        return $db->insert_id();

    }
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'thread', 'BoincThread');
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'thread', $clause);
    }
    static function enum($clause="") {
        $db = BoincDb::get();
        return $db->enum('thread', 'BoincThread', $clause);
    }

    function rating() {
        return $this->score*$this->votes;
    }
    function delete() {
        $db = BoincDb::get();
        return $db->delete($this, 'thread');
    }
}

#[AllowDynamicProperties]
class BoincPost {
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('post', $clause);
        if (!$ret) return null;
        return $db->insert_id();
    }
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'post', 'BoincPost');
    }
    static function count($clause) {
        $db = BoincDb::get();
        return $db->count('post', $clause);
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'post', $clause);
    }
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('post', 'BoincPost', $clause);
    }
    static function enum_general($query) {
        $db = BoincDb::get();
        return $db->enum_general('BoincPost', $query);
    }
    function rating() {
        return $this->score*$this->votes;
    }
    function delete() {
        $db = BoincDb::get();
        return $db->delete($this, 'post');
    }
    static function delete_aux($clause) {
        $db = BoincDb::get();
        return $db->delete_aux('post', $clause);
    }
}

#[AllowDynamicProperties]
class BoincForumPrefs {
    static $cache;
    static function lookup_userid($id) {
        $db = BoincDb::get();
        return $db->lookup('forum_preferences', 'BoincForumPrefs', "userid=$id");
    }
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('forum_preferences', $clause);
    }
    static function lookup(&$user, $nocache=false) {
        if (!$user) return;
        if (isset($user->prefs)) return;
        if (!$nocache && isset(self::$cache[$user->id])) {
            $prefs = self::$cache[$user->id];
        } else {
            $prefs = self::lookup_userid($user->id);
            if (!$prefs) {
                self::insert(
                    sprintf(
                        "(userid, last_post, forum_sorting, thread_sorting, rated_posts, ignorelist, pm_notification) values (%d, 0, 0, %d, '', '', 0)",
                        $user->id, CREATE_TIME_OLD
                    )
                );
                $prefs = self::lookup_userid($user->id);
            }
            self::$cache[$user->id] = $prefs;
        }
        $user->prefs = $prefs;
    }
    function privilege($specialbit) {
         return (substr($this->special_user, $specialbit, 1) == '1');
    }
    function update($clause) {
        $db = BoincDb::get();
        $clause = "$clause where userid=$this->userid";
        return $db->update_aux('forum_preferences', $clause);
    }
    function delete() {
        $db = BoincDb::get();
        return $db->delete_aux('forum_preferences', "userid=$this->userid");
    }
    static function enum($clause=null) {
        $db = BoincDb::get();
        return $db->enum('forum_preferences', 'BoincForumPrefs', $clause);
    }
}

#[AllowDynamicProperties]
class BoincForumLogging {
    static $cache;
    static function replace($userid, $threadid, $timestamp) {
        $db = BoincDb::get();
        return $db->replace('forum_logging', "userid=$userid, threadid=$threadid, timestamp=$timestamp");
    }
    static function lookup($userid, $threadid) {
        $db = BoincDb::get();
        return $db->lookup('forum_logging', 'BoincForumLogging', "userid=$userid and threadid=$threadid");
    }
    static function lookup_cached($userid, $threadid) {
        if (isset(self::$cache[$threadid])) {
            return self::$cache[$threadid];
        }
        $x = self::lookup($userid, $threadid);
        if (!$x) {
            $x = new BoincForumLogging();
            $x->timestamp = 0;
        }
        self::$cache[$threadid] = $x;
    }
    static function delete_aux($clause) {
        $db = BoincDb::get();
        return $db->delete_aux('forum_logging', $clause);
    }
    static function cleanup() {
        // Every 28 days, delete records older than 28 days.
        // Keep track of the last time we did this in a special record
        // with userid = threadid = 0.
        // This gets called from forum_index.php
        // (i.e. each time the forum main page is loaded).
        //
        $fl = BoincForumLogging::lookup(0, 0);
        if ($fl) {
            if ($fl->timestamp<time()-MAX_FORUM_LOGGING_TIME){
                BoincForumLogging::delete_aux("timestamp<'".(time()-MAX_FORUM_LOGGING_TIME)."' and userid != 0");
                BoincForumLogging::replace(0, 0, time());
            }
        } else {
            // No cleanup timestamp found, make one
            //
            BoincForumLogging::replace(0, 0, 0);
        }
    }
}

#[AllowDynamicProperties]
class BoincSubscription {
    static function lookup($userid, $threadid) {
        $db = BoincDb::get();
        return $db->lookup('subscriptions', 'BoincSubscription', "userid=$userid and threadid=$threadid");
    }
    static function delete($userid, $threadid) {
        $db = BoincDb::get();
        return $db->delete_aux('subscriptions', "userid=$userid and threadid=$threadid");
    }
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('subscriptions', 'BoincSubscription', $clause);
    }
    static function replace($userid, $threadid) {
        $db = BoincDb::get();
        return $db->replace('subscriptions', "userid=$userid, threadid=$threadid");
    }
}

#[AllowDynamicProperties]
class BoincPostRating {
    static function lookup($userid, $postid) {
        $db = BoincDb::get();
        return $db->lookup('post_ratings', 'BoincPostRating', "user=$userid and post=$postid");
    }
    static function replace($userid, $postid, $rating) {
        $db = BoincDb::get();
        return $db->replace('post_ratings', "user=$userid, post=$postid, rating=$rating");
    }
}

#[AllowDynamicProperties]
class BoincFriend {
    static function insert($clause) {
        $db = BoincDb::get();
        return $db->insert('friend', $clause);
    }
    static function lookup($uid1, $uid2) {
        $db = BoincDb::get();
        return $db->lookup('friend', 'BoincFriend', "user_src=$uid1 and user_dest=$uid2");
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update_aux('friend', "$clause where user_src=$this->user_src and user_dest=$this->user_dest");
    }
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('friend', 'BoincFriend', $clause);
    }
    static function delete_aux($clause) {
        $db = BoincDb::get();
        return $db->delete_aux('friend', $clause);
    }
    static function delete($id1, $id2) {
        $db = BoincDb::get();
        $db->delete_aux('friend', "user_src=$id1 and user_dest=$id2");
        $db->delete_aux('friend', "user_src=$id2 and user_dest=$id1");
    }
    static function replace($clause) {
        $db = BoincDb::get();
        return $db->replace('friend', $clause);
    }
}

#[AllowDynamicProperties]
class BoincNotify {
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('notify', $clause);
        if (!$ret) return null;
        return $db->insert_id();
    }
    static function replace($clause) {
        $db = BoincDb::get();
        return $db->replace('notify', $clause);
    }
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('notify', 'BoincNotify', $clause);
    }
    static function lookup($userid, $type, $opaque) {
        $db = BoincDb::get();
        return $db->lookup('notify', 'BoincNotify', "userid=$userid and type=$type and opaque=$opaque");
    }
    function delete() {
        $db = BoincDb::get();
        return $db->delete($this, 'notify');
    }
    static function delete_aux($clause) {
        $db = BoincDb::get();
        $db->delete_aux('notify', $clause);
    }
    static function enum_general($query) {
        $db = BoincDb::get();
        return $db->enum_general('BoincNotify', $query);
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'notify', $clause);
    }
}

define ('NOTIFY_FRIEND_REQ', 1);
define ('NOTIFY_FRIEND_ACCEPT', 2);
define ('NOTIFY_PM', 3);
define ('NOTIFY_SUBSCRIBED_THREAD', 4);
define ('NOTIFY_SUBSCRIBED_FORUM', 5);

?>

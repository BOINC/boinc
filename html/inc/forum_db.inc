<?php

require_once("../inc/boinc_db.inc");

class BoincCategory {
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'category', 'BoincCategory');
    }
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('category', 'BoincCategory', $clause);
    }
}

class BoincForum {
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'forum', 'BoincForum');
    }
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('forum', 'BoincForum', $clause);
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'forum', $clause);
    }
}

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
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('thread', 'BoincThread', $clause);
    }

    function rating() {
        return $this->score*$this->votes;
    }
}

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
}

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
    static function lookup(&$user) {
        if (!$user) return;
        if (isset($user->prefs)) return;
        if (isset($cache[$user->id])) {
            $prefs = $cache[$user->id];
        } else {
            $prefs = self::lookup_userid($user->id);
            if (!$prefs) {
                self::insert("(userid, thread_sorting) values ($user->id, 8)");
                $prefs = new BoincForumPrefs();
                $prefs->userid = $user->id;
                $prefs->thread_sorting = 6;
            }
            $cache[$user->id] = $prefs;
        }
        $user->prefs = $prefs;
    }
    function privilege($specialbit) {
         return (substr($this->special_user, $specialbit,1)==1);
    }
    function update($clause) {
        $db = BoincDb::get();
        $clause = "$clause where userid=$this->userid";
        return $db->update_aux('forum_preferences', $clause);
    }
}

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
        if (isset($cache[$threadid])) {
            return $cache[$threadid];
        }
        $x = self::lookup($userid, $threadid);
        if (!$x) {
            $x = new BoincForumLogging();
            $x->timestamp = 0;
        }
        $cache[$threadid] = $x;
    }
    static function cleanup() {
        // TODO - clean this up

        $db = BoincDb::get();
        $sql = "SELECT timestamp FROM DBNAME.forum_logging where userid=0 and threadid=0";
        $result=$db->do_query($sql);
        if (mysql_num_rows($result)>0) {
            $data=mysql_fetch_object($result);
            if ($data->timestamp<time()-MAX_FORUM_LOGGING_TIME){
                $sql = "DELETE FROM DBNAME.forum_logging where timestamp<'".(time()-MAX_FORUM_LOGGING_TIME)."' and userid != 0";
                $db->do_query($sql);
                echo mysql_error();
                $sql = "REPLACE INTO DBNAME.forum_logging set userid=0, threadid=0, timestamp='".time()."'";
                $db->do_query($sql);
            }
        } else {
            //No cleanup timestamp found, make one:
            $sql = "INSERT INTO DBNAME.forum_logging set userid=0, threadid=0, timestamp=0";
            $db->do_query($sql);
            echo mysql_error();
        }
    }
}

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

?>

<?php

mysql_pconnect("localhost", "boincadm", null);
mysql_select_db("support");

function rating_insert($r) {
    $auth = mysql_real_escape_string($r->auth);
    $comment = mysql_real_escape_string($r->comment);
    $query = "insert into rating (volunteerid, rating, timestamp, auth, comment) values ($r->volunteerid, $r->rating, $r->timestamp, '$auth', '$comment')";
    return mysql_query($query);
}

function rating_lookup($r) {
    $auth = mysql_real_escape_string($r->auth);
    $query = "select * from rating where volunteerid=$r->volunteerid and auth='$auth'";
    $result = mysql_query($query);
    $rating = mysql_fetch_object($result);
    mysql_free_result($result);
    return $rating;
}

function rating_update($r) {
    $auth = mysql_real_escape_string($r->auth);
    $comment = mysql_real_escape_string($r->comment);
    $query = "update rating set rating=$r->rating, timestamp=$r->timestamp, comment='$comment' where volunteerid=$r->volunteerid and auth='$auth'";
    return mysql_query($query);
}

function ratings_get($volid) {
    $ratings = array();
    $query = "select * from rating where volunteerid=$volid order by timestamp desc";
    $result = mysql_query($query);
    while ($r = mysql_fetch_object($result)) {
        $ratings[] = $r;
    }
    mysql_free_result($result);
    return $ratings;
}

function rating_vol_auth($volid, $auth) {
    $auth = mysql_real_escape_string($auth);
    $result = mysql_query("select * from rating where volunteerid=$volid and auth='$auth'");
    $rating = mysql_fetch_object($result);
    mysql_free_result($result);
    return $rating;
}

function vol_insert($vol) {
    $password = mysql_real_escape_string($vol->password);
    $email_addr = mysql_real_escape_string($vol->email_addr);
    $country = mysql_real_escape_string($vol->country);
    $skypeid = mysql_real_escape_string($vol->skypeid);
    $lang1 = mysql_real_escape_string($vol->lang1);
    $lang2 = mysql_real_escape_string($vol->lang2);
    $specialties = mysql_real_escape_string($vol->specialties);
    $projects = mysql_real_escape_string($vol->projects);
    $availability = mysql_real_escape_string($vol->availability);

    $query = "insert into volunteer (id, create_time, name, password, email_addr, country, skypeid, lang1, lang2, specialties, projects, availability, voice_ok, text_ok, timezone) values (0, $vol->create_time, '$vol->name', '$vol->password', '$vol->email_addr', '$vol->country', '$vol->skypeid', '$vol->lang1', '$vol->lang2', '$vol->specialties', '$vol->projects', '$vol->availability', $vol->voice_ok, $vol->text_ok, $vol->timezone)";
    return mysql_query($query);
}

function vol_update($vol) {
    $password = mysql_real_escape_string($vol->password);
    $email_addr = mysql_real_escape_string($vol->email_addr);
    $country = mysql_real_escape_string($vol->country);
    $skypeid = mysql_real_escape_string($vol->skypeid);
    $lang1 = mysql_real_escape_string($vol->lang1);
    $lang2 = mysql_real_escape_string($vol->lang2);
    $specialties = mysql_real_escape_string($vol->specialties);
    $projects = mysql_real_escape_string($vol->projects);
    $availability = mysql_real_escape_string($vol->availability);

    $query = "update volunteer set name='$vol->name', password='$vol->password', email_addr='$vol->email_addr', country='$vol->country', skypeid='$vol->skypeid', lang1='$vol->lang1', lang2='$vol->lang2', specialties='$vol->specialties', projects='$vol->projects', availability='$availability', voice_ok=$vol->voice_ok, text_ok=$vol->text_ok, timezone=$vol->timezone, hide=$vol->hide where id=$vol->id";
    return mysql_query($query);
}

function vol_update_rating($vol, $old_rating, $rating) {
    $diff = $rating->rating - $old_rating->rating;
    $query = "update volunteer set rating_sum=rating_sum+$diff where id=$vol->id";
    return mysql_query($query);
}

function vol_new_rating($vol, $rating) {
    $query = "update volunteer set nratings=nratings+1, rating_sum=rating_sum+$rating where id=$vol->id";
    return mysql_query($query);
}

function vol_update_status($vol) {
    $query = "update volunteer set last_check=$vol->last_check, last_online=$vol->last_online, status=$vol->status where id=$vol->id";
    return mysql_query($query);
}

function get_vols($lang) {
    $vols = array();
    $result = mysql_query("select * from volunteer where hide=0");
    while ($vol = mysql_fetch_object($result)) {
        if ($lang) {
            if ($vol->lang1 == $lang || $vol->lang2 == $lang) {
                $vols[] = $vol;
            }
        } else {
            $vols[] = $vol;
        }
    }
    mysql_free_result($result);
    return $vols;
}

function vol_lookup($id) {
    $result = mysql_query("select * from volunteer where id=$id");
    if (!$result) return null;
    $vol = mysql_fetch_object($result);
    mysql_free_result($result);
    return $vol;
}

function vol_lookup_email($email) {
    $result = mysql_query("select * from volunteer where email_addr='$email'");
    if (!$result) return null;
    $vol = mysql_fetch_object($result);
    mysql_free_result($result);
    return $vol;
}

function vol_lookup_name($name) {
    $result = mysql_query("select * from volunteer where name='$name'");
    if (!$result) return null;
    $vol = mysql_fetch_object($result);
    mysql_free_result($result);
    return $vol;
}

function get_languages() {
    $langs = array();
    $result = mysql_query("select lang1 from volunteer");
    while ($lang = mysql_fetch_object($result)) {
        $langs[] = $lang->lang1;
    }
    mysql_free_result($result);
    $result = mysql_query("select lang2 from volunteer where lang2<>''");
    while ($lang = mysql_fetch_object($result)) {
        $langs[] = $lang->lang2;
    }
    mysql_free_result($result);
    $temp = array_unique($langs);
    $langs = array_values($temp);
    return $langs;
}

?>

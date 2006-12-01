<?php

mysql_pconnect("localhost", "boincadm", null);
mysql_select_db("support");

function rating_insert($r) {
    $query = "insert into rating (volunteerid, rating, timestamp, comment) values ($r->volunteerid, $r->rating, $r->timestamp, '$r->comment')";
    return mysql_query($query);
}

function vol_insert($vol) {
    $query = "insert into volunteer (id, create_time, name, password, email_addr, country, skypeid, lang1, lang2, specialties, projects, availability, voice_ok, text_ok, timezone) values (0, $vol->create_time, '$vol->name', '$vol->password', '$vol->email_addr', '$vol->country', '$vol->skypeid', '$vol->lang1', '$vol->lang2', '$vol->specialties', '$vol->projects', '$vol->availability', $vol->voice_ok, $vol->text_ok, $vol->timezone)";
    return mysql_query($query);
}

function vol_update($vol) {
    $query = "update volunteer set name='$vol->name', password='$vol->password', email_addr='$vol->email_addr', country='$vol->country', skypeid='$vol->skypeid', lang1='$vol->lang1', lang2='$vol->lang2', specialties='$vol->specialties', projects='$vol->projects', availability='$vol->availability', voice_ok=$vol->voice_ok, text_ok=$vol->text_ok, timezone=$vol->timezone where id=$vol->id";
    return mysql_query($query);
}

function get_vols($lang) {
    $vols = array();
    $result = mysql_query("select * from volunteer");
    while ($vol = mysql_fetch_object($result)) {
        $vols[] = $vol;
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
    $result = mysql_query("select lang2 from volunteer");
    while ($lang = mysql_fetch_object($result)) {
        $langs[] = $lang->lang2;
    }
    mysql_free_result($result);
    $temp = array_unique($langs);
    $langs = array_values($temp);
    return $langs;
}

?>

<?php

require_once('docutil.php');
require_once('../html/inc/translation.inc');

$cachefile = "cache/poll_results_$language_in_use.html";

//$cache_time = 0;
$cache_time = 3600*24;
if (file_exists($cachefile)) {
    $age = time() - filemtime($cachefile);
    //if ($age < $cache_time) {
        readfile($cachefile);
        exit();
    //}
}
set_time_limit(0);
ini_set("memory_limit", "2048M");
ob_start();
ob_implicit_flush(0);

require_once('poll.inc');
require_once('poll_data.inc');

mysql_pconnect("localhost", "boincadm", null);
mysql_select_db("poll");

$last_time = 0;

function parse_xml2($resp, &$sums) {
    $x = array();
    $xml = $resp->xml;
    $lines = explode("\n", $xml);
    foreach ($lines as $line) {
        $matches = array();
        $retval = ereg('<([^>]*)>([^<]*)', $line, $matches);
        $tag = $matches[1];
        $val = $matches[2];
        $x[$tag] = $val;
    }
    return $x;
}

function parse_xml($resp, &$sums) {
    global $last_time;
    $xml = $resp->xml;
    $lines = explode("\n", $xml);
    foreach ($lines as $line) {
        $matches = array();
        $retval = ereg('<([^>]*)>([^<]*)', $line, $matches);
        $tag = $matches[1];
        $val = $matches[2];
        if (strstr($tag, 'text')) {
            if ($val && $resp->update_time > 1197202264) {
                $val = str_replace("\\r\\n", "\n", urldecode($val));
                $val = str_replace('\\\\\\', '', $val);
                $d = gmdate("g:i A \U\T\C, F d Y", $resp->update_time);
                $sums[$tag][] = "<font size=-2>$d</font><br>$val";
            }
        } else {
            if ($val) {
                $sums[$tag][$val]++;
            }
        }
    }
}

function bar($n, $ntotal) {
    if ($ntotal==0) {
        return "<font size=-2>$n</font> &nbsp;&nbsp;";
    }
    $w = (int)(100*$n/$ntotal);
    if (!$w) $w=1;
    return "<img height=12 width=$w src=colors/6633ff.gif> <font size=-2>$n</font> &nbsp;&nbsp;";
}

function other_link($sums, $other_name, $link_text, $ntotal) {
    $y = "";
    $n = count($sums[$other_name]);
    if ($n) {
        $fname = "poll_$other_name.html";
        $b = bar($n, $ntotal);
        $y .= "$b <a href=$fname>$link_text</a>";
        $f = fopen($fname, "w");
        $old_name = str_replace(".html", ".old.html", $fname);
        fwrite($f, "<a href=$old_name>Old responses</a><hr>\n");
        foreach ($sums[$other_name] as $text) {
            if (!strstr($text, '<a')) {
                fwrite($f, $text);
                fwrite($f, "\n<hr>\n");
            }
        }
        fclose($f);
    } else {
        $y .= "$link_text";
    }
    return $y;
}

function display_choice($sums, $choice, $ntotal) {
    global $run_boinc;
    $text = $choice['text'];
    $rname = $choice['rname'];
    $radio_name = $choice['radio_name'];
    if ($rname) {
        $n = $sums[$run_boinc][$rname];
        if (!$n) $n = 0;
        $b = bar($n, $ntotal);
        $x = "$b $text\n";
    } else {
        $x = "$text\n";
    }
    $y = "";
    if ($radio_name) {
        $ntotal = 0;
        foreach($choice['options'] as $name=>$text) {
            $ntotal += $sums[$radio_name][$name];
        }
        foreach($choice['options'] as $name=>$text) {
            $n = $sums[$radio_name][$name];
            if (!$n) $n = 0;
            $b = bar($n, $ntotal);
            $y .= "$b $text<br>\n";
        }
    } else {
        $ntotal = 0;
        foreach($choice['options'] as $name=>$text) {
            $n = $sums[$name]['on'];
            if ($n > $ntotal) $ntotal = $n;
        }
        $other_name = $choice['other_name'];
        $n = count($sums[$other_name]);
        if ($n > $ntotal) $ntotal = $n;

        foreach($choice['options'] as $name=>$text) {
            $n = $sums[$name]['on'];
            if (!$n) $n = 0;
            $b = bar($n, $ntotal);
            $y .= "$b $text<br>\n";
        }
        $other_name = $choice['other_name'];
        $y .= other_link($sums, $other_name, "Other", $ntotal);
    }
    list_item2($x, $y);
}

function display_choices($sums, $choices) {
    global $run_boinc;
    $n = 0;
    $rname = $choices[0]['rname'];
    if ($rname) {
        foreach($choices as $choice) {
            $rname = $choice['rname'];
            $n += $sums[$run_boinc][$rname];
        }
    }
    foreach($choices as $choice) {
        display_choice($sums, $choice, $n);
    }
}

function display_countries($sums) {
    $y = "";
    $ntotal = 0;
    foreach ($sums['country'] as $country=>$n) {
        if ($n < 20) continue;
        $ntotal += $n;
    }
    foreach ($sums['country'] as $country=>$n) {
        if ($n < 20) continue;
        $c = urldecode($country);
        $b = bar($n, $ntotal);
        $y .= "$b $c<br>";
    }
    list_item2("Nationality", $y);
}
$sums = array();
$result = mysql_query("select * from response order by update_time");
while ($resp = mysql_fetch_object($result)) {
    parse_xml($resp, $sums);
    if (0) {
        $x = parse_xml2($resp, $sums);
        if ($x['fother_text'] == "") continue;
        if ($x['wother_text'] == "") continue;
        if ($x['nother_text'] == "") continue;
        if ($x['cother_text'] == "") continue;
        if ($x['vother_text'] == "") continue;
        echo "deleting $resp->uid\n";
        mysql_query("delete from response where uid='$resp->uid'");
    }
}
//exit;
//print_r($sums);

page_head(tra("Survey results"));
echo tra("These are the current results of the <a href=poll.php>BOINC user survey</a>.  This page is updated every hour.")."
<p>
";
list_start();
list_bar('Do you run BOINC?');
display_choices($sums, $overall_choices);
list_bar('Your participation');
display_choices($sums, $project_items);
list_bar('Your computers');
display_choices($sums, $comp_items);
list_bar('You');
display_choices($sums, $you_items);
display_countries($sums);
list_bar('Comments');
list_item2(
    "Please suggest ways that BOINC,
    and the projects that use it, could be improved:",
    other_link($sums, $improved, "Show", 0)
);

list_end();
page_tail(true);

$f = fopen($cachefile, "w");
fwrite($f, ob_get_contents());
fclose($f);

?>

<?php
require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once('../inc/sanitize_html.inc');
db_init();


function image_as_bb($text){
        /* This function depends on sanitized HTML */
    // Build some regex (should be a *lot* faster)
    $pattern = '@<img src=\"([^>]+)\">@si'; // Gives us the URL in ${1}...
    $replacement = '[img]${1}[/img]'; // Turns that URL into a hyperlink
    $text = preg_replace($pattern, $replacement, $text);

    $pattern = "@<img src='([^>]+)'>@si"; // Gives us the URL in ${1}...
    $replacement = '[img]${1}[/img]'; // Turns that URL into a hyperlink
    $text = preg_replace($pattern, $replacement, $text);

    return $text;
}

function link_as_bb($text){
        /* This function depends on sanitized HTML */
    // Build some regex (should be a *lot* faster)
    $pattern = '@<a href=\"([^>]+)\">@si'; // Gives us the URL in ${1}...
    $replacement = '[url="${1}"]'; // Turns that URL into a hyperlink
    $text = preg_replace($pattern, $replacement, $text);
    $pattern = "@<a href='([^>]+)'>@si"; // Gives us the URL in ${1}...
    $replacement = '[url="${1}"]'; // Turns that URL into a hyperlink
    $text = preg_replace($pattern, $replacement, $text);

    $pattern = "@</a>@si"; // Gives us the URL in ${1}...
    $replacement = '[/url]'; // Turns that URL into a hyperlink
    $text = preg_replace($pattern, $replacement, $text);
    return $text;
}

function formatting_as_bb($text){
        /* This function depends on sanitized HTML */
    $in[]="<b>";$out[]="[b]";
    $in[]="</b>";$out[]="[/b]";

    $in[]="<i>";$out[]="[i]";
    $in[]="</i>";$out[]="[/i]";

    $in[]="<u>";$out[]="[u]";
    $in[]="</u>";$out[]="[/u]";

    $in[]="<b>";$out[]="[b]";
    $in[]="</b>";$out[]="[/b]";

    $in[]="<ul>";$out[]="[list]";
    $in[]="</ul>";$out[]="[/list]";

    $in[]="<ol>";$out[]="[list=1]";
    $in[]="</ol>";$out[]="[/list]";

    $in[]="<pre>";$out[]="[pre]";
    $in[]="</pre>";$out[]="[/pre]";

    $in[]="<br>";$out[]="\n";

    return str_replace($in, $out, $text);
}

function fix_text($text) {
    $text = sanitize_html($text);    
    $text = image_as_bb($text);
    $text = link_as_bb($text);
    $text = formatting_as_bb($text);
    return $text;
}

function fix_posts() {
    $start_id = 0; //Set this to something else if you like
    $posts = mysql_query("select * from post where id>$start_id order by id");
    echo mysql_error();
    $i=0;
    while ($thispost = mysql_fetch_object($posts)){
        $i++; 
        if ($i%100 == 0) {                      //For every 100 posts
            echo $thispost->id.". "; flush();   // print out where we are
            usleep(200000); // Wait a short amount of time (1/5 sec) to give other DB queries a chance
        }
        
        if ($thispost->id > $start_id){
            $text = fix_text($thispost->content);
            if ($text != $thispost->content) {
                $query = "update low_priority post set content = '".mysql_escape_string($text)."' where id=".$thispost->id;
                //echo $query;
                //exit();
                mysql_query($query);
                echo mysql_error();
            }
        }
    }
}

fix_posts();

?>

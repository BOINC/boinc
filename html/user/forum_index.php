<?php
/**
 * This is the forum index
 * It shows the categories available and each of the forums that are
 * contained in those categories
 **/

require_once('../inc/forum.inc');
require_once('../inc/forum_std.inc');
db_init();

function forum_summary($forum) {
    echo '
        <tr class="row1">
        <td>
            <em>
            <a href="forum_forum.php?id='.$forum->getID().'">'.$forum->getTitle().'
            </a></em>
            <br><font size="-2">'.$forum->getDescription().'
        </td>
        <td>'.$forum->getThreadCount().'</td>
        <td>'.$forum->getPostCount().'</td>
        <td>'.time_diff_str($forum->getLastTimestamp(), time()).'</td>
    </tr>';
}

page_head(tr(FORUM_TITLE));
show_forum_title(NULL, NULL, false);
start_forum_table(array(tr(FORUM_TOPIC), tr(FORUM_THREADS), tr(FORUM_POSTS), tr(FORUM_LAST_POST)));

$categories = $mainFactory->getCategories();
$i=0;
// For each category
while ($categories[$i]){
    echo '
        <tr class="subtitle">
            <td class="category" colspan="4">'.$categories[$i]->getName().'</td>
        </tr>
    ';
    $forums = $categories[$i]->getForums();
    $ii=0;
    // Show a summary of each of the forums
    while ($forums[$ii]){
	echo forum_summary($forums[$ii]);
	$ii++;
    }
    $i++;
}

end_table();
page_tail();
flush();
cleanup_forum_log();
?>

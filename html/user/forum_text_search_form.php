<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");

page_head("Message search");

echo "
    This page lets you search for messages containing particular words.
    All Message boards and Question/answer areas will be searched.
    <br><br>

    <table cellpadding=8 bgcolor=ffffcc>
    <tr>
    <td align=right>
    Search titles:
    </td><td>
    <form action=forum_text_search_action.php>
    <input name=search_string>
    <input type=submit name=titles value=Search>
    </form>
    </td></tr>
    <tr><td align=right>
    Search messages:
    </td><td>
    <form action=forum_text_search_action.php>
    <input name=search_string>
    <input type=submit name=bodies value=Search>
    </form>
    </td></tr></table>
";

page_tail();
?>

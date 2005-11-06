<?php

require_once("docutil.php");

page_head("Participant account info");

$num = (int)($_GET["num"]);

$file = "piecharts/x_$num.html";

include($file);

page_tail();
?>

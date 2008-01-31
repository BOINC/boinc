<?php

function choose_select() {
}

function choose_xset() {
}

function show_form() {
    choose_select();
    choose_xset();
}

function show_results() {

}

if (get_str('submit', true)) {
    show_results();
} else {
    show_form();
}

?>

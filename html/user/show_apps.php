<?php

require_once('../inc/util.inc');
require_once('../inc/buda.inc');
require_once('../inc/keywords.inc');
require_once('../project/remote_apps.inc');

function show_buda_apps() {
    $apps = get_buda_apps();
    foreach ($apps as $app) {
        $desc = get_buda_desc($app);
        table_row(
            $desc->long_name,
            $desc->description,
            empty($desc->url)?'':"<a href=$desc->url>View</a>",
            kw_array_to_str($desc->sci_kw)
        );
    }
}

function main() {
    global $remote_apps;
    page_head("Apps");
    echo sprintf('%s distributes jobs for the following applications:<p>', PROJECT);
    start_table('table-striped');
    table_header(
        'Name',
        'Description',
        'Web page',
        'Science keywords'
    );
    foreach ($remote_apps as $category => $apps) {
        if (strstr($category, 'Docker')) {
            show_buda_apps();
        } else {
            foreach ($apps as $app) {
                table_row(
                    $app->long_name,
                    $app->description,
                    "<a href=$app->url>View</a>",
                    kw_array_to_str($app->sci_kw)
                );
            }
            echo '</ul>';
        }
    }
    end_table();
    page_tail();
}

main();

?>

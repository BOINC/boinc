<?php

require_once('../inc/util.inc');
require_once('../inc/json_form.inc');

function main() {
    $form_desc = json_decode(file_get_contents('form.json'));
    if (!$form_desc) {
        error_page('parse error in JSON file');
    }
    [$vals, $errs] = get_inputs($form_desc);
    if ($errs) {
        page_head('Input errors');
        echo "
            Your form input had the following errors:
            <p>
        ";
        start_table('table-striped');
        row_heading_array(['Field title', 'error']);
        foreach ($errs as [$title, $errmsg]) {
            row_array([$title, $errmsg]);
        }
        end_table();
        echo "
            Please go back, correct these, and submit again.
        ";
        page_tail();
        return;
    }
    page_head('Form values');
    start_table();
    foreach ($form_desc->fields as $field) {
        $x = $field->name;
        if (!isset($vals->$x) || $vals->$x === null) {
            $val = 'missing';
        } else {
            $val = $vals->$x;
            if (is_array($val)) {
                $val = implode(', ', $val);
            }
        }
        row2(
            $field->title,
            $val
        );
    }
    end_table();
    page_tail();
}

main();

?>

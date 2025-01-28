<?php
// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// show potentially long text with More/Less buttons

// output text; if significantly more than $nchars,
// show the first part and a More/Less button that toggles the remainder
//
// $text can't contain HTML tags (else would have to do lots of parsing)
//
function show_text_more($text, $nchars) {
    echo show_text_more_aux($test, $nchars);
}

// same, but returns text rather than outputting it
//
function show_text_more_aux($text, $nchars) {
    static $count = 0;
    if (!$text) return '';

    $n = strlen($text);
    if ($n < $nchars) {
        return $text;
    }
    // find where to break
    $b = strpos($text, ' ', $nchars);
    if ($b === false) {
        return $text;
    }

    // don't break if tail is short
    if (($n - $b) < 40) {
        return $text;
    }

    $x = '';
    if ($count == 0) {
        $x .= more_text_script();
    }
    $x .= substr($text, 0, $b);
    $x .= sprintf('<span id="dots_%d">...</span>', $count);
    $x .= sprintf('<span id="more_%d">', $count);
    $x .= substr($text, $b);
    $x .= '</span>';
    $x .= sprintf(
        '<a onclick="toggle_more(\'dots_%d\', \'more_%d\', \'btn_%d\')" id="btn_%d"> (more)</a>',
        $count, $count, $count, $count
    );
    $x .= sprintf('
        <script>document.getElementById("more_%d").style.display = "none";</script>
        ',
        $count
    );
    $count++;
    return $x;
}

function more_text_script() {
    return '
<script>
function toggle_more(dots_id, more_id, btn_id) {
    var dots = document.getElementById(dots_id);
    var moreText = document.getElementById(more_id);
    var btnText = document.getElementById(btn_id);

    if (dots.style.display === "none") {
        dots.style.display = "inline";
        btnText.innerHTML = " (more)";
        moreText.style.display = "none";
    } else {
        dots.style.display = "none";
        btnText.innerHTML = " (less)";
        moreText.style.display = "inline";
    }
}
</script>
';
}

?>

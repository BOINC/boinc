<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

function credit_to_ops($credit, &$ops, &$unit) {
    // 100 units of credit is 86400*10^9 ops
    // 1 unit of credit is 864*10^9 ops
    $ops_trillion = ($credit*864)/1000;
    $unit = "trillion";
    $ops = $ops_trillion;
    if ($ops_trillion > 1000) {
        $ops = $ops_trillion/1000;
        $unit = "quadrillion";
    }
    if ($ops_trillion > 1000000) {
        $ops = $ops_trillion/1000000;
        $unit = "quintillion";
    }
    if ($ops_trillion > 1000000000) {
        $ops = $ops_trillion/1000000000;
        $unit = "sextillion";
    }
    $ops = number_format($ops, 2);
}

function credit_string($credit, $bolden) {
    $cobbs = number_format($credit, 0);

    credit_to_ops($credit, $ops, $unit);

    if ($bolden) {
        $lbold="[[";
        $rbold="]]";
    } else {
        $lbold="";        $rbold="";
    }
    return " $lbold$cobbs Cobblestones$rbold of computation ($ops $unit floating-point operations)";
}

// show button to download certificate as PNG
//
function show_download_button() {
    echo '
        <p>
        <button id="export" style="margin-top: 5px;">Download as PNG image</button>
        <script src="html2canvas.min.js"></script>
        <script type="text/javascript">
            const $btn = document.querySelector("#export");
            const $toExport = document.querySelector("#certificate");
            const $myCanvas = document.querySelector("#canvasDiv");

            $btn.addEventListener("click", () => {
                html2canvas($toExport)
                .then(canvas => {
                    var dataURL = canvas.toDataURL("image/png");
                    downloadImage(dataURL, "certificate_image.");
                });
            });

            function downloadImage(data, filename = "untitled.png") {
                var a = document.createElement("a");
                a.href = data;
                a.download = filename;
                a.click();
            }
        </script>
    ';
}

// show buttons to share certificate on facebook and twitter
// Note: the cert pages don't use bootstrap, so we can't use show_button()
//
function show_share_buttons() {
    echo '<p>';
    $current_url = (empty($_SERVER['HTTPS']) ? 'http' : 'https') . "://$_SERVER[HTTP_HOST]$_SERVER[REQUEST_URI]";
    $cert_url = urlencode($current_url);

    // Share on Facebook
    $fb_url = 'https://www.facebook.com/sharer/sharer.php?u='.$cert_url;
    echo sprintf(
        '<button onclick="window.location.href=\'%s\';">%s</button>',
        $fb_url,
        tra('Share on Facebook')
    );
    echo '<p>';

    // Share on Twitter
    $twitter_text = urlencode("Check out my computation certificate from BOINC!");
    $tw_url = 'https://twitter.com/intent/tweet?url='.$cert_url.'&text='.$twitter_text;
    echo sprintf('<button onclick="window.location.href=\'%s\';">%s</button>',
        $tw_url,
        tra('Share on Twitter')
    );
}
?>

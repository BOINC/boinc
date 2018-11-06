<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2018 University of California
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

require_once("../inc/util.inc");
page_head(tra("Welcome to %1", PROJECT));
text_start();
echo tra("
    At this point BOINC should be installed and running on your computer;
    the system tray should have an icon like this: %1
    If not, %2get help here%3.
    ",
    "<img src=https://boinc.berkeley.edu/logo/boinc32.bmp>",
    "<a href=https://boinc.berkeley.edu/wiki/BOINC_Help>",
    "</a>"
);

echo sprintf('
    <h3>%s</h3>
    <p>
    %s',
    tra("What happens now?"),
    tra("BOINC, running in the background, will download computing jobs,
        process them, and upload the results.
        This will happen automatically; you don't have to do anything.
        This computation may cause your computer's fan to run,
        and it may slightly increase your electric bill.
    ")
);

echo sprintf('
    <h3>%s</h3>
    <p>
    %s',
    tra("How to monitor and control BOINC"),
    tra("You can monitor what BOINC is doing, and control it, using the BOINC Manager.
        Open this by double-clicking on the BOINC icon.
        Details are %1here%2.",
        "<a href=https://boinc.berkeley.edu/wiki/BOINC_Manager>",
        "</a>"
    )
);

echo sprintf('
    <h3>%s</h3>
    <p>
    %s',
    tra("Computing preferences"),
    tra("BOINC is designed to not impact
        the performance of your computer for normal use.
        If it does, you can reduce this impact by,
        for example, limiting the amount of memory BOINC uses.
        You can control many settings.
        Check this out %1here%2.",
        "<a href=prefs.php>",
        "</a>"
    )
);


echo sprintf('
    <h3>%s</h3>
    <p>
    %s',
    tra("Community"),
    tra("You can meet and communicate with other %1 participants
        in a variety of ways - take a look under the Community
        menu above.", PROJECT
    )
);

echo sprintf('
    <h3>%s</h3>
    <p>
    %s',
    tra("Other projects"),
    tra("%1 is one of many science projects using BOINC.
        Your computer is now 'attached' to %1,
        but you can attach it to other projects if you want.
        It will process jobs for all attached projects.
        %2Learn about how to do this%3.",
        PROJECT,
        "<a href=https://boinc.berkeley.edu/wiki/Choosing_and_joining_projects>",
        "</a>"
    )
);

echo "<p><p>";
echo sprintf('<a href=home.php class="btn btn-success">%s</a>
    ',
    tra('Continue to your home page')
);
text_end();
page_tail();
?>

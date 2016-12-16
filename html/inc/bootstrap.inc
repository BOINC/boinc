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

// An interface to bootstrap navbars and grids.

$fixed_navbar = false;

////////////// NAVBAR ////////////////

// call this to start the navbar.
// $brand: the text or image to show at left of navbar
// If text, put it in <a class="navbar-brand" ...
//
function navbar_start($brand, $fixed, $inverse) {
    global $fixed_navbar;
    $class = "navbar";
    if ($inverse) {
        $class .= " navbar-inverse";
    } else {
        $class .= " navbar-default";
    }
    if ($fixed) {
        $class .= " navbar-fixed-top";
        $fixed_navbar = true;
    }
    echo "<nav class=\"$class\">\n";
    echo '
  <div class="container-fluid">
    <div class="navbar-header">
      <button type="button" class="navbar-toggle" data-toggle="collapse" data-target="#myNavbar">
        <span class="icon-bar"></span>
        <span class="icon-bar"></span>
        <span class="icon-bar"></span>
      </button>
      '.$brand.'
    </div>
    <div class="collapse navbar-collapse" id="myNavbar">
      <ul class="nav navbar-nav">
    ';
}

// call this to end it
//
function navbar_end() {
    echo '
      </ul>
    </div>
  </div>
</nav>
    ';
}

// put the login/logout stuff at the right side of navbar
//
function navbar_right($user) {
    global $is_login_page;
    echo '
      </ul>
      <ul class="nav navbar-nav navbar-right">
    ';
    if (!$is_login_page) {
        if ($user) {
            echo '
                <li><a href=home.php>'.$user->name.'</a></li>
            ';
            $url_tokens = url_tokens($user->authenticator);
            echo '
                <li><a href="logout.php?'.$url_tokens.'">Log out</a></li>
            ';
        } else {
            echo '
                <li><a href="create_account_form.php">Sign Up</a></li>
                <li><a href="login_form.php">Login</a></li>
            ';
        }
    }
}

// add a dropdown menu
//
function navbar_menu($name, $items) {
    echo '
      <li class="dropdown">
        <a class="dropdown-toggle" data-toggle="dropdown" href="#">'.$name.'
        <span class="caret"></span></a>
        <ul class="dropdown-menu">
    ';
    foreach ($items as $item) {
        if (is_array($item)) {
            echo '<li><a href="'.$item[1].'">'.$item[0].'</a></li>
            ';
        } else {
            echo '<li class="dropdown-header">'.$item.'</li>
            ';
        }
    }
    echo '
        </ul>
      </li>
    ';
}

// add a single item (not menu)
//
function navbar_item($name, $url) {
    echo '<li><a href="'.$url.'">'.$name.'</a></li>
    ';
}

// A generic navbar.
// Call this from project_banner().
// If you want to customized it, copy it to your project.inc
// and give it a new name
//
function sample_navbar(
    $url_prefix,
        // prefix for links; needed for pages not in top dir
    $user,
        // logged-in user, if any
    $fixed=false,
        // if true, navbar is fixed at top of page.
        // NOTE: if you do this, you must set a global var $fixed_navbar
        // to true at compile time
        // (it needs to be set when page_head() is called).
    $inverse=false
        // white on black?
) {
    global $master_url;

    $brand = "<a class=\"navbar-brand\" href=$master_url>".PROJECT."</a>";
    navbar_start($brand, $fixed, $inverse);

    $x = array();
    if ($user) {
        $x[] = array(tra("Account"), $url_prefix."home.php");
        $x[] = array(tra("Join"), $url_prefix."join.php");
    }
    $x[] = array(tra("About %1", PROJECT), $url_prefix."about.php");
    $x[] = array(tra("Help"), $url_prefix."help.php");
    navbar_menu(tra("Project"), $x);

    if (defined('NO_COMPUTING')) {
        // this is for projects that don't do computing, e.g. BOSSA-based
        //
        navbar_menu(tra("Participate"), array(
            array(tra("Do work"), $url_prefix."bossa_apps.php"),
        ));
    } else {
        $x = array(
            array(tra("Server status"), $url_prefix."server_status.php"),
            array(tra("Statistics"), $url_prefix."stats.php"),
            array(tra("Applications"), $url_prefix."apps.php"),
        );
        if (defined('REMOTE_JOB_SUBMISSION') && REMOTE_JOB_SUBMISSION) {
            require_once("../inc/submit_db.inc");
            if ($user && BoincUserSubmit::lookup_userid($user->id)) {
                $x[] = array("Job submission", $url_prefix."submit.php");
            }
        }
        $x = array_merge($x,
            array(
                'Leader boards:',
                array(tra("Participants"), $url_prefix."top_users.php"),
                array(tra("Computers"), $url_prefix."top_hosts.php"),
                array(tra("Teams"), $url_prefix."top_teams.php"),
                array(tra("GPU models"), $url_prefix."gpu_list.php"),
                array(tra("CPU models"), $url_prefix."cpu_list.php"),
            )
        );
        navbar_menu(tra("Computing"), $x);
    }

    navbar_menu(tra("Community"), array(
        array(tra("Message boards"), $url_prefix."forum_index.php"),
        //array(tra("Questions and Answers"), $url_prefix."forum_help_desk.php"),
        array(tra("Teams"), $url_prefix."team.php", tra("create or join a team")),
        array(tra("Profiles"), $url_prefix."profile_menu.php"),
        array(tra("User search"), $url_prefix."user_search.php"),
        array(tra("User of the day"), $url_prefix."uotd.php"),
        array(tra("Certificate"), $url_prefix."cert1.php", "", "_blank"),
    ));
    navbar_menu(tra("Site"), array(
        array(tra("Site search"), $url_prefix."site_search.php"),
        array(tra("Languages"), $url_prefix."language_select.php")
    ));

    // add your own menu here if you want

    navbar_right($user);
    navbar_end();
}

// output a panel.
// $content_func is a function that generates the panel contents
//
function panel($title, $content_func) {
    echo '
    <div class="panel panel-primary">
    ';
    if ($title) {
        echo '
            <div class="panel-heading">
                <h1 class="panel-title">'.$title.'</h1>
            </div>
        ';
    }
    echo '
        <div class="panel-body">
    ';
    $content_func();
    echo '
        </div>
        </div>
    ';
}

// grid layout with a full-width row followed by two equal columns
// $top_func, $left_func, and $right_func
// are functions that generate the top, left, and right content
// $left_width is the width of left column in 1/12 units.
//
function grid($top_func, $left_func, $right_func, $left_width=6) {
    echo '
        <div class="container">
    ';
    if ($top_func) {
        echo '
            <div class="row">
            <div class="col-sm-12">
        ';
        $top_func();
        echo '
            </div>
            </div>
        ';
    }
    $right_width = 12-$left_width;
    echo '
        <div class="row">
        <div class="col-sm-'.$left_width.'">
    ';
    $left_func();
    echo '
        </div>
        <div class="col-sm-'.$right_width.'">
    ';
    $right_func();
    echo '
        </div>
        </div>
        </div>
    ';
}

function form_start($action) {
    echo '
        <form class="form-horizontal" method="get" action="'.$action.'">
    ';
}

function form_input_hidden($name, $value) {
    echo '<input type="hidden" name="'.$name.'" value="'.$value.'">
    ';
}

function form_end() {
    echo '</form>
    ';
}

function form_input_text($label, $name, $value='', $type='text', $attrs='') {
    echo '
        <div class="form-group">
            <label class="control-label col-sm-2" for="'.$name.'">'.$label.'</label>
            <div class="col-sm-10">
                <input '.$attrs.' type="'.$type.'" class="form-control" id="'.$name.'" name="'.$name.'" value="'.$value.'">
            </div>
        </div>
    ';
}

function form_select($label, $name, $items) {
    echo '
        <div class="form-group">
            <label class="control-label col-sm-2" for="'.$name.'">'.$label.'</label>
            <div class="col-sm-10">
                <select class="form-control" id="'.$name.'" name="'.$name.'">
    ';
    foreach ($items as $i) {
        echo '<option value="'.$i[0].'">'.$i[1].'</option>
        ';
    }
    echo "</select></div></div>\n";
}

// same, for multiple select.
// flags, if non-null, says which ones are selected
//
function form_select_multiple($label, $name, $items, $flags) {
    echo '
        <div class="form-group">
            <label class="control-label col-sm-2" for="'.$name.'">'.$label.'</label>
            <div class="col-sm-10">
                <select multiple class="form-control" id="'.$name.'" name="'.$name.'[]">
    ';
    $n = 0;
    foreach ($items as $i) {
        $s = ($flags && $flags[$n])?'selected':'';
        echo '<option '.$s.' value="'.$i[0].'">'.$i[1].'</option>
        ';
        $n++;
    }
    echo "</select></div></div>\n";
}

function form_submit($text) {
    echo '
        <div class="form-group">
            <div class="col-sm-offset-2 col-sm-10">
                <button type="submit" class="btn btn-success">'.$text.'</button>
            </div>
        </div>
    ';
}

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

////////////// NAVBAR ////////////////

// call this to start the navbar.
//
function navbar_start($brand, $fixed) {
    $master_url = parse_config(get_config(), "<master_url>");
    if ($fixed) {
        echo '<nav class="navbar navbar-default navbar-fixed-top">';
    } else {
        echo '<nav class="navbar navbar-default">';
    }
    echo '
  <div class="container-fluid">
    <div class="navbar-header">
      <button type="button" class="navbar-toggle" data-toggle="collapse" data-target="#myNavbar">
        <span class="icon-bar"></span>
        <span class="icon-bar"></span>
        <span class="icon-bar"></span>
      </button>
      <a class="navbar-brand" href="'.$master_url.'">'.$brand.'</a>
    </div>
    <div class="collapse navbar-collapse" id="myNavbar">
      <ul class="nav navbar-nav">
    ';
}

// call this to end it
//
function navbar_end($fixed) {
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
            $url_tokens = url_tokens($user->authenticator);
            echo '
                <li><p class="navbar-text">'.$user->name.'</p></li>
            ';
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
          echo '<li><a href="'.$item[1].'">'.$item[0].'</a></li>
        ';
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
function sample_navbar($prefix, $user, $fixed) {
    navbar_start(PROJECT, $fixed);
        // instead of PROJECT you can use an <img> or other HTML

    if ($user) {
        navbar_item(tra("Your account"), $prefix."home.php");
    }
    if (defined('NO_COMPUTING')) {
        // this is for projects that don't do computing, e.g. BOSSA-based
        //
        navbar_menu(tra("Participate"), array(
            array(tra("Do work"), $prefix."bossa_apps.php"),
        ));
    } else {
        navbar_menu(tra("Project"), array(
            array(tra("Server status"), $prefix."server_status.php"),
            array(tra("Statistics"), $prefix."stats.php"),
            array(tra("Top participants"), $prefix."top_users.php"),
            array(tra("Top computers"), $prefix."top_hosts.php"),
            array(tra("Top teams"), $prefix."top_teams.php"),
            array(tra("Top GPU models"), $prefix."gpu_list.php"),
            array(tra("Applications"), $prefix."apps.php")
        ));
    }

    navbar_menu(tra("Community"), array(
        array(tra("Message boards"), $prefix."forum_index.php"),
        //array(tra("Questions and Answers"), $prefix."forum_help_desk.php"),
        array(tra("Teams"), $prefix."team.php", tra("create or join a team")),
        array(tra("Profiles"), $prefix."profile_menu.php"),
        array(tra("User search"), $prefix."user_search.php"),
        array(tra("User of the day"), $prefix."uotd.php"),
        array(tra("Certificate"), $prefix."cert1.php", "", "_blank"),
    ));
    navbar_menu(tra("Site"), array(
        array(tra("Site search"), $prefix."site_search.php"),
        array(tra("Languages"), $prefix."language_select.php")
    ));

    // add your own menu here if you want

    navbar_right($user);
    navbar_end(true);
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

?>

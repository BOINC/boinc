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

// An interface to bootstrap navbars, grids, and forms.

$fixed_navbar = false;

if (defined('REMOTE_JOB_SUBMISSION') && REMOTE_JOB_SUBMISSION) {
    require_once("../inc/submit_db.inc");
}

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
            echo sprintf('
                <li><a href=%s%s>%s</a></li>
                ', url_base(), HOME_PAGE, $user->name
            );
            $url_tokens = url_tokens($user->authenticator);
            echo sprintf('<li><a href="%slogout.php?%s">Log out</a></li>',
                url_base(), $url_tokens
            );
        } else {
            echo sprintf('
                <li><a href="%ssignup.php">%s</a></li>
                <li><a href="%slogin_form.php">%s</a></li>
                ', url_base(),
                tra("Join"),
                url_base(),
                tra("Login")
            );
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
    $master_url = master_url();

    $brand = "<a class=\"navbar-brand\" href=$master_url>".PROJECT."</a>";
    navbar_start($brand, $fixed, $inverse);

    $x = array();
    if ($user) {
        $x[] = array(tra("Account"), $url_prefix.HOME_PAGE);
        $x[] = array(tra("Join"), $url_prefix."join.php");
        $x[] = array(tra("Preferences"), $url_prefix."prefs.php?subset=project");
    }
    $x[] = array(tra("About %1", PROJECT), $url_prefix."about.php");
    $x[] = array(tra("Help"), $url_prefix."welcome.php");
    navbar_menu(tra("Project"), $x);

    if (NO_COMPUTING) {
        // this is for projects that don't do computing, e.g. BOSSA-based
        //
        if (defined('BOSSA')) {
            navbar_menu(tra("Participate"), array(
                array(tra("Do work"), $url_prefix."bossa_apps.php"),
            ));
        }
    } else {
        $x = array(
            array(tra("Preferences"), $url_prefix."prefs.php?subset=global"),
            array(tra("Server status"), $url_prefix."server_status.php"),
            array(tra("Credit statistics"), $url_prefix."stats.php"),
            array(tra("Applications"), $url_prefix."apps.php"),
            array(tra("GPU models"), $url_prefix."gpu_list.php"),
            array(tra("CPU models"), $url_prefix."cpu_list.php"),
            array(tra("Computer types"), $url_prefix."host_stats.php"),
        );
        navbar_menu(tra("Computing"), $x);

        if (defined('REMOTE_JOB_SUBMISSION') && REMOTE_JOB_SUBMISSION) {
            $user_submit = null;
            if ($user) {
                $user_submit = BoincUserSubmit::lookup_userid($user->id);
            }
            if ($user_submit) {
                $x = [];
                $x[] = ["File sandbox", "sandbox.php"];
                $x[] = ["Submit jobs", $url_prefix."submit.php"];
                $x[] = ["Job status", $url_prefix."submit.php?action=show_user_batches"];
                if ($user_submit->manage_all) {
                    $x[] = ["Manage batches and apps", $url_prefix."submit.php?action=admin"];
                    $x[] = ["Manage permissions", $url_prefix."manage_project.php"];
                } else {
                    // if user has manage permissions for apps, show links
                    $usas = BoincUserSubmitApp::enum("user_id=$user->id");
                    foreach ($usas as $usa) {
                        if ($usa->manage) {
                            $app = BoincApp::lookup_id($usa->app_id);
                            if ($app->name == 'buda') {
                                $x[] = [
                                    'Manage BUDA apps',
                                    'buda.php'
                                ];
                            } else {
                                $x[] = [
                                    "Manage $app->name",
                                    "manage_project.php"
                                ];
                            }
                        }
                    }
                }
                navbar_menu(tra("Job submission"), $x);
            }
        }
    }

    navbar_menu(tra("Community"), array(
        array(tra("Message boards"), $url_prefix."forum_index.php"),
        //array(tra("Questions and Answers"), $url_prefix."forum_help_desk.php"),
        array(tra("Teams"), $url_prefix."team.php", tra("create or join a team")),
        array(tra("Profiles"), $url_prefix."profile_menu.php"),
        array(tra("Preferences"), $url_prefix."edit_forum_preferences_form.php"),
        array(tra("User search"), $url_prefix."user_search.php"),
        array(tra("User of the day"), $url_prefix."uotd.php"),
        array(tra("Certificate"), $url_prefix.cert_filename(), "", "_blank"),
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
function panel($title, $content_func, $class="panel-primary", $body_class="") {
    echo sprintf('<div class="panel %s">
        ', $class
    );
    if ($title) {
        echo '
            <div class="panel-heading">
                <h1 class="panel-title">'.$title.'</h1>
            </div>
        ';
    }
    echo sprintf('<div class="panel-body %s">
        ', $body_class
    );
    $content_func();
    echo '
        </div>
        </div>
    ';
}

// grid layout with a full-width row followed by two columns.
// $top_func, $left_func, and $right_func
// are functions that generate the top, left, and right content.
// $left_width is the width of left column in 1/12 units.
// $arg is passed to the functions.
//
function grid($top_func, $left_func, $right_func, $left_width=6, $arg=null) {
    echo '
        <div class="container-fluid">
    ';
    if ($top_func) {
        echo '
            <div class="row">
            <div class="col-sm-12">
        ';
        $top_func($arg);
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
    $left_func($arg);
    echo '
        </div>
        <div class="col-sm-'.$right_width.'">
    ';
    $right_func($arg);
    echo '
        </div>
        </div>
        </div>
    ';
}

// to upload files:
//  use method = POST and extra=ENCTYPE="multipart/form-data"
// to have initial focus on input field foo:
//      use extra = "name=x"
//      call forum_focus(x, foo) after defining the field
//
function form_start($action, $method='get', $extra='') {
    echo sprintf(
        '<div class="container-fluid">
        <form class="form-horizontal" method="%s" action="%s" %s>'
        ,
        $method, $action, $extra
    );
}

function form_input_hidden($name, $value) {
    echo sprintf(
        '<input type="hidden" name="%s" value="%s">',
        $name, $value
    );
}

function form_focus($form_name, $field_name) {
    echo "<script>document.$form_name.$field_name.focus()</script>\n";

}

function form_end() {
    echo '</form>
        </div>
    ';
}

define('FORM_LEFT_CLASS', 'col-sm-3');
define('FORM_LEFT_OFFSET', 'col-sm-offset-3');
define('FORM_RIGHT_CLASS', 'col-sm-9');

// just the input field
//
function form_input_text_field(
    $name, $value='', $type='text', $attrs='', $extra=''
) {
    return sprintf(
        '<input %s type="%s" class="form-control" name="%s" value="%s">%s',
        $attrs, $type, $name, $value, $extra
    );
}

// the whole row
//
function form_input_text(
    $label, $name, $value='', $type='text', $attrs='', $extra=''
) {
    echo sprintf('
        <div class="form-group">
            <label align=right class="%s">%s</label>
            <div class="%s">
                %s
            </div>
        </div>
        ',
        FORM_LEFT_CLASS, $label, FORM_RIGHT_CLASS,
        form_input_text_field($name, $value, $type, $attrs, $extra)
    );
}

// display name/value with same formatting as form
//
function form_attr($name, $value) {
    echo sprintf('
        <div class="form-group">
            <div class="%s text-right">%s</div>
            <div class="%s">%s</div>
        </div>
        ',
        FORM_LEFT_CLASS, $name, FORM_RIGHT_CLASS, $value
    );
}

function form_input_textarea($label, $name, $value='', $nrows=4) {
    echo sprintf('
        <div class="form-group">
            <label align=right class="%s" for="%s">%s</label>
            <div class="%s">
                <textarea rows="%d" class="form-control" id="%s" name="%s">%s</textarea>
            </div>
        </div>
        ',
        FORM_LEFT_CLASS, $name, $label, FORM_RIGHT_CLASS,
        $nrows, $name, $name, $value
    );
}

// $items is either a string of <option> elements,
// or an array of [value, name] pairs
//
function form_select($label, $name, $items, $selected=null) {
    echo sprintf('
        <div class="form-group">
            <label align=right class="%s" for="%s">%s</label>
            <div class="%s">
                <select class="form-control" id="%s" name="%s">
        ',
        FORM_LEFT_CLASS, $name, $label, FORM_RIGHT_CLASS, $name, $name
    );
    if (is_array($items)) {
        foreach ($items as $i) {
            echo sprintf(
                '<option %s value="%s">%s</option>',
                ($i[0]==$selected)?'selected':'',
                $i[0], $i[1]
            );
        }
    } else {
        echo $items;
    }
    echo "</select></div></div>\n";
}

// same, for multiple select.
// $selected, if non-null, is a list of selected values
//
function form_select_multiple($label, $name, $items, $selected=null, $size=0) {
    echo sprintf('
        <div class="form-group">
            <label align=right class="%s" for="%s">%s</label>
            <div class="%s">
                <select multiple class="form-control" id="%s" name="%s[]" size=%d>
        ',
        FORM_LEFT_CLASS, $name, $label, FORM_RIGHT_CLASS, $name, $name, $size
    );
    foreach ($items as $i) {
        echo sprintf(
            '<option %s value="%s">%s</option>',
            ($selected && in_array($i[0], $selected))?'selected':'',
            $i[0], $i[1]
        );
    }
    echo "</select></div></div>\n";
}

// return a list of strings for checkbox items
//
function checkbox_item_strings($items, $attrs='') {
    $x = [];
    foreach ($items as $i) {
        $x[] = sprintf('<input %s type="checkbox" name="%s" %s> %s
            ',
            $attrs, $i[0], $i[2]?"checked":"", $i[1]
        );
    }
    return $x;
}

// $items is list of (name, label, checked)
//
function form_checkboxes($label, $items, $attrs='') {
    echo sprintf('
        <div class="form-group">
            <label align=right class="%s">%s</label>
            <div class="%s">
        ',
        FORM_LEFT_CLASS, $label, FORM_RIGHT_CLASS
    );
    $x = checkbox_item_strings($items, $attrs);
    echo implode('<br>', $x);
    echo '</div>
        </div>
    ';
}

// $items is list of [value, label]
//
function form_radio_buttons(
    $label, $name, $items, $selected,
    $assign_ids=false       // assign IDs to buttons based on names
) {
    echo sprintf('
        <div class="form-group">
            <label align=right class="%s">%s</label>
            <div class="%s">
        ',
        FORM_LEFT_CLASS, $label, FORM_RIGHT_CLASS
    );
    foreach ($items as $i) {
        $checked = ($selected == $i[0])?"checked":"";
        if ($assign_ids) {
            $id = sprintf('id="%s_%s"', $name, $i[0]);
        } else {
            $id = '';
        }
        echo sprintf('<input type="radio" name="%s" value="%s" %s %s> %s <br>
            ',
            $name, $i[0], $checked, $id, $i[1]
        );
    }
    echo '</div>
        </div>
    ';
}

function form_general($label, $item) {
    echo '
        <div class="form-group">
    ';
    if (strlen($label)) {
        echo sprintf(
'           <label align=right class="%s">%s</label>
            <div class="%s">%s</div>
        ',
            FORM_LEFT_CLASS, $label, FORM_RIGHT_CLASS, $item
        );
    } else {
        echo sprintf(
'           <div class="%s %s">%s</div>
        ',
            FORM_LEFT_OFFSET, FORM_RIGHT_CLASS, $item
        );
    }
    echo '</div>
';
}

function form_submit($text, $attrs='') {
    form_general(
        "",
        sprintf(
            '<button %s type="submit" class="btn" %s>%s</button>',
            $attrs, button_style(), $text
        )
    );
}

function form_checkbox($label, $name, $checked=false) {
    echo sprintf('
        <div class="form-group">
            <label align=right class="%s">%s</label>
            <div class="%s">
        ',
        FORM_LEFT_CLASS, $label, FORM_RIGHT_CLASS
    );
    echo sprintf('
              <input type="checkbox" name="%s" %s>
           </div>
        </div>
        ', $name, $checked?"checked":""
    );
}

// 'select2' is replacement for multiple select with a nicer UI:
// https://github.com/select2/select2
//
// To use it you must use this version of page_head():

function page_head_select2($title) {
    $head_extra = '
        <link href="https://cdn.jsdelivr.net/npm/select2@4.1.0-rc.0/dist/css/select2.min.css" rel="stylesheet" />
        <script src="https://cdn.jsdelivr.net/npm/select2@4.1.0-rc.0/dist/js/select2.min.js"></script>
    ';
    page_head($title, null, false, '', $head_extra);
    echo "<script>
        $(document).ready(function() {
            $('.js-example-basic-multiple').select2({placeholder: 'click to select'});
        });
        </script>
    ";
}

// show a multi-select using select2.
// $items is a list of [value, name] pairs;
// $selected is the list of selected values.
// $extra is e.g. id=foo
//
function form_select2_multi($label, $name, $items, $selected=null, $extra='') {
    echo sprintf('
        <div class="form-group">
            <label align=right class="%s" for="%s">%s</label>
            <div class="%s">
                <select class="js-example-basic-multiple" name="%s[]" multiple="multiple" style="width: 100%%" %s>
        ',
        FORM_LEFT_CLASS, $name, $label, FORM_RIGHT_CLASS, $name, $extra
    );
    foreach ($items as $i) {
        echo sprintf(
            '<option %s value="%s">%s</option>',
            ($selected && in_array($i[0], $selected))?'selected':'',
            $i[0], $i[1]
        );
    }
    echo "</select></div></div>\n";
}

?>

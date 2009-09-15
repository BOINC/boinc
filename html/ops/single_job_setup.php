#! /usr/bin/env php
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


// configure a BOINC server to run single jobs;
// see http://boinc.berkeley.edu/trac/wiki/SingleJob
//
// Run this from project home dir.
// usage: html/ops/single_job_setup path-to-boinc_samples


ini_set('error_reporting', E_ALL);

// globals
$platform = 'i686-pc-linux-gnu';
$boinc_samples_dir = null;
$wrapper_filename = null;
$wrapper_md5 = null;
$app_name = null;
$app_id = 0;

function error($x) {
    echo "$x\n";
    exit(1);
}

function check_dirs() {
    if (!file_exists('config.xml')) {
        error("Run this from project home dir");
    }
}

function usage() {
    error("Usage: single_job_setup path-to-boinc_samples");
}

function get_includes() {
    $c = getcwd();
    chdir('html/ops');
    require_once('../inc/util_ops.inc');
    BoincDb::get();
    chdir($c);
}


// check for existence of wrapper, get its checksum
//
function check_wrapper_exists() {
    global $boinc_samples_dir, $wrapper_filename, $wrapper_md5;

    $wrapper_filename = "$boinc_samples_dir/wrapper/wrapper";
    if (!file_exists($wrapper_filename)) {
        echo "$wrapper_filename doesn't exist.\n";
        error("Make sure you've built boinc_samples.");
    }
    $wrapper_md5 = md5_file($wrapper_filename);
    if (!$wrapper_md5) {
        error("Can't read wrapper");
    }
}

// add application record in DB if not there
//
function add_application() {
    global $app_name, $app_id, $platform;

    $app_name = "single_job_$platform";
    $app = BoincApp::lookup("name='$app_name'");
    if ($app) {
        $app_id = $app->id;
    } else {
        $now = time();
        $app_id = BoincApp::insert("(create_time, name, user_friendly_name) values ($now, '$app_name','Jobs for $platform')");
        if (!$app_id) {
            error("Couldn't add application");
        }
    }
}

// create apps/appname
//
function add_apps_dir() {
    global $app_name;

    $app_dir = "apps/$app_name";
    if (!is_dir($app_dir)) {
        if (!mkdir($app_dir)) {
            error("Couldn't make app dir");
        }
    }
}

// check for apps/appname/appname_platform_N,
// find the largest such N; see if have new wrapper
// If needed, create new version, copy wrapper
//
function create_app_dir() {
    global $app_name, $app_id, $platform, $wrapper_filename;
    global $wrapper_md5;

    $i = 0;
    $latest_i = -1;
    $have_latest_wrapper = false;
    while (1) {
        $app_dir = "apps/$app_name/".$app_name."_1.".$i."_$platform";
        if (!file_exists($app_dir)) break;
        $latest_i = $i;
        $i++;
    }

    if ($latest_i >= 0) {
        $i = $latest_i;
        $app_dir = "apps/$app_name/".$app_name."_1.".$i."_$platform";
        $file = "$app_dir/".$app_name."_1.".$i."_$platform";
        $latest_md5 = md5_file($file);
        if ($latest_md5 == $wrapper_md5) {
            $have_latest_wrapper = true;
            echo "App version is current.\n";
        } else {
            echo "$latest_md5 != $wrapper_md5\n";
        }
    }

    if ($have_latest_wrapper) {
        echo "Current wrapper already installed.\n";

        // make sure they ran update_versions
        //
        $av = BoincAppVersion::lookup("appid=$app_id and version_num=$i");
        if (!$av) {
            echo "- type 'bin/update_versions', and answer 'y' to all questions.\n";
        }
    } else {
        echo "Installing current wrapper.\n";
        $i = $latest_i + 1;
        $app_dir = "apps/$app_name/".$app_name."_1.".$i."_$platform";
        $file = "$app_dir/".$app_name."_1.".$i."_$platform";
        if (!mkdir($app_dir)) {
            error("Couldn't created dir: $app_dir");
        }
        if (!copy($wrapper_filename, $file)) {
            error("Couldn't copy $wrapper_filename to $file");
        }
        chmod($file, 0750);
        echo "- type 'bin/update_versions', and answer 'y' to all questions.\n";
    }
}

// make sure daemons are in the config file
//
function check_config_file() {
    global $app_name, $platform;

    $config = file_get_contents('config.xml');
    if (!strstr($config, "single_job_assimilator")) {
        echo "- Add the following to the <daemons> section of config.xml:\n
    <daemon>
      <cmd>single_job_assimilator -app $app_name</cmd>
      <output>single_job_assimilator_$platform.out</output>
      <pid>single_job_assimilator_$platform.pid</pid>
    </daemon>
    <daemon>
      <cmd>sample_trivial_validator -app $app_name</cmd>
      <output>sample_trivial_validator_$platform.out</output>
      <pid>sample_trivial_validator_$platform.pid</pid>
    </daemon>
Then restart your project by typing
bin/stop
bin/start
    ";
    }
}

if ($argc != 2) usage();
$boinc_samples_dir = $argv[1];
check_wrapper_exists();
get_includes();
add_application();
add_apps_dir();
create_app_dir();
check_config_file();
?>

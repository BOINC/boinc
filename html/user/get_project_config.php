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

require_once("../inc/consent.inc");
require_once("../inc/util.inc");
require_once("../inc/xml.inc");
require_once("../inc/account_ownership.inc");
require_once("../inc/server_version.inc");

BoincDb::get(true);
xml_header();

// This all needs to work even when DB is down.
// So cache list of platforms in a file,
// and update it every hour if possible.
//
function show_platforms() {
    $xmlFragment = unserialize(get_cached_data(3600, "project_config_platform_xml"));
    if ($xmlFragment==false){
        $platforms = BoincDB::get()->enum_fields("platform, DBNAME.app_version, DBNAME.app", "BoincPlatform", "platform.name, platform.user_friendly_name, plan_class", "app_version.platformid = platform.id and app_version.appid = app.id and app_version.deprecated=0 and app.deprecated=0 group by platform.name, plan_class", "");
        $xmlFragment = "    <platforms>";
        foreach ($platforms as $platform){
            $xmlFragment .= "
            <platform>
                <platform_name>$platform->name</platform_name>
                <user_friendly_name>$platform->user_friendly_name</user_friendly_name>";
            if ($platform->plan_class) $xmlFragment .= "
                <plan_class>$platform->plan_class</plan_class>\n";
            $xmlFragment .= "
            </platform>";
        }
        $xmlFragment .= "\n    </platforms>\n";
        set_cached_data(3600, serialize($xmlFragment), "project_config_platform_xml");
    }
    echo $xmlFragment;
}

$config = get_config();
$master_url = master_url();
$long_name = parse_config($config, "<long_name>");

$min_passwd_length = parse_config($config, "<min_passwd_length>");
if (!$min_passwd_length) {
    $min_passwd_length = 6;
}
$disable_account_creation = parse_bool($config, "disable_account_creation");

echo "<project_config>
    <name>$long_name</name>
    <master_url>$master_url</master_url>
    <web_rpc_url_base>".secure_url_base()."</web_rpc_url_base>
";

echo "<server_version>$server_version_str</server_version>\n";

if (parse_bool($config, "account_manager")) {
    echo "    <account_manager/>\n";
}

$local_revision = @trim(file_get_contents("../../local.revision"));
if ($local_revision) {
    echo "<local_revision>$local_revision</local_revision>\n";
}

if (web_stopped()) {
    echo "
        <error_num>".ERR_PROJECT_DOWN."</error_num>
        <web_stopped>1</web_stopped>
    ";
} else {
    echo "    <web_stopped>0</web_stopped>\n";
}

if ($disable_account_creation || defined('INVITE_CODES')) {
	echo "    <account_creation_disabled/>\n";
}

if (defined('INVITE_CODES')) {
	echo "    <invite_code_required/>\n";
}

echo "    <min_passwd_length>$min_passwd_length</min_passwd_length>\n";

if (sched_stopped()) {
    echo "    <sched_stopped>1</sched_stopped>\n";
} else {
    echo "    <sched_stopped>0</sched_stopped>\n";
}

$min_core_client_version = parse_config($config, "<min_core_client_version>");
if ($min_core_client_version) {
    echo "<min_client_version>$min_core_client_version</min_client_version>\n";
}

show_platforms();

// Conditional added to allow for backwards-compatability. If a
// project has not defined the constant TERMSOFUSE_FILE, then look for
// the terms_of_use.txt file in the project base directory.
//
if (defined('TERMSOFUSE_FILE')) {
  $tou_file = TERMSOFUSE_FILE;
} else {
  $tou_file =  "../../terms_of_use.txt";
}
if (file_exists($tou_file)) {
    $terms_of_use = trim(file_get_contents($tou_file));

    if ($terms_of_use) {
        echo "    <terms_of_use>\n$terms_of_use\n</terms_of_use>\n";
    }
}

if (LDAP_HOST) {
    echo "<ldap_auth/>\n";
}

if (file_exists("../../project_keywords.xml")) {
    readfile("../../project_keywords.xml");
}

if (is_readable($account_ownership_public_key)) {
    echo "    <account_ownership_public_key>";
    echo base64_encode(file_get_contents($account_ownership_public_key));
    echo "</account_ownership_public_key>\n";
}

echo "</project_config>";

?>

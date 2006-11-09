<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");

function db_init_xml() {
    if (project_is_stopped()) {
        return -183;
    }
    $retval = db_init_aux();
    if ($retval) return -138;
    return 0;
}

function xml_header() {
    global $generating_xml;
    header('Content-type: text/xml');
    echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n";
    $generating_xml = true;
}

function xml_error($num, $msg=null) {
    if (!$msg) {
        switch($num) {
        case -112: $msg = "Invalid XML"; break;
        case -136: $msg = "Invalid ID"; break;
        case -137: $msg = "Name or email address is not unique"; break;
        case -138: $msg = "Can't access database"; break;
        case -183: $msg = "Project is temporarily offline"; break;
        case -205: $msg = "Email address has invalid syntax"; break;
        case -206: $msg = "Invalid password"; break;
        case -207: $msg = "Email address is not unique"; break;
        case -208: $msg = "Account creation is disabled"; break;
        case -209: $msg = "Invalid invitation code"; break;
	case -210: $msg = "Invalid request method"; break;
        default: "Unknown error"; break;
        }
    }
    echo "<error>
    <error_num>$num</error_num>
    <error_msg>$msg</error_msg>
</error>
";
    exit();
}

function show_host_xml($host) {
    echo "    <host>
        <id>$host->id</id>
        <create_time>$host->create_time</create_time>
        <rpc_seqno>$host->rpc_seqno</rpc_seqno>
        <host_cpid>$host->host_cpid</host_cpid>
        <total_credit>$host->total_credit</total_credit>
        <expavg_credit>$host->expavg_credit</expavg_credit>
        <expavg_time>$host->expavg_time</expavg_time>
        <domain_name>$host->domain_name</domain_name>
        <p_ncpus>$host->p_ncpus</p_ncpus>
        <p_vendor>$host->p_vendor</p_vendor>
        <p_model>$host->p_model</p_model>
        <p_fpops>$host->p_fpops</p_fpops>
        <p_iops>$host->p_iops</p_iops>
        <os_name>$host->os_name</os_name>
        <os_version>$host->os_version</os_version>
        <venue>$host->venue</venue>
    </host>
";
}

function show_user_xml($user, $show_hosts) {
    $cpid = md5($user->cross_project_id.$user->email_addr);
    echo "<user>
    <id>$user->id</id>
    <cpid>$cpid</cpid>
    <create_time>$user->create_time</create_time>
    <name>".htmlspecialchars($user->name)."</name>
    <country>$user->country</country>
    <total_credit>$user->total_credit</total_credit>
    <expavg_credit>$user->expavg_credit</expavg_credit>
    <expavg_time>$user->expavg_time</expavg_time>
    <teamid>$user->teamid</teamid>
    <url>".htmlspecialchars($user->url)."</url>
    <has_profile>$user->has_profile</has_profile>
";
    if ($show_hosts) {
        $result = mysql_query("select * from host where userid=$user->id");
        echo "   <venue>$user->venue</venue>\n";
        while ($host = mysql_fetch_object($result)) {
            show_host_xml($host);
        }
    }
echo"</user>
";
}

function show_team_member($user, $show_email) {
    $cpid = md5($user->cross_project_id.$user->email_addr);
    echo "<user>
    <id>$user->id</id>
";
    if ($show_email) {
        echo "<email_addr>$user->email_addr</email_addr>
";
    }
    echo "<cpid>$cpid</cpid>
    <create_time>$user->create_time</create_time>
    <name>".htmlspecialchars($user->name)."</name>
    <country>$user->country</country>
    <total_credit>$user->total_credit</total_credit>
    <expavg_credit>$user->expavg_credit</expavg_credit>
    <expavg_time>$user->expavg_time</expavg_time>
    <url>".htmlspecialchars($user->url)."</url>
    <has_profile>$user->has_profile</has_profile>
</user>
";
}

function show_team_xml($team) {
    echo "<team>
    <id>$team->id</id>
    <name>".htmlspecialchars($team->name)."</name>
    <country>$team->country</country>
</team>
";
}

?>

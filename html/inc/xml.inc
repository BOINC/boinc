<?php

function xml_header() {
    header('Content-type: text/xml');
    echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n";
}

function xml_error($msg) {
    xml_header();
    echo "<error>$msg</error>\n";
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
    xml_header();
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

function show_team_member($user) {
    $cpid = md5($user->cross_project_id.$user->email_addr);
    echo "<user>
    <id>$user->id</id>
    <email_addr>$user->email_addr</email_addr>
    <cpid>$cpid</cpid>
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

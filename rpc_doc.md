# GUI RPC Protocol

The GUI RPC protocol lets GUIs like the BOINC Manager communicate with the core client.

Note that the RPC server is the core client, and the RPC client is a GUI or add-on communicating with it (such as BOINC Manager). This may seem confusing but this terminology will be used on the rest of the page.

## Basic Structure

The protocol is based on XML, and it's strictly request-reply. The client sends requests to the server, and waits for a reply; the server never sends anything without getting a request from the client first. Both requests and replies are terminated with the control character `0x03`.

Self-closing tags must not have a space before the slash, or current client and server will not parse it correctly. For example, send `<authorized/>`, not `<authorized />`.

Requests are inside `<boinc_gui_rpc_request>` elements, and replies from the RPC server are inside `<boinc_gui_rpc_reply>` elements (in both cases there is a `0x03` byte after the closing tag). The current RPC server implementation doesn't require the `<boinc_gui_rpc_request>` tag, which is handy for debugging (you can connect via ​[netcat](http://netcat.sourceforge.net) and just type `<auth1/>`); however, clients must not rely on this, and must always send the `<boinc_gui_rpc_request>` root tag.

The current RPC server doesn't support pipelining of requests (pipelining means sending a batch of multiple requests without waiting for a reply, then getting all the replies together; this improves latency). For compatibility, pipelining must not be used.

To terminate the RPC session, just close the connection on the client side. Warning: some protocols have a specific "quit" command for this. The GUI RPC protocol has a `<quit/>` command, but it has a completely different purpose: telling the core client to quit!

Every request has the following structure:

```XML
<boinc_gui_rpc_request>
  <request>
  </request>
</boinc_gui_rpc_request>003
```
where `<request>` is a placeholder for one of the actual requests that are listed [later](#requests-and-replies)

Self-closing tags could also be used for requests that do not require data to be passed to the client. Their structure is the following:

```XML
<boinc_gui_rpc_request>
  <request/>
</boinc_gui_rpc_request>003
```

## Common replies

If a command requires [authentication](#authentication) but the client hasn't authenticated yet, the RPC server will reply

```XML
<boinc_gui_rpc_reply>
  <unauthorized/>
</boinc_gui_rpc_reply>003
```

The client should be prepared to receive this in reply to any command.

Successful commands usually get the reply:

```XML
<boinc_gui_rpc_reply>
    <success/>
</boinc_gui_rpc_reply>003
```

although individual commands, especially those that retrieve data, may return the requested information instead of `<success/>`.

If a command isn't successful, the reply is:

```XML
<boinc_gui_rpc_reply>
    <error>human-readable error message</error>
</boinc_gui_rpc_reply>003
```
Clients should not try to parse the error message. The current gui_rpc_client.cpp library sometimes tries to parse errors, but this is very unreliable, since the message wording can change (and has changed) between RPC server versions. (r15942 even changed "unrecognized op")


## Authentication

The RPC protocol allows the RPC client to authenticate itself using a password. Most RPC operations can only be done by an authenticated client. Some can be done without authentication, but only by a client running on the same machine.

Authentication on the RPC protocol is password-based, and negotiated with a challenge-response system. To initiate the authentication process, send an `<auth1/>` command:

```XML
<boinc_gui_rpc_request>
    <auth1/>
</boinc_gui_rpc_request>003
```

The response will be the authentication challenge:

```XML
<boinc_gui_rpc_reply>
    <nonce>1198959933.057125</nonce>
</boinc_gui_rpc_reply>003
```

The value of `nonce` is to be used as a salt with the password. It is randomly generated for each request. To calculate the response, concatenate the nonce and the password (nonce first), then calculate the MD5 hash of the result, i.e: `md5(nonce+password)`. Finally, send an `<auth2>` request with the calculated hash, in *lowercase* hexadecimal format.

```XML
<boinc_gui_rpc_request>
    <auth2>
        <nonce_hash>d41d8cd98f00b204e9800998ecf8427e</nonce_hash>
    </auth2>
</boinc_gui_rpc_request>003
```

The reply will be either `<authorized/>` or `<unauthorized/>`.

If the client hasn't authenticated yet, and it is connecting remotely (ie. not via localhost), `<auth1/>` is the only command that can be sent, and all other commands will return `<unauthorized/>`.

## Common XML elements

There are some XML elements (like `<project>`, `<result>`, `<workunit>`, and `<app>`) that are common to many command replies. Such elements will be documented in this section.

The XML responses are relatively flat, and are parsed in one pass. The relationship between XML elements is determined by what was parsed before it, instead of based on the tree hierarchy like other XML formats do. For example, `<result>` elements that come after a particular `<project>` element are results that belong to that project. They won't be inside the `<project>` element.

## Requests and Replies

These are the requests that can be issued to the client and the replies that are expected. The requests are placed between the opening and closing `<boinc_gui_rpc_request>` tags. The replies are described without the enclosing `<boinc_gui_rpc_reply>` tags.   
***

* Requests not requiring authentication 
    * [exchange_versions](#exchange_versions)
    * [get\_all\_projects_list](#get_all_projects_list)
    * [get\_cc_status](#get_cc_status)
    * [get\_disk_usage](#get_disk_usage)
    * [get\_daily\_xfer_history](#get_daily_xfer_history)
    * [get\_file_transfers](#get_file_transfers)
    * [get\_host_info](#get_host_info)
    * [get_messages](#get_messages)
    * [get\_message_count](#get_message_count)
    * [get\_newer_version](#get_newer_version)
    * [get\_notices_public](#get_notices_public)
    * [get\_old_results](#get_old_results)
    * [get\_project_status](#get_project_status)
    * [get_results](#get_results)
    * [get\_screensaver_tasks](#get_screensaver_tasks)
    * [get\_simple\_gui_info](#get_simple_gui_info)
    * [get_state](#get_state)
    * [get_statistics](#get_statistics)
 
* Requests requiring authentication
    * [File transfer operations](#file-transfer-operations)
        * [abort\_file_transfer](#abort_file_transfer)
        * [retry\_file_transfer](#retry_file_transfer)
    * [Task operations](#task-operations)
        * [abort_result](#abort_result)
        * [suspend_result](#suspend_result)
        * [resume_result](#resume_result)
    * [Project operations](#project-operations)
        * [project_reset](#project_reset)
        * [project_detach](#project_detach)
        * [project_update](#project_update)
        * [project_suspend](#project_suspend)
        * [project_resume](#project_resume)
        * [project_nomorework](#project_nomorework)
        * [project_allowmorework](#project_allowmorework)
        * [project\_detach\_when_done](#project_detach_when_done)
        * [project\_dont\_detach\_when_done](#project_dont_detach_when_done)
        * [project_attach](#project_attach)
        * [project\_attach_poll](#project_attach_poll)
        * [get\_project\_init_status](#get_project_init_status)
        * [get\_project_config](#get_project_config)
        * [get\_project\_config_poll](#get_project_config_poll)
    * [Account operations](#account-operations)
        * [create_account](#create_account)
        * [create\_account_poll](#create_account_poll)
        * [lookup_account](#lookup_account)
        * [lookup\_account_poll](#lookup_account_poll)
    * [Account Manager operations](#account-manager-operations)
        * [acct\_mgr_info](#acct_mgr_info)
        * [acct\_mgr_rpc](#acct_mgr_rpc)
        * [acct\_mgr\_rpc_poll](#acct_mgr_rpc_poll)       
    * [Global preferences operations](#global-preferences-operations)
        * [get\_global\_prefs_file](#get_global_prefs_file)
        * [get\_global\_prefs_override](#get_global_prefs_override)
        * [set\_global\_prefs_override](#set_global_prefs_override)
        * [read\_global\_prefs_override](#read_global_prefs_override)
        * [get\_global\_prefs_working](#get_global_prefs_working)
    * [Other operations](#other-operations)
        * [get_notices](#get_notices)
        * [set\_host_info](#set_host_info)
        * [run_benchmarks](#run_benchmarks)
        * [get\_proxy_settings](#get_proxy_settings)
        * [network_available](#network_available)
        * [quit](#quit)
        * [set_language](#set_language)
        * [set\_network_mode](#set_network_mode)
        * [set\_gpu_mode](#set_gpu_mode)
        * [set\_run_mode](#set_run_mode)
        * [set\_proxy_settings](#set_proxy_settings)
        * [get\_cc_config](#get_cc_config)
        * [read\_cc_config](#read_cc_config)
        * [set\_cc_config](#set_cc_config)
        * [get\_app_config](#get_app_config)
        * [set\_app_config](#set_app_config)
        * [report\_device_status](#report_device_status)
    
    
&nbsp;


### The following requests do not require local authorisation.
---
&nbsp;


#### `exchange_versions`

Used to get the version of the running core client and send the version of the request's source.

Request:

```XML
<exchange_versions>
    <major></major>
    <minor></minor>
    <release></release>
</exchange_versions>
```
The sending of the source's version is optional and a simple:

```XML
</exchange_versions>
```
would suffice to get the version of the running core client.

Reply:

```XML
<server_version>
    <major></major>
    <minor></minor>
    <release></release>
 </server_version>
```

&nbsp;

#### `get_all_projects_list`
Used to get a list of all the projects as found in the all\_projects_list.xml file.

Request:

```XML
<get_all_projects_list/>
```

Reply:

```XML
<?xml version="1.0" encoding="ISO-8859-1" ?>
<projects>
    <project>
        <name></name>
        <url></url>
        <general_area></general_area>
        <specific_area></specific_area>
        <description><![CDATA[]]></description>
        <home></home>
    <platforms>
        <name></name>
        <name></name>
        <name></name>
        .
        .
        .
    </platforms>
      <summary></summary>
    </project>
    .
    .
    .
</projects>
```

&nbsp;

#### `get_cc_status`
Show CPU/GPU/network run modes and network connection status (version 6.12+)

Request:

```XML
<get_cc_status/>
```

Reply:

```XML
<cc_status>
    <network_status></network_status>
    <ams_password_error></ams_password_error>
    <task_suspend_reason></task_suspend_reason>
    <task_mode></task_mode>
    <task_mode_perm></task_mode_perm>
    <task_mode_delay></task_mode_delay>
    <gpu_suspend_reason></gpu_suspend_reason>
    <gpu_mode></gpu_mode>
    <gpu_mode_perm></gpu_mode_perm>
    <gpu_mode_delay></gpu_mode_delay>
    <network_suspend_reason></network_suspend_reason>
    <network_mode></network_mode>
    <network_mode_perm></network_mode_perm>
    <network_mode_delay></network_mode_delay>
    <disallow_attach></disallow_attach>
    <simple_gui_only></simple_gui_only>
    <max_event_log_lines></max_event_log_lines>
</cc_status>
```
&nbsp;

#### `get_disk_usage`
Show disk usage by project

Request:

```XML
<get_disk_usage/>
```

Reply:

```XML
<disk_usage_summary>
    <project>
        <master_url></master_url>
        <disk_usage></disk_usage>
    </project>
    <d_total></d_total>
    <d_free></d_free>
    <d_boinc></d_boinc>
    <d_allowed></d_allowed>
</disk_usage_summary>
```
&nbsp;

#### `get_daily_xfer_history`
Show network traffic history of the BOINC client. Read from daily\_xfer\_history.xml

Request:
```XML
<get_daily_xfer_history/>
```

Reply:

```XML
<daily_xfers>
    <dx>
        <when></when>
        <up></up>
        <down></down>
    </dx>
    .
    .
    .
</daily_xfers>
```
&nbsp;

#### `get_file_transfers`
Show all current file transfers

Request:
```XML
<get_file_transfers/>
```

Reply:
```XML
<file_transfers>
    <file_transfer>
        <project_url></project_url>
        <project_name></project_name>
        <name></name>
        <nbytes></nbytes>
        <max_nbytes></max_nbytes>
        <status></status>
        <persistent_file_xfer>
            <num_retries></num_retries>
            <first_request_time></first_request_time>
            <next_request_time></next_request_time>
            <time_so_far></time_so_far>
            <last_bytes_xferred></last_bytes_xferred>
            <is_upload></is_upload>
        </persistent_file_xfer>
        <file_xfer>
            <bytes_xferred></bytes_xferred>
            <file_offset></file_offset>
            <xfer_speed></xfer_speed>
            <url></url>
        </file_xfer>
    </file_transfer>
    .
    .
    .
</file_transfers>
```

&nbsp;

#### `get_host_info`
Get information about host hardware and usage

Request:
```XML
<get_host_info/>
```

Reply:

```XML
<host_info>
    <timezone></timezone>
    <domain_name></domain_name>
    <ip_addr></ip_addr>
    <host_cpid></host_cpid>
    <p_ncpus></p_ncpus>
    <p_vendor></p_vendor>
    <p_model></p_model>
    <p_features></p_features>
    <p_fpops></p_fpops>
    <p_iops></p_iops>
    <p_membw></p_membw>
    <p_calculated></p_calculated>
    <p_vm_extensions_disabled></p_vm_extensions_disabled>
    <m_nbytes></m_nbytes>
    <m_cache></m_cache>
    <m_swap></m_swap>
    <d_total></d_total>
    <d_free></d_free>
    <os_name></os_name>
    <os_version></os_version>
    <product_name></product_name>
    <virtualbox_version></virtualbox_version>
    <coprocs>
        <coproc_intel_gpu>
            <count></count>
            <name></name>
            <available_ram></available_ram>
            <have_opencl></have_opencl>
            <peak_flops></peak_flops>
            <version></version>
            <coproc_opencl>
                <name></name>
                <vendor></vendor>
                <vendor_id></vendor_id>
                <available></available>
                <half_fp_config></half_fp_config>
                <single_fp_config></single_fp_config>
                <double_fp_config></double_fp_config>
                <endian_little></endian_little>
                <execution_capabilities></execution_capabilities>
                <extensions></extensions>
                <global_mem_size></global_mem_size>
                <local_mem_size></local_mem_size>
                <max_clock_frequency></max_clock_frequency>
                <max_compute_units></max_compute_units>
                <nv_compute_capability_major></nv_compute_capability_major>
                <nv_compute_capability_minor></nv_compute_capability_minor>
                <amd_simd_per_compute_unit></amd_simd_per_compute_unit>
                <amd_simd_width></amd_simd_width>
                <amd_simd_instruction_width></amd_simd_instruction_width>
                <opencl_platform_version></opencl_platform_version>
                <opencl_device_version></opencl_device_version>
                <opencl_driver_version></opencl_driver_version>
            </coproc_opencl>
        </coproc_intel_gpu>
    </coprocs>
    <opencl_cpu_prop>
        <platform_vendor></platform_vendor>
        <opencl_cpu_info>
            <name></name>
            <vendor></vendor>
            <vendor_id></vendor_id>
            <available></available>
            <half_fp_config></half_fp_config>
            <single_fp_config></single_fp_config>
            <double_fp_config></double_fp_config>
            <endian_little></endian_little>
            <execution_capabilities></execution_capabilities>
            <extensions></extensions>
            <global_mem_size></global_mem_size>
            <local_mem_size></local_mem_size>
            <max_clock_frequency></max_clock_frequency>
            <max_compute_units></max_compute_units>
            <nv_compute_capability_major></nv_compute_capability_major>
            <nv_compute_capability_minor></nv_compute_capability_minor>
            <amd_simd_per_compute_unit></amd_simd_per_compute_unit>
            <amd_simd_width></amd_simd_width>
            <amd_simd_instruction_width></amd_simd_instruction_width>
            <opencl_platform_version></opencl_platform_version>
            <opencl_device_version></opencl_device_version>
            <opencl_driver_version></opencl_driver_version>
        </opencl_cpu_info>
    </opencl_cpu_prop>
</host_info>
```
&nbsp;

#### `get_messages`
Show messages with sequence numbers beyond the given `seqno`

Request:

````XML
<get_messages>
    <seqno></seqno>
    <translatable/>
</get_messages>
````
The `<translatable/>` tag is optional.

Reply:

```XML
<msgs>
    <msg>
        <project></project>
        <pri></pri>
        <seqno></seqno>
        <body><![CDATA[]]></body>
        <time></time>
    </msg>
    .
    .
    .
</msgs>
```

&nbsp;

#### `get_message_count`

Show largest message seqno

Request:

```XML
<get_message_count/>
```

Reply:

```XML
<seqno></seqno>
```

&nbsp;

#### `get_newer_version`
Get newer version number, if any, and download url

Request:

```XML
<get_newer_version/>
```

Reply:

```XML
<newer_version></newer_version>
<download_url></download_url>
```
&nbsp;

#### `get_notices_public`
Returns only non-private notices, doesn't require authentication

Request:

```XML
<get_notices_public>
    <seqno></seqno>
</get_notices_public>
```

Reply:

```XML
<notices>
    <notice>
        <title></title>
        <description><![CDATA[]]></description>
        <create_time></create_time>
        <arrival_time></arrival_time>
        <is_private></is_private>
        <project_name></project_name>
        <category></category>
        <link></link>
        <seqno></seqno>
    </notice>
    .
    .
    .
</notices>
```
&nbsp;

#### `get_old_results`
Show old tasks

Request:

```XML
<get_old_results/>
```

Reply:

```XML
<old_results>
    <old_result>
        <project_url></project_url>
        <result_name></result_name>
        <app_name></app_name>
        <exit_status></exit_status>
        <elapsed_time></elapsed_time>
        <cpu_time></cpu_time>
        <completed_time></completed_time>
        <create_time></create_time>
    </old_result>
    .
    .
    .
</old_results>
```
&nbsp;

#### `get_project_status`
Show status of all attached projects

Request:

```XML
<get_project_status/>
```
Reply:

```XML
<projects>
    <project>
        <master_url></master_url>
        <project_name></project_name>
        <symstore></symstore>
        <user_name></user_name>
        <team_name></team_name>
        <host_venue></host_venue>
        <email_hash></email_hash>
        <cross_project_id></cross_project_id>
        <external_cpid></external_cpid>
        <cpid_time></cpid_time>
        <user_total_credit></user_total_credit>
        <user_expavg_credit></user_expavg_credit>
        <user_create_time></user_create_time>
        <rpc_seqno></rpc_seqno>
        <userid></userid>
        <teamid></teamid>
        <hostid></hostid>
        <host_total_credit></host_total_credit>
        <host_expavg_credit></host_expavg_credit>
        <host_create_time></host_create_time>
        <nrpc_failures></nrpc_failures>
        <master_fetch_failures></master_fetch_failures>
        <min_rpc_time></min_rpc_time>
        <next_rpc_time></next_rpc_time>
        <rec></rec>
        <rec_time></rec_time>
        <resource_share></resource_share>
        <desired_disk_usage></desired_disk_usage>
        <duration_correction_factor></duration_correction_factor>
        <sched_rpc_pending></sched_rpc_pending>
        <send_time_stats_log></send_time_stats_log>
        <send_job_log></send_job_log>
        <njobs_success></njobs_success>
        <njobs_error></njobs_error>
        <elapsed_time></elapsed_time>
        <last_rpc_time></last_rpc_time>
        
        (<anonymous_platform/>)
        (<master_url_fetch_pending/>)
        (<trickle_up_pending/>)
        (<send_full_workload/>)
        (<dont_use_dcf/>)
        (<non_cpu_intensive/>)
        (<verify_files_on_app_start/>)
        (<suspended_via_gui/>)
        (<dont_request_more_work/>)
        (<detach_when_done/>)
        (<ended/>)
        (<attached_via_acct_mgr/>)
        (<scheduler_rpc_in_progress/>)
        (<use_symlinks/>)        
        
        <rsc_backoff_time>
            <name>intel_gpu</name>
            <value></value>
        </rsc_backoff_time>
        <rsc_backoff_interval>
            <name></name>
            <value></value>
        </rsc_backoff_interval>
        .
        .
        .
        <no_rsc_ams></no_rsc_ams>
        <no_rsc_apps></no_rsc_apps>
        <no_rsc_pref></no_rsc_pref>
        <no_rsc_config></no_rsc_config>
        <ams_resource_share_new></ams_resource_share_new>
        <gui_urls>
            <gui_url>
                <name></name>
                <description></description>
                <url></url>
            </gui_url>
            .
            .
            .
        </gui_urls>
        <sched_priority></sched_priority>
        <project_files_downloaded_time></project_files_downloaded_time>
        <download_backoff></download_backoff>
        <upload_backoff></upload_backoff>
        <venue></venue>
        <project_dir></project_dir>
        <scheduler_url></scheduler_url>
        <code_sign_key></code_sign_key>
        <trickle_up_url></trickle_up_url>
        <cpu_ec></cpu_ec>
        <cpu_time></cpu_time>
        <gpu_ec></gpu_ec>
        <gpu_time></gpu_time>
    </project>
    .
    .
    .
</projects>
```
&nbsp;

#### `get_results`
Show tasks

Request:

```XML
<get_results>
    <active_only></active_only>
</get_results>
```

Reply:

```XML
<results>
    <result>
        <name></name>
        <wu_name></wu_name>
        <platform></platform>
        <version_num></version_num>
        <plan_class></plan_class>
        <project_url></project_url>
        <final_cpu_time></final_cpu_time>
        <final_elapsed_time></final_elapsed_time>
        <exit_status></exit_status>
        <state></state>
        <report_deadline></report_deadline>
        <received_time></received_time>
        <estimated_cpu_time_remaining></estimated_cpu_time_remaining>
        <project_suspended_via_gui/>
        <report_immediately/>
        <active_task>
            <active_task_state></active_task_state>
            <app_version_num></app_version_num>
            <slot></slot>
            <pid></pid>
            <scheduler_state></scheduler_state>
            <checkpoint_cpu_time></checkpoint_cpu_time>
            <fraction_done></fraction_done>
            <current_cpu_time></current_cpu_time>
            <elapsed_time></elapsed_time>
            <swap_size></swap_size>
            <working_set_size></working_set_size>
            <working_set_size_smoothed></working_set_size_smoothed>
            <page_fault_rate></page_fault_rate>
            <bytes_sent></bytes_sent>
            <bytes_received></bytes_received>
            <progress_rate></progress_rate>
        </active_task>
        .
        .
        .
        <resources></resources>
    </result>
    .
    .
    .
</results>
```

&nbsp;

#### `get_screensaver_tasks`
Show suspend reason and active tasks

Request:

```XML
<get_screensaver_tasks/>
```

Reply:

```XML
<handle_get_screensaver_tasks>
    <suspend_reason></suspend_reason>
    <active_task>
            <active_task_state></active_task_state>
            <app_version_num></app_version_num>
            <slot></slot>
            <pid></pid>
            <scheduler_state></scheduler_state>
            <checkpoint_cpu_time></checkpoint_cpu_time>
            <fraction_done></fraction_done>
            <current_cpu_time></current_cpu_time>
            <elapsed_time></elapsed_time>
            <swap_size></swap_size>
            <working_set_size></working_set_size>
            <working_set_size_smoothed></working_set_size_smoothed>
            <page_fault_rate></page_fault_rate>
            <bytes_sent></bytes_sent>
            <bytes_received></bytes_received>
            <progress_rate></progress_rate>
        </active_task>
        .
        .
        .
</handle_get_screensaver_tasks>
```

&nbsp;

#### `get_simple_gui_info`
Show status of projects and active tasks

Request:

```XML
<get_simple_gui_info/>
```

Reply:

```XML
<simple_gui_info>
    <project>
        .
        .
        .
    </project>
    <result>
        .
        .
        .
    </result>
</simple_gui_info>
```

&nbsp;

#### `get_state`
Get the entire state

Request:

```XML
<get_state/>
```

Reply:

```XML
<client_state>
    <host_info>
    </host_info>
    <net_stats>
    </net_stats>
    <time_stats>
    </time_stats>
    <project>
    </project>
    .
    .
    .
    <app>
    </app>
    .
    .
    .
    <app_version>
    </app_version>
    .
    .
    .
    <workunit>
    </workunit>
    .
    .
    .
    <result>
        <active_task>
        </active_task>
    </result>
    .
    .
    .
    platform_name></platform_name>
    <core_client_major_version></core_client_major_version>
    <core_client_minor_version></core_client_minor_version>
    <core_client_release></core_client_release>
    <executing_as_daemon></executing_as_daemon>
    <platform></platform>
    <global_preferences>
        .
        .
        .
    </global_preferences>
</client_state>
```

&nbsp;

#### `get_statistics`
Get statistics for the projects the client is attached to

Request:

```XML
<get_statistics/>
```

Reply:

```XML
<statistics>
<project_statistics>
        <master_url></master_url>
        <daily_statistics>
            <day></day>
            <user_total_credit></user_total_credit>
            <user_expavg_credit></user_expavg_credit>
            <host_total_credit></host_total_credit>
            <host_expavg_credit></host_expavg_credit>
        </daily_statistics>
        .
        .
        .
    </project_statistics>
    .
    .
    .
</statistics>
```
&nbsp;

---
### The following requests require local authentication


In this section the replies fall in one of three categories. For requests that retrieve data the replies depend on the kind of data that is retrieved. For _control of the client_ operations they are either:

```XML
<success/>
```

upon a successful request, or:

```XML
<error>human-readable error message</error>
``` 

upon an unsuccessful request.

If the request retrieves data the reply will be documented.
Otherwise only requests will be documented.

See also ( [Common Replies](#common-replies) )


&nbsp;

### File transfer operations
---
 
#### `abort_file_transfer`
Abort a pending file transfer

Request:

```XML
<abort_file_transfer>
    <project_url></project_url>
    <filename></filename>
</abort_file_transfer>
``` 
&nbsp;

#### `retry_file_transfer`
Retry a file transfer (Client will need temporary network access)

Request:

```XML
<retry_file_transfer>
    <project_url></project_url>
    <filename></filename>
</retry_file_transfer>
``` 


&nbsp;

### Task operations
---
#### `abort_result`
Abort a task

Request:

```XML
<abort_result>
    <project_url></project_url>
    <name></name>
</abort_result>
```
&nbsp;

#### `suspend_result`
Suspend a running task (Note: Even if a task is already suspended the request will return success)

Request:

```XML
<suspend_result>
    <project_url></project_url>
    <name></name>
</suspend_result>
```
&nbsp;

#### `resume_result`
Resume a suspended task (Note: Even if a task is already running the request will return success)

Request:

```XML
<resume_result>
    <project_url></project_url>
    <name></name>
</resume_result>
```

&nbsp;

### Project operations
---

&nbsp;
#### `project_reset`
Reset a project (Client will need temporary network access)

Request:

```XML
<project_reset>
    <project_url></project_url>
</project_reset>
```

&nbsp;
#### `project_detach`
Detach from a project

Request:

```XML
<project_detach>
    <project_url></project_url>
</project_detach>
```

&nbsp;
#### `project_update`
Update a project (Client will need temporary network access)

Request:

```XML
<project_update>
    <project_url></project_url>
</project_update>
```

&nbsp;
#### `project_suspend`
Suspend a project

Request:

```XML
<project_suspend>
    <project_url></project_url>
</project_suspend>
```

&nbsp;
#### `project_resume`
Resume a project

Request:

```XML
<project_resume>
    <project_url></project_url>
</project_resume>
```

&nbsp;
#### `project_nomorework`
Stop getting new tasks for a project

Request:

```XML
<project_nomorework>
    <project_url></project_url>
</project_nomorework>
```

&nbsp;
#### `project_allowmorework`
Receive new tasks for a project. Reverse `project_nomorework`.

Request:

```XML
<project_allowmorework>
    <project_url></project_url>
</project_allowmorework>
```

&nbsp;
#### `project_detach_when_done`
Detach from a project after all it's tasks are finished.

Request:

```XML
<project_detach_when_done>
    <project_url></project_url>
</project_detach_ahen_done>
```
&nbsp;
#### `project_dont_detach_when_done`
Don't detach from a project after all it's tasks are finished. Reverse `project_detach_when_done`

Request:

```XML
<project_dont_detach_when_done>
    <project_url></project_url>
</project_dont_detach_ahen_done>
```
&nbsp;
#### `project_attach`
Attach the client to a project. There are two kinds of requests. One using a project_init.xml file with all the necessary data and one not. (Client will need temporary network access)

Request using file:

```XML
<project_attach>
    <use_config_file/>
</project_attach>
```

Request not using file:

```XML
<project_attach>
    <project_url></project_url>
    <authenticator></authenticator>
    <project_name></project_name>
</project_attach>
```

This request is asynchronous. This means that it will reply immediately with either `<success/>` or an error concerning missing or malformated input. Another kind of possible error is: `<error>Already attached to project</error>`. 

**Note:** `<success/>` does not mean that the attachment was successful but that the request was made successfuly. 

To see if the attachment was successful the request `<project_attach_poll/>` has to be made.

&nbsp;
#### `project_attach_poll`
The aforementioned request. (Client will need temporary network access)

Request:

```XML
<project_attach_poll/>
```

Reply:

```XML
<project_attach_reply>
    [<message></message>]
    [         .         ]
    [         .         ]
    [         .         ]
    <error_num></error_num>
</project_attach_reply>
```

**Note:** A source of confusion could be the fact that the `<project_attach_poll>`request will only return errors associated with the attachment process. If a user attaches to a non existing project or uses an invalid authenticator but the attachment per se has no errors the request will return with 0 exit code. In that case the client's messages will have to be checked.

&nbsp;
#### `get_project_init_status`
Get the contents of the project_init.xml file if present

Request:

```XML
<get_project_init_status/>
```
Reply:

```XML
<get_project_init_status>
    <url></url>
    <name></name>
    <team_name></team_name>
    <setup_cookie></setup_cookie>
</get_project_init_status>
```
&nbsp;
#### `get_project_config`
Fetch the project configuration file from the specified url. Asynchronous request. (Client will need temporary network access)

Request:

```XML
<get_project_config>
    <url></url>
</get_project_config>
```
&nbsp;
#### `get_project_config_poll`
The polling call for the previous request. Not a check for the successful fetching of the file but of the successful request. (Client will need temporary network access)  

Request:

```XML
<get_project_config_poll/>
```

Reply:

Successful request:

```XML
[<project_config>]
[        .       ]
[        .       ]
[        .       ]
</project_config>
```
Unsuccessful request:

```XML
<project_config>
    <error_num></error_num>
</project_config>
```


&nbsp;

### Account operations (all require network access)
---

#### `create_account`
Create an account for a given project. Asynchronous call

Request:

```XML
<create_account>
    <url></url>
    <email_addr></email_addr>
    <passwd_hash></passwd_hash>
    <user_name></user_name>
    <team_name></team_name>
</create_account>
```

&nbsp;
#### `create_account_poll`
The polling call for the previous request

Request:

```XML
<create_account_poll/>
```

&nbsp;
#### `lookup_account`
Look for an account in a given project. Asynchronous call.

Request:

```XML
<lookup_account>
    <url></url>
    <email_addr></email_addr>
    <passwd_hash></passwd_hash>
    <ldap_auth></ldap_auth>
    <server_assigned_cookie></server_assigned_cookie>
    <server_cookie></server_cookie>
</lookup_account>
```

&nbsp;
#### `lookup_account_poll`
The polling call for the previous request. This request is designed to be used within the context of a function (e.g. inside boinccmd's `--lookup_account`) and the results to be printed by an appropriate function (e.g. `ACCOUNT_OUT::print()`) so it will not be very useful as a standalone RPC call. To get the same functionality as the above command within the context of an RPC the [lookup_account.php](https://boinc.berkeley.edu/trac/wiki/WebRpc) script can be used. 

Request:

```XML
<lookup_account_poll/>
```



&nbsp;

### Account manager operations
---

#### `acct_mgr_info`
Retrieve account manager information

Request:

```XML
<acct_mgr_info/>
``` 

Reply:

```XML
<acct_mgr_info>
   <acct_mgr_url></acct_mgr_url>
   <acct_mgr_name></acct_mgr_name>
   (<have_credentials/>)
   (<cookie_required/>)
   (<cookie_failure_url></cookie_failure_url>)
</acct_mgr_info>
```
&nbsp;

#### `acct_mgr_rpc`
Make an rpc to an account manager. (Client will need temporary network access). It has three uses. Used by the `--join_acct_mgr` command of the [boinccmd](http://boinc.berkeley.edu/wiki/Boinccmd_tool) tool to join an account manager. Used by the same tool's `--quit_acct_mgr`command with null arguments to quit an account manager. And lastly used to trigger an RPC to the current account manager.

There are two requests depending on whether there is a file with the necessary data or not.

Using said file:

```XML
<acct_mgr_rpc>
    <use_config_file/>
</acct_mgr_rpc>
```

Not using it:

```XML
<acct_mgr_rpc>
    <url></url>
    <name></name>
    <password></password>
</acct_mgr_rpc>
```

This request is asynchronous. It returns immediately with either `<success/>`or one of the following errors: `<error>bad arg</error>` or `<error>unrecognized op: act_mgr_rpc</error>`. To get the results of the request a call to `<acct_mgr_rpc_poll/>` has to be made.

&nbsp;

#### `acct_mgr_rpc_poll`
The previously mentioned call. (Client will need temporary network access)

Request:

```XML
<acct_mgr_rpc_poll/>
```

Reply:

```XML
<acct_mgr_rpc_reply>
    [<message></message>]
    <error_num></error_num>
</acct_mgr_rpc_reply>
```

&nbsp;

### Global preferences operations
---

#### `get_global_prefs_file`
Get the contents of the `global_prefs.xml` file if present

Request:

```XML
<get_global_prefs_file/>
```

Reply:

```XML
<global_preferences>
         .
         .
         .
</global_preferences>
```

&nbsp;

#### `get_global_prefs_override`
Get the contents of the `global_prefs_override.xml` file if present

Request:

```XML
<get_global_prefs_override/>
```

Reply:

```XML
<global_preferences>
         .
         .
         .
</global_preferences>
```
&nbsp;

#### `set_global_prefs_override`
Write the `global_prefs_override.xml` file

Request:

```XML
<set_global_prefs_override>
    <global_preferences>
             .
             .
             .
    </global_preferences>
</set_global_prefs_override>
```

&nbsp;

#### `read_global_prefs_override`
Read the `global_prefs_override.xml` file and set the preferences accordingly

Request:

```XML
<read_global_prefs_override/>
```

&nbsp;

#### `get_global_prefs_working`
Get the currently used `global_prefs`

Request:

```XML
<get_global_prefs_working/>
```

Reply:

```XML
<global_preferences>
         .
         .
         .
</global_preferences>
```

&nbsp;

### Other operations
---

#### `get_notices`
Returns both private and non-private notices

Request:

```XML
<get_notices>
    <seqno></seqno>
</get_notices>
```

Reply:

```XML
<notices>
    <notice>
        <title></title>
        <description><![CDATA[]]></description>
        <create_time></create_time>
        <arrival_time></arrival_time>
        <is_private></is_private>
        <project_name></project_name>
        <category></category>
        <link></link>
        <seqno></seqno>
    </notice>
    .
    .
    .
</notices>
```
&nbsp;

#### `set_host_info`
Set the `product_name` field of `host_info`

Request:

```XML
<set_host_info>
    <host_info>
        <product_name></product_name>
    </host_info>
</set_host_info>
```
&nbsp;

#### `run_benchmarks`
Run benchmarks 

Request:

```XML
<run_benchmarks/>
```
&nbsp;

#### `get_proxy_settings`
Get the proxy settings

Request:

```XML
<get_proxy_settings/>
```

Reply:

```XML
<proxy_info>
    [<use_http_proxy/>]
    [<use_socks_proxy/>]
    [<use_http_auth/>]
    <socks_server_name></socks_server_name>
    <socks_server_port></socks_server_port>
    <http_server_name></http_server_name>
    <http_server_port></http_server_port>
    <socks5_user_name></socks5_user_name>
    <socks5_user_passwd></socks5_user_passwd>
    <socks5_remote_dns></socks5_remote_dns>
    <http_user_name></http_user_name>
    <http_user_passwd></http_user_passwd>
    <no_autodetect></no_autodetect>
    <no_proxy></no_proxy>
    [<autodetect_protocol></autodetect_protocol>]
    [<autodetect_server_name></autodetect_server_name>]
    [<autodetect_port></autodetect_port>]
</proxy_info>
```

&nbsp;

#### `network_available`
Retry deferred network communication

Request:

```XML
<network_available/>
```

&nbsp;

#### `quit`
Tell client to exit

Request:

```XML
<quit/>
```

&nbsp;

#### `set_language`
Set the language field in the client_state.xml file to append it in any subsequent GET calls to the original URL and translate notices

Request:

```XML
<set_language>
    <language></language>
</set_language>
```
&nbsp;

#### `set_network_mode`
Set the network mode for given duration (in seconds)

Request:

```XML
<set_network_mode>
    [<always/>]
    [<never/>]
    [<auto/>]
    [<restore/>]
    <duration></duration>
</set_network_mode>
```

&nbsp;

#### `set_gpu_mode`
Set GPU run mode for given duration (in seconds)

Request:

```XML
<set_gpu_mode>
    [<always/>]
    [<never/>]
    [<auto/>]
    [<restore/>]
    <duration></duration>
</set_gpu_mode>
```

&nbsp;

#### `set_run_mode`
Set run mode for given duration (in seconds)

Request:

```XML
<set_run_mode>
    [<always/>]
    [<never/>]
    [<auto/>]
    [<restore/>]
    <duration></duration>
</set_run_mode>
```

&nbsp;

#### `set_proxy_settings`
Set the proxy settings

Request:

```XML
<set_proxy_settings>
    <proxy_info>
        [<use_http_proxy/>]
        [<use_socks_proxy/>]
        [<use_http_auth/>]
        <http_server_name></http_server_name>
        <http_server_port></http_server_port>
        <http_user_name></http_user_name>
        <http_user_passwd></http_user_passwd>
        <socks_server_name></socks_server_name>
        <socks_server_port></socks_server_port>
        <socks5_user_name></socks5_user_name>
        <socks5_user_passwd></socks5_user_passwd>
        <socks5_remote_dns></socks5_remote_dns>
        <no_proxy></no_proxy>
    </proxy_info>
</set_proxy_settings>
``` 

&nbsp;

#### `get_cc_config`
Get the contents of the cc_config.xml file if present

Request:

```XML
<get_cc_config/>
```

Reply: The contents of the file.

&nbsp;

#### `read_cc_config`
Read the `cc_config.xml` file and set the configuration accordingly. If no such file is present or it's contents are not formatted correctly the defaults are used.

Request:

```XML
<read_cc_config/>
```

&nbsp;

#### `set_cc_config`
Write a new cc_config.xml file

Request:

```XML
<set_cc_config>
      .
      .
      .
</set_cc_config>
```
&nbsp;

#### `get_app_config`
Get the contents of the app_config.xml file if present

Request:

```XML
<get_app_config/>
```

Reply: The contents of the file.

&nbsp;

#### `set_app_config`
Write a new app_config.xml file

Request:

```XML
<set_app_config>
    <url></url>
      .   
      .
      .
</set_app_config>
```

&nbsp;

#### `report_device_status`
Used to report the status of an android device to the client. This is used to more easily access the status of the device. It essentially extracts the information using the, written in JAVA, Android GUI and using the RPC passes them to the client (a kind of bridge between Android's JAVA interface and the client's C++ one). It is therefore not of any use to be documented here.  

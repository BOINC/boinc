<?php
// $Id$

/**
 * @file
 * Exposes BOINC work data to Views.
 */

/**
 * Implementation of hook_views_data().
 * Defines the structure of data and how Views should handle it (i.e. which
 * relationships exist; is data text, integer, float; what sort options exist).
 */
function boincwork_views_data() {

  // -----------------------------------------------------------------------------------------------
  // Definition for host table
  // -----------------------------------------------------------------------------------------------

  $data['host']['table']['group'] = t('BOINC');

  $data['host']['table']['base'] = array(
      'field' => 'id',
      'title' => t('BOINC host'),
      'help' => t('Computers attached to BOINC'),
      'database' => 'boinc_ro'
  );

  // This table references the {user} table.
  // This join creates an 'implicit' relationship to the user table, so that when
  // "User" is the base table, the fields are automatically available.

  // Index this array by the table name to which this table refers.
  // 'left_field' is the primary key in the referenced table.
  // 'field' is the foreign key in this table.

  $data['host']['table']['join'] = array(
    'user' => array(
      'left_field' => 'id',
      'field' => 'userid',
    )
  );

  // Describe each of the individual fields in this table to Views. For
  // each field, you may define what field, sort, argument, and/or filter
  // handlers it supports. This will determine where in the Views interface you
  // may use the field.

  // Primary keys allowed as arguments

  $data['host']['id'] = array(
    'title' => bts('Computer ID', array(), NULL, 'boinc:host-list'),
    'help' => t('The ID number of this host.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'argument' => array(
      'handler' => 'views_handler_argument',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['venue'] = array(
    'title' => bts('Location', array(), NULL, 'boinc:host-list'),
    'help' => t('The name of the preference set assigned to this host.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'argument' => array(
      'handler' => 'views_handler_argument',
      'name field' => 'title'
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );

  // Foreign key fields

  $data['host']['userid'] = array(
    'title' => bts('User ID', array(), NULL, 'boinc:user-details'),
    'help' => t('The owner of this host.'),
    // Because this is a foreign key to the {user} table. This allows us to
    // have, when the view is configured with this relationship, all the fields
    // for the related node available.
    'argument' => array(
      'handler' => 'views_handler_argument_boincuser_id',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'relationship' => array(
      'base' => 'user',
      'field' => 'userid',
      'handler' => 'views_handler_relationship',
      'label' => t('User')
    ),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // Descriptions of general host fields (alphabetized)

  $data['host']['active_frac'] = array(
    'title' => bts('While BOINC running, % of time work is allowed', array(), NULL, 'boinc:host-details'),
    'help' => t('The percentage of time that work is allowed.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['avg_turnaround'] = array(
    'title' => bts('Average turnaround time', array(), NULL, 'boinc:host-details'),
    'help' => t('The average time required for work to be completed.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['create_time'] = array(
    'title' => bts('Created', array(), NULL, 'boinc:host-details'),
    'help' => t('The time the host record was created.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date'
    )
  );
  $data['host']['connected_frac'] = array(
    'title' => bts('While BOINC running, % of time host has an Internet connection', array(), NULL, 'boinc:host-details'),
    'help' => t('The percentage of time that the host has an Internet connection.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['cpu_efficiency'] = array(
    'title' => bts('Average CPU efficiency', array(), NULL, 'boinc:host-details'),
    'help' => t('The average efficiency of the host CPU.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['d_free'] = array(
    'title' => bts('Free disk space', array(), NULL, 'boinc:host-details'),
    'help' => t('The amount of disk space that is not in use.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['d_total'] = array(
    'title' => bts('Total disk space', array(), NULL, 'boinc:host-details'),
    'help' => t('The size of the disk on which BOINC is installed.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['domain_name'] = array(
    'title' => bts('Name', array(), NULL, 'boinc:details:-1:name-of-the-host-or-task-or-workunit-etc-being-viewed-ignoreoverwrite'),
    'help' => t('The name of the host.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );
  $data['host']['duration_correction_factor'] = array(
    'title' => bts('Task duration correction factor', array(), NULL, 'boinc:host-details'),
    'help' => t('Task duration correction factor.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['expavg_credit'] = array(
    'title' => bts('Avg. credit', array(), NULL, 'boinc:host-details'),
    'help' => t('A decaying average of credit per day.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['external_ip_addr'] = array(
    'title' => bts('External IP address', array(), NULL, 'boinc:host-details'),
    'help' => t('The IP address from which the host last communicated.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );
  $data['host']['host_cpid'] = array(
    'title' => bts('Cross project ID', array(), NULL, 'boinc:host-details'),
    'help' => t('The consistent identifier of the host across projects.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );
  $data['host']['last_ip_addr'] = array(
    'title' => bts('Last IP address', array(), NULL, 'boinc:host-details'),
    'help' => t('The IP address of the host at last communication.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );
  $data['host']['m_cache'] = array(
    'title' => bts('Cache', array(), NULL, 'boinc:host-details'),
    'help' => t('The amount of cache on the processor(s).'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['m_nbytes'] = array(
    'title' => bts('Memory', array(), NULL, 'boinc:host-details'),
    'help' => t('The amount of memory installed.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['m_swap'] = array(
    'title' => bts('Swap space', array(), NULL, 'boinc:host-details'),
    'help' => t('The amount of swap space configured.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['max_results_day'] = array(
    'title' => bts('Maximum daily WU quota per CPU', array(), NULL, 'boinc:host-details'),
    'help' => t('The maximum number of workunits to be allocated per CPU.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['n_bwdown'] = array(
    'title' => bts('Average download rate', array(), NULL, 'boinc:host-details'),
    'help' => t('The average rate at which new work is downloaded.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['n_bwup'] = array(
    'title' => bts('Average upload rate', array(), NULL, 'boinc:host-details'),
    'help' => t('The average rate at which completed work is uploaded.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['nsame_ip_addr'] = array(
    'title' => bts('Same IP address count', array(), NULL, 'boinc:host-details'),
    'help' => t('The number of times the last IP address.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );
  $data['host']['on_frac'] = array(
    'title' => bts('% of time BOINC client is running', array(), NULL, 'boinc:host-details'),
    'help' => t('The percentage of time that the BOINC client is running.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['os_name'] = array(
    'title' => bts('Operating system', array(), NULL, 'boinc:host-details'),
    'help' => t('The name of the host operating system.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );
  $data['host']['os_version'] = array(
    'title' => bts('Operating system version', array(), NULL, 'boinc:host-details'),
    'help' => t('The version of the host operating system.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );
  $data['host']['p_fpops'] = array(
    'title' => bts('Measured floating point speed', array(), NULL, 'boinc:host-details'),
    'help' => t('The capability of the host in floating point operations per second.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['p_iops'] = array(
    'title' => bts('Measured integer speed', array(), NULL, 'boinc:host-details'),
    'help' => t('The capability of the host in integer operations per second.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['p_model'] = array(
    'title' => bts('CPU model', array(), NULL, 'boinc:host-details'),
    'help' => t('The model of the CPU in the host.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );
  $data['host']['p_ncpus'] = array(
    'title' => bts('Number of processors', array(), NULL, 'boinc:host-details'),
    'help' => t('The count of CPU cores in the system.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );
  $data['host']['p_vendor'] = array(
    'title' => bts('CPU', array(), NULL, 'boinc:host-details'),
    'help' => t('The manufacturer of the CPU in the host.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );
  $data['host']['rpc_seqno'] = array(
    'title' => bts('Number of times client has contacted server', array(), NULL, 'boinc:host-details'),
    'help' => t('The number of times the client has contacted the server.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['rpc_time'] = array(
    'title' => bts('Last contact', array(), NULL, 'boinc:host-details'),
    'help' => t('The time of the last RPC contact with the host.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date'
    )
  );
  $data['host']['serialnum'] = array(
    'title' => bts('Serial number', array(), NULL, 'boinc:host-details'),
    'help' => t('Contains the release number of the BOINC application in use and the GPU of the host.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );
  $data['host']['timezone'] = array(
    'title' => bts('Timezone', array(), NULL, 'boinc:host-details'),
    'help' => t('The UTC offset of the local time.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host']['total_credit'] = array(
    'title' => bts('Total credit', array(), NULL, 'boinc:user-or-team-total-credits'),
    'help' => t('The total accumulated BOINC credit for a host.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // ------------------------------------------------------------------------------------------------
  // Definition for platform table
  // ------------------------------------------------------------------------------------------------

  $data['platform']['table']['group'] = t('BOINC');
  $data['platform']['table']['base'] = array(
      'field' => 'id',
      'title' => t('BOINC platform'),
      'help' => t('Operating system platforms'),
      'database' => 'boinc_ro'
  );

  // Primary keys allowed as arguments

  $data['platform']['id'] = array(
    'title' => bts('Platform ID', array(), NULL, 'boinc:platform-details'),
    'help' => t('The ID number of the platform.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'argument' => array(
      'handler' => 'views_handler_argument',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // Descriptions of app fields (alphabetized)

  $data['platform']['name'] = array(
    'title' => bts('Name', array(), NULL, 'boinc:details:-1:name-of-the-host-or-task-or-workunit-etc-being-viewed-ignoreoverwrite'),
    'help' => t('The name of the platform.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );

  // ------------------------------------------------------------------------------------------------
  // Definition for result (task) table
  // ------------------------------------------------------------------------------------------------

  $data['result']['table']['group'] = t('BOINC');
  $data['result']['table']['base'] = array(
      'field' => 'id',
      'title' => t('BOINC result'),
      'help' => t('Results produced by BOINC'),
      'database' => 'boinc_ro'
  );

  // Create implicit relationships to the user, workunit, and host tables

  $data['result']['table']['join'] = array(
    'user' => array(
      'left_field' => 'id',
      'field' => 'userid'
    ),
    'workunit' => array(
      'left_field' => 'id',
      'field' => 'workunitid'
    ),
    'host' => array(
      'left_field' => 'id',
      'field' => 'hostid'
    )
  );

  // Primary keys allowed as arguments

  $data['result']['id'] = array(
    'title' => bts('Task ID', array(), NULL, 'boinc:task-details'),
    'help' => t('The ID number of this result.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'argument' => array(
      'handler' => 'views_handler_argument',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // Foreign key fields

  $data['result']['userid'] = array(
    'title' => bts('User ID', array(), NULL, 'boinc:user-details'),
    'help' => t('The owner of this result.'),
    // Foreign key to the {user} table
    'argument' => array(
      'handler' => 'views_handler_argument_boincuser_id',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'relationship' => array(
      'base' => 'user',
      'field' => 'userid',
      'handler' => 'views_handler_relationship',
      'label' => t('User')
    )
  );
  $data['result']['workunitid'] = array(
    'title' => bts('Workunit ID', array(), NULL, 'boinc:workunit-details'),
    'help' => t('The workunit associated with this result.'),
    // Foreign key to the {workunit} table
    'argument' => array(
      'handler' => 'views_handler_argument',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'relationship' => array(
      'base' => 'workunit',
      'field' => 'workunitid',
      'handler' => 'views_handler_relationship',
      'label' => t('Workunit')
    ),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['result']['hostid'] = array(
    'title' => bts('Host ID', array(), NULL, 'boinc:task-details'),
    'help' => t('The host associated with this result.'),
    // Foreign key to the {host} table
    'argument' => array(
      'handler' => 'views_handler_argument',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'relationship' => array(
      'base' => 'host',
      'field' => 'hostid',
      'handler' => 'views_handler_relationship',
      'label' => t('Host')
    ),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // Descriptions of result fields (alphabetized)

  $data['result']['appid'] = array(
    'title' => bts('Application ID', array(), NULL, 'boinc:application-details'),
    'help' => t('The ID number of the application.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['result']['app_version_id'] = array(
    'title' => bts('Application version', array(), NULL, 'boinc:task-details'),
    'help' => t('The version number of the application.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['result']['claimed_credit'] = array(
    'title' => bts('Claimed credit', array(), NULL, 'boinc:task-details'),
    'help' => t('The credit claimed for this result.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['result']['client_state'] = array(
    'title' => bts('Client state', array(), NULL, 'boinc:task-details'),
    'help' => t('The state of the task on the client side.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['result']['cpu_time'] = array(
    'title' => bts('CPU time', array(), NULL, 'boinc:task-details'),
    'help' => t('The CPU time spent on the task.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['result']['create_time'] = array(
    'title' => bts('Created', array(), NULL, 'boinc:task-details:-1:ignoreoverwrite'),
    'help' => t('The time that the task was created.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date'
    )
  );
  $data['result']['elapsed_time'] = array(
    'title' => bts('Run time', array(), NULL, 'boinc:task-details'),
    'help' => t('The run time of the task.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['result']['exit_status'] = array(
    'title' => bts('Exit status', array(), NULL, 'boinc:task-details'),
    'help' => t('The exit code of the task.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['result']['granted_credit'] = array(
    'title' => bts('Granted credit', array(), NULL, 'boinc:task-details'),
    'help' => t('The credit granted for this result.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['result']['name'] = array(
    'title' => bts('Name', array(), NULL, 'boinc:details:-1:name-of-the-host-or-task-or-workunit-etc-being-viewed-ignoreoverwrite'),
    'help' => t('The name of the task.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );
  $data['result']['outcome'] = array(
    'title' => bts('Outcome', array(), NULL, 'boinc:task-details'),
    'help' => t('The outcome of the task.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['result']['received_time'] = array(
    'title' => bts('Received time', array(), NULL, 'boinc:task-details'),
    'help' => t('The time that the result was received.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date'
    )
  );
  $data['result']['report_deadline'] = array(
    'title' => bts('Report deadline', array(), NULL, 'boinc:task-details'),
    'help' => t('The deadline for the task.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date'
    )
  );
  $data['result']['sent_time'] = array(
    'title' => bts('Sent', array(), NULL, 'boinc:task-details'),
    'help' => t('The time that the task was sent.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date'
    )
  );
  $data['result']['server_state'] = array(
    'title' => bts('Server state', array(), NULL, 'boinc:task-details'),
    'help' => t('The state of task on the server side.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['result']['stderr_out'] = array(
    'title' => bts('Stderr out', array(), NULL, 'boinc:task-details'),
    'help' => t('The output to standard error.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );
  $data['result']['validate_state'] = array(
    'title' => bts('Validation state', array(), NULL, 'boinc:task-details'),
    'help' => t('The state of validation of this result.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['result']['peak_working_set_size'] = array(
    'title' => bts('Peak working set size', array(), NULL, 'boinc:task-details'),
    'help' => t('Peak working set size.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['result']['peak_swap_size'] = array(
    'title' => bts('Peak swap size', array(), NULL, 'boinc:task-details'),
    'help' => t('Peak swap size.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
    $data['result']['peak_disk_usage'] = array(
    'title' => bts('Peak disk usage', array(), NULL, 'boinc:task-details'),
    'help' => t('Peak disk usage.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );


  // ------------------------------------------------------------------------------------------------
  // Definition for workunit table
  // ------------------------------------------------------------------------------------------------

  $data['workunit']['table']['group'] = t('BOINC');
  $data['workunit']['table']['base'] = array(
      'field' => 'id',
      'title' => t('BOINC workunit'),
      'help' => t('Workunits produced by BOINC'),
      'database' => 'boinc_ro'
  );

  // Create implicit relationship to the app table

  $data['workunit']['table']['join'] = array(
    'app' => array(
      'left_field' => 'id',
      'field' => 'appid'
    )
  );

  // Primary keys allowed as arguments

  $data['workunit']['id'] = array(
    'title' => bts('Workunit ID', array(), NULL, 'boinc:workunit-details'),
    'help' => t('The ID number of this workunit.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'argument' => array(
      'handler' => 'views_handler_argument',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // Foreign key fields

  $data['workunit']['appid'] = array(
    'title' => bts('Application ID', array(), NULL, 'boinc:application-details'),
    'help' => t('The application associated with this workunit.'),
    'argument' => array( // foreign key to the {app} table
      'handler' => 'views_handler_argument',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'relationship' => array(
      'base' => 'app',
      'field' => 'appid',
      'handler' => 'views_handler_relationship',
      'label' => t('Application')
    ),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // Descriptions of workunit fields (alphabetized)

  $data['workunit']['canonical_credit'] = array(
    'title' => bts('Canonical credit', array(), NULL, 'boinc:workunit-details'),
    'help' => t('The canonical credit of the workunit.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
      'float' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['workunit']['canonical_resultid'] = array(
    'title' => bts('Canonical result ID', array(), NULL, 'boinc:workunit-details'),
    'help' => t('The ID of the canonical result associated with the workunit.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['workunit']['create_time'] = array(
    'title' => bts('Created time', array(), NULL, 'boinc:workunit-details'),
    'help' => t('The time that the workunit was created.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date'
    )
  );
  $data['workunit']['error_mask'] = array(
    'title' => bts('Error mask', array(), NULL, 'boinc:workunit-details'),
    'help' => t('Error mask of the workunit.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['workunit']['min_quorum'] = array(
    'title' => bts('Minimum quorum', array(), NULL, 'boinc:workunit-details'),
    'help' => t('Minimum quorum of the workunit.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['workunit']['name'] = array(
    'title' => bts('Name', array(), NULL, 'boinc:details:-1:name-of-the-host-or-task-or-workunit-etc-being-viewed-ignoreovewrite'),
    'help' => t('The name of the workunit.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );
  $data['workunit']['max_error_results'] = array(
    'title' => bts('Max error tasks', array(), NULL, 'boinc:workunit-details'),
    'help' => t('The maximum number of error tasks in the workunit.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['workunit']['max_success_results'] = array(
    'title' => bts('Max success tasks', array(), NULL, 'boinc:workunit-details'),
    'help' => t('The maximum number of successful tasks in the workunit.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['workunit']['max_total_results'] = array(
    'title' => bts('Max total tasks', array(), NULL, 'boinc:workunit-details'),
    'help' => t('The maximum number of total tasks in the workunit.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['workunit']['need_validate'] = array(
    'title' => bts('Needs validation', array(), NULL, 'boinc:workunit-details'),
    'help' => t('Whether or not a workunit needs validation.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['workunit']['target_nresults'] = array(
    'title' => bts('Initial replication', array(), NULL, 'boinc:workunit-details'),
    'help' => t('The target number of results for the workunit.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // ------------------------------------------------------------------------------------------------
  // Definition for app table
  // ------------------------------------------------------------------------------------------------

  $data['app']['table']['group'] = t('BOINC');
  $data['app']['table']['base'] = array(
      'field' => 'id',
      'title' => t('BOINC app'),
      'help' => t('Research applications run by BOINC'),
      'database' => 'boinc_ro'
  );

  // Primary keys allowed as arguments

  $data['app']['id'] = array(
    'title' => bts('Application ID', array(), NULL, 'boinc:application-details'),
    'help' => t('The ID number of this application.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'argument' => array(
      'handler' => 'views_handler_argument',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // Descriptions of app fields (alphabetized)

  $data['app']['target_nresults'] = array(
    'title' => bts('Target number of results', array(), NULL, 'boinc:application-details'),
    'help' => t('The number of times to replicate tasks in this application.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['app']['user_friendly_name'] = array(
    'title' => bts('Name', array(), NULL, 'boinc:details:-1:name-of-the-host-or-task-or-workunit-etc-being-viewed-ignoreoverwrite'),
    'help' => t('The user friendly name of the application.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );

  // ------------------------------------------------------------------------------------------------
  // Definition for app_version table
  // ------------------------------------------------------------------------------------------------

  $data['app_version']['table']['group'] = t('BOINC');
  $data['app_version']['table']['base'] = array(
      'field' => 'id',
      'title' => t('BOINC app version'),
      'help' => t('Versions of research applications run by BOINC'),
      'database' => 'boinc_ro'
  );

  // Primary keys allowed as arguments

  $data['app_version']['id'] = array(
    'title' => bts('Application version ID', array(), NULL, 'boinc:application-details'),
    'help' => t('The ID number of this application version.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'argument' => array(
      'handler' => 'views_handler_argument',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // Foreign key fields

  $data['app_version']['appid'] = array(
    'title' => bts('Application ID', array(), NULL, 'boinc:application-details'),
    'help' => t('The application ID associated with this app version.'),
    // Because this is a foreign key to the {app} table. This allows us to
    // have, when the view is configured with this relationship, all the fields
    // for the related node available.
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'relationship' => array(
      'base' => 'app',
      'field' => 'id',
      'handler' => 'views_handler_relationship',
      'label' => t('App ID')
    ),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  $data['app_version']['platformid'] = array(
    'title' => bts('Platform ID', array(), NULL, 'boinc:platform-details'),
    'help' => t('The platform ID associated with this app version.'),
    // Because this is a foreign key to the {platform} table. This allows us to
    // have, when the view is configured with this relationship, all the fields
    // for the related node available.
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'relationship' => array(
      'base' => 'platform',
      'field' => 'id',
      'handler' => 'views_handler_relationship',
      'label' => t('Platform ID')
    ),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // Descriptions of app fields (alphabetized)

  $data['app_version']['plan_class'] = array(
    'title' => bts('Plan class', array(), NULL, 'boinc:application-details'),
    'help' => t('App version plan class.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );
  $data['app_version']['version_num'] = array(
    'title' => bts('Version number', array(), NULL, 'boinc:application-details'),
    'help' => t('The application version number.'),
    'field' => array(
      'handler' => 'views_handler_field_boincwork_app_version_number',
      'click sortable' => TRUE,
      'float' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // ------------------------------------------------------------------------------------------------
  // Definition for host_app_version table
  // ------------------------------------------------------------------------------------------------

  $data['host_app_version']['table']['group'] = t('BOINC');

  // This table references the {host} table.
  // This join creates an 'implicit' relationship to the host table, so that when
  // "Host" is the base table, the fields are automatically available.

  // Index this array by the table name to which this table refers.
  // 'left_field' is the primary key in the referenced table.
  // 'field' is the foreign key in this table.

  $data['host_app_version']['table']['join'] = array(
    'host' => array(
      'left_field' => 'id',
      'field' => 'host_id',
    )
  );

  // Primary keys allowed as arguments

  $data['host_app_version']['host_id'] = array(
    'title' => bts('Computer', array(), NULL, 'boinc:host-details'),
    'help' => t('The host ID connected to this app version data.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'argument' => array(
      'handler' => 'views_handler_argument',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // Foreign key fields

  $data['host_app_version']['app_version_id'] = array(
    'title' => bts('App version ID', array(), NULL, 'boinc:application-details'),
    'help' => t('The app version ID connected to this host data.'),
    // Because this is a foreign key to the {host} table. This allows us to
    // have, when the view is configured with this relationship, all the fields
    // for the related node available.
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'name field' => 'title',
      'numeric' => TRUE,
      'validate type' => 'id'
    ),
    'relationship' => array(
      'base' => 'app_version',
      'field' => 'id',
      'handler' => 'views_handler_relationship',
      'label' => t('App version')
    ),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );

  // Descriptions of host_app_version fields (alphabetized)

  $data['host_app_version']['consecutive_valid'] = array(
    'title' => bts('Consecutive valid tasks', array(), NULL, 'boinc:application-details'),
    'help' => t('The number of consecutive valid tasks for this app and by this host.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host_app_version']['et_avg'] = array(
    'title' => t('Average processing rate'),
    'help' => t('The "et_avg" for this app and by this host.'),
    'field' => array(
      'handler' => 'views_handler_field_boincwork_host_app_et_avg',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host_app_version']['et_n'] = array(
    'title' => bts('Number of tasks completed', array(), NULL, 'boinc:application-details'),
    'help' => t('The tasks completed for this app and by this host.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host_app_version']['max_jobs_per_day'] = array(
    'title' => bts('Max tasks per day', array(), NULL, 'boinc:application-details'),
    'help' => t('The maximum tasks per day for this app and by this host.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host_app_version']['n_jobs_today'] = array(
    'title' => bts('Number of tasks today', array(), NULL, 'boinc:application-details'),
    'help' => t('The number of tasks today for this app and by this host.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host_app_version']['turnaround_avg'] = array(
    'title' => bts('Average turnaround time', array(), NULL, 'boinc:application-details:-1:ignoreoverwrite'),
    'help' => t('The average turnaround time per task for this app and by this host.'),
    'field' => array(
      'handler' => 'views_handler_field_boincwork_host_app_turnaround_avg',
      'click sortable' => TRUE,
      'float' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_numeric'
    )
  );
  $data['host_app_version']['user_friendly_name'] = array(
    'title' => bts('Name', array(), NULL, 'boinc:details:-1:name-of-the-host-or-task-or-workunit-etc-being-viewed-ignoreoverwrite'),
    'help' => t('The user friendly name of the application.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string'
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_string'
    )
  );

  return $data;
}

/*
 * hook_views_handlers(): Reference custom handlers for data.
 * Custom handlers can manipulate data in useful ways. The boincuser_id
 * argument takes a Drupal user ID and converts it to BOINC ID via a
 * reference table for seamless access of user data in the BOINC database.
 */

function boincwork_views_handlers() {

  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'boincwork') . '/views',
    ),
    'handlers' => array(
      'views_handler_argument_boincuser_id' => array(
        'parent' => 'views_handler_argument_numeric'
      ),
      'views_handler_field_boincwork_app_version_number' => array(
        'parent' => 'views_handler_field_numeric'
      ),
      'views_handler_field_boincwork_host_app_et_avg' => array(
        'parent' => 'views_handler_field_numeric'
      ),
      'views_handler_field_boincwork_host_app_turnaround_avg' => array(
        'parent' => 'views_handler_field_numeric'
      ),
    )
  );
}

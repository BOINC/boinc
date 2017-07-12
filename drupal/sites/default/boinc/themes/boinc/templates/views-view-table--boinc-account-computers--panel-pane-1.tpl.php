<?php
  global $base_path;
  // If a host hasn't been active in 30 days, set class to inactive
  $inactive_threshold = time() - (30 * 24 * 60 * 60);
?>

<script>
  $(document).ready(function() {
    $("tr.link").click(function() {
      window.location = $(this).attr("dest");
    });
  });
</script>

<table>
<thead>
  <tr>
    <th><?php print bts('Name', array(), NULL, 'boinc:details:-1:name-of-the-host-or-task-or-workunit-etc-being-viewed-ignoreoverwrite'); ?></th>
    <th class="numeric"><?php print bts('Avg credit', array(), NULL, 'boinc:account-dashboard'); ?></th>
    <th class="numeric"><?php print bts('Total credit', array(), NULL, 'boinc:user-or-team-total-credits'); ?></th>
  </tr>
</thead>
<tbody>
  <?php foreach ($rows as $row): ?>
    <tr class="link <?php print ($row['rpc_time'] < $inactive_threshold) ? 'inactive' : ''; ?>" dest="<?php print $base_path; ?>host/<?php print $row['id']; ?>">
      <td><?php print l($row['domain_name'], "host/{$row['id']}"); ?>
      <td class="numeric"><?php print $row['expavg_credit']; ?>
      <td class="numeric"><?php print $row['total_credit']; ?>
    </tr>
  <?php endforeach; ?>
</tbody>
</table>
<ul class="more-link tab-list">
  <li class="first tab">
    <?php print l(bts('More', array(), NULL, 'boinc:account-dashboard'), 'account/computers'); ?>
  </li>
  <li class="first alt tab">
    <?php print l(bts('Tasks', array(), NULL, 'boinc:host-tasks-link'), 'account/tasks/0/0'); ?>
  </li>
</ul>
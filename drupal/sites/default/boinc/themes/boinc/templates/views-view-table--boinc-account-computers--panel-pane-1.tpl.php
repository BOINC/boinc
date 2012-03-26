<?php
  // If a host hasn't been active in 30 days, set class to inactive
  $inactive_threshold = time() - (30 * 24 * 60 * 60);
?>

<table>
<thead>
  <tr>
    <th>Name</th>
    <th class="numeric">Avg credit</th>
    <th class="numeric">Total credit</th>
  </tr>
</thead>
<tbody>
  <?php foreach ($rows as $row): ?>
    <tr <?php print ($row['rpc_time'] < $inactive_threshold) ? 'class="inactive"' : ''; ?>>
      <td><?php print $row['domain_name']; ?>
      <td class="numeric"><?php print $row['expavg_credit']; ?>
      <td class="numeric"><?php print $row['total_credit']; ?>
    </tr>
  <?php endforeach; ?>
</tbody>
</table>
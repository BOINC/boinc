<?php
// Need to get BOINC user name from Drupal UID
$user = user_load($fields['uid']->content);
?>
<a href="<?php echo url('account/' . $fields['uid']->content); ?>" alt="<?php echo $user->boincuser_name; ?>" title="<?php echo $user->boincuser_name; ?>">
  <?php echo $fields['field_image_fid']->content; ?>
</a>

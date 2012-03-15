<?php
$imce =& $imce_ref['imce'];//keep this line.

/*
 * Although the file list table here is avaliable for theming,
 * it is not recommended to change the table structure, because
 * it is read and manipulated by javascript assuming this is the deafult structure.
 * You can always change the data created by format functions
 * such as format_size or format_date, or you can do css theming which is the best practice here.
 */
?>

<table id="file-list" class="files"><tbody><?php
if ($imce['perm']['browse'] && !empty($imce['files'])) {
  foreach ($imce['files'] as $name => $file) {?>
  <tr id="<?php print $raw = rawurlencode($file['name']); ?>">
    <td class="name"><?php print $raw; ?></td>
    <td class="size" id="<?php print $file['size']; ?>"><?php print format_size($file['size']); ?></td>
    <td class="width"><?php print $file['width']; ?></td>
    <td class="height"><?php print $file['height']; ?></td>
    <td class="date" id="<?php print $file['date']; ?>"><?php print format_date($file['date'], 'small'); ?></td>
  </tr><?php
  }
}?>
</tbody></table>
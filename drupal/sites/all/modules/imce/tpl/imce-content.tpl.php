<?php
$imce =& $imce_ref['imce'];//keep this line.
?>

<script type="text/javascript">
<!--//--><![CDATA[//><!--
  imce.hooks.load.push(imce.initiateShortcuts); //shortcuts for directories and files
  imce.hooks.load.push(imce.initiateSorting); //file sorting
  imce.hooks.load.push(imce.initiateResizeBars); //area resizing

  //inline preview
  imce.hooks.list.push(imce.thumbRow);
  imce.vars.tMaxW = 120; //maximum width of an image to be previewed inline
  imce.vars.tMaxH = 120; //maximum height of an image to be previewed inline
  imce.vars.prvW = 40; //maximum width of the thumbnail used in inline preview.
  imce.vars.prvH = 40; //maximum height of the thumbnail used in inline preview.
  //imce.vars.prvstyle = 'stylename'; //preview larger images inline using an image style(imagecache preset).

  //enable box view for file list. set box dimensions = preview dimensions + 30 or more
  //imce.vars.boxW = 100; //width of a file info box
  //imce.vars.boxH = 100; //height of a file info box

  //imce.vars.previewImages = 0; //disable click previewing of images.
  //imce.vars.cache = 0; //disable directory caching. File lists will always refresh.
  //imce.vars.absurls = 1; //make IMCE return absolute file URLs to external applications.
//--><!]]>
</script>

<div id="imce-content">

<div id="message-box"></div>

<div id="help-box"><!-- Update help content if you disable any of the extra features above. -->
  <div id="help-box-title"><span><?php print t('Help'); ?>!</span></div>
  <div id="help-box-content">
    <h4><?php print t('Tips'); ?>:</h4>
    <ul class="tips">
      <li><?php print t('Select a file by clicking the corresponding row in the file list.'); ?></li>
      <li><?php print t('Ctrl+click to add files to the selection or to remove files from the selection.'); ?></li>
      <li><?php print t('Shift+click to create a range selection. Click to start the range and shift+click to end it.'); ?></li>
      <li><?php print t('In order to send a file to an external application, double click on the file row.'); ?></li>
      <li><?php print t('Sort the files by clicking a column header of the file list.'); ?></li>
      <li><?php print t('Resize the work-spaces by dragging the horizontal or vertical resize-bars.'); ?></li>
      <li><?php print t('Keyboard shortcuts for file list: up, down, left, home, end, ctrl+A.'); ?></li>
      <li><?php print t('Keyboard shortcuts for selected files: enter/insert, delete, R(esize), T(humbnails), U(pload).'); ?></li>
      <li><?php print t('Keyboard shortcuts for directory list: up, down, left, right, home, end.'); ?></li>
    </ul>
    <h4><?php print t('Limitations'); ?>:</h4>
    <ul class="tips">
      <li><?php print t('Maximum file size per upload') .': '. ($imce['filesize'] ? format_size($imce['filesize']) : t('unlimited')); ?></li>
      <li><?php print t('Permitted file extensions') .': '. ($imce['extensions'] != '*' ? $imce['extensions'] : t('all')); ?></li>
      <li><?php print t('Maximum image resolution') .': '. ($imce['dimensions'] ? $imce['dimensions'] : t('unlimited')); ?></li>
      <li><?php print t('Maximum number of files per operation') .': '. ($imce['filenum'] ? $imce['filenum'] : t('unlimited')); ?></li>
    </ul>
  </div>
</div>

<div id="ops-wrapper">
  <div id="op-items"><ul id="ops-list"><li></li></ul></div>
  <div id="op-contents"></div>
</div>

<div id="resizable-content">

<div id="browse-wrapper">

  <div id="navigation-wrapper">
    <div class="navigation-text" id="navigation-header"><span><?php print t('Navigation'); ?></span></div>
    <ul id="navigation-tree"><li class="expanded root"><?php print $tree; ?></li></ul>
  </div>

  <div id="navigation-resizer" class="x-resizer"></div>

  <div id="sub-browse-wrapper">

    <div id="file-header-wrapper">
      <table id="file-header" class="files"><tbody><tr>
        <td class="name"><?php print t('File name'); ?></td>
        <td class="size"><?php print t('Size'); ?></td>
        <td class="width"><?php print t('Width'); ?></td>
        <td class="height"><?php print t('Height'); ?></td>
        <td class="date"><?php print t('Date'); ?></td>
      </tr></tbody></table>
    </div>

    <div id="file-list-wrapper">
      <?php print theme('imce_file_list', $imce_ref); /* see imce-file-list-tpl.php */?>
    </div>

    <div id="dir-stat"><?php print t('!num files using !dirsize of !quota', array(
        '!num' => '<span id="file-count">'. count($imce['files']) .'</span>',
        '!dirsize' => '<span id="dir-size">'. format_size($imce['dirsize']) .'</span>',
        '!quota' => '<span id="dir-quota">'. ($imce['quota'] ? format_size($imce['quota']) : ($imce['tuquota'] ? format_size($imce['tuquota']) : t('unlimited quota'))) .'</span>'
      )); ?>
    </div>

  </div><!-- sub-browse-wrapper -->
</div><!-- browse-wrapper -->

<div id="browse-resizer" class="y-resizer"></div>

<div id="preview-wrapper"><div id="file-preview"></div></div>

</div><!-- resizable-content -->

<div id="forms-wrapper"><?php print $forms; ?></div>

</div><!-- imce-content -->
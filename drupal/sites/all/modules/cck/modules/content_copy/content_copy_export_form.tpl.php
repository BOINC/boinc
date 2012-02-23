<?php
// $Id: content_copy_export_form.tpl.php,v 1.1.2.2 2008/10/28 02:11:49 yched Exp $

if ($form['#step'] == 2):
  if ($rows): ?>
    <table id="content-copy-export" class="sticky-enabled">
      <thead>
        <tr>
          <th><?php print t('Export'); ?></th>
          <th><?php print t('Label'); ?></th>
          <th><?php print t('Name'); ?></th>
          <th><?php print t('Type'); ?></th>
        </tr>
      </thead>
      <tbody>
        <?php
        $count = 0;
        foreach ($rows as $row): ?>
          <tr class="<?php print $count % 2 == 0 ? 'odd' : 'even'; ?>">
          <?php
          switch ($row->row_type):
            case 'field': ?>
              <td><?php print $row->checkbox; ?></td>
              <td><?php print $row->indentation; ?><span class="<?php print $row->label_class; ?>"><?php print $row->human_name; ?></span></td>
              <td><?php print $row->field_name; ?></td>
              <td><?php print $row->type; ?></td>
            <?php break;
            case 'group': ?>
              <td><?php print $row->checkbox; ?></td>
              <td><?php print $row->indentation; ?><span class="<?php print $row->label_class; ?>"><?php print $row->human_name; ?></span></td>
              <td colspan="2"><?php print $row->group_name; ?></td>
              <?php break;
            endswitch; ?>
          </tr>
          <?php $count++;
        endforeach; ?>
      </tbody>
    </table>
  <?php endif;
  endif;
print $submit; ?>


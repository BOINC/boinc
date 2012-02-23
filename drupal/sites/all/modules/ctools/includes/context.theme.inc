<?php
// $Id: context.theme.inc,v 1.7.2.4 2010/09/10 18:25:13 merlinofchaos Exp $

/**
 * @file
 * Contains theme registry and theme implementations for the context tool.
 */

/**
 * Implementation of hook_theme()
 */
function ctools_context_theme(&$theme) {
  $theme['ctools_context_list'] = array(
    'arguments' => array('object'),
    'file' => 'includes/context.theme.inc',
  );
  $theme['ctools_context_list_no_table'] = array(
    'arguments' => array('object'),
    'file' => 'includes/context.theme.inc',
  );
  $theme['ctools_context_item_form'] = array(
    'arguments' => array('form'),
    'file' => 'includes/context.theme.inc',
  );
  $theme['ctools_context_item_row'] = array(
    'arguments' => array('type', 'form', 'position', 'count', 'with_tr' => TRUE),
    'file' => 'includes/context.theme.inc',
  );

  // For the access plugin
  $theme['ctools_access_admin_add'] = array(
    'arguments' => array('form'),
    'file' => 'includes/context-admin.inc',
  );
}

/**
 * Theme the form item for the context entry.
 */
function theme_ctools_context_item_row($type, $form, $position, $count, $with_tr = TRUE) {
  $output = '<td class="title">&nbsp;' . drupal_render($form['title']) . '</td>';
  if (!empty($form['position'])) {
    $output .= '<td class="position">&nbsp;' . drupal_render($form['position']) . '</td>';
  }
  $output .= '<td class="operation">' . drupal_render($form['settings']);
  $output .= drupal_render($form['remove']) . '</td>';

  if ($with_tr) {
    $output = '<tr id="' . $type . '-row-' . $position . '" class="draggable ' . $type . '-row ' . ($count % 2 ? 'even' : 'odd') . '">' . $output . '</tr>';
  }
  return $output;
}

/**
 * Display the context item.
 */
function theme_ctools_context_item_form($form) {
  $output = '';
  $type   = $form['#ctools_context_type'];
  $module = $form['#ctools_context_module'];
  $name   = $form['#object_name'];

  $type_info = ctools_context_info($type);

  if (!empty($form[$type]) && empty($form['#only_buttons'])) {
    $count = 0;
    $rows = '';
    foreach (array_keys($form[$type]) as $id) {
      if (!is_numeric($id)) {
        continue;
      }
      $rows .= theme('ctools_context_item_row', $type, $form[$type][$id], $id, $count++);
    }

    $output .= '<table id="' . $type . '-table">';
    $output .= '<thead>';
    $output .= '<tr>';
    $output .= '<th class="title">' . $type_info['title'] . '</th>';
    if (!empty($type_info['sortable']) && $count) {
      $output .= '<th class="position">' . t('Weight') . '</th>';
    }
    $output .= '<th class="operation">' . t('Operation') . '</th>';
    $output .= '</tr>';
    $output .= '</thead>';
    $output .= '<tbody>';

    $output .= $rows;

    $output .= '</tbody>';
    $output .= '</table>';
  }

  if (!empty($form['buttons'])) {
    // Display the add context item.
    $row   = array();
    $row[] = array('data' => drupal_render($form['buttons'][$type]['item']), 'class' => 'title');
    $row[] = array('data' => drupal_render($form['buttons'][$type]['add']), 'class' => 'add', 'width' => "60%");
    $output .= '<div class="buttons">';
    $output .= drupal_render($form['buttons'][$type]);
    $output .= theme('table', array(), array($row), array('id' => $type . '-add-table'));
    $output .= '</div>';
  }
  if (!empty($form['description'])) {
    $output .= drupal_render($form['description']);
  }

  if (!empty($type_info['sortable'])) {
    drupal_add_tabledrag($type . '-table', 'order', 'sibling', 'drag-position');
  }

  return $output;
}

/**
 * Create a visible list of all the contexts available on an object.
 * Assumes arguments, relationships and context objects.
 *
 * Contexts must be preloaded.
 */
function theme_ctools_context_list($object, $header = '', $description = '') {
  $titles = array();
  $output = '';
  $count  = 1;

  $contexts = ctools_context_load_contexts($object);

  // Describe 'built in' contexts.
  if (!empty($object->base_contexts)) {
    foreach ($object->base_contexts as $id => $context) {
      $output .= '<tr>';
      $output .= '<td valign="top"><em>' . t('Built in context') . '</em></td>';
      $desc = check_plain($context->identifier);
      if (isset($context->keyword)) {
        $desc .= '<div class="description">' . t('Keyword: %@keyword', array('@keyword' => $context->keyword));
        foreach (ctools_context_get_converters('%' . $context->keyword . ':', $context) as $keyword => $title) {
          $desc .= '<br />' . t('@keyword --&gt; @title', array('@keyword' => $keyword, '@title' => $title));
        }
        $desc .= '</div>';

      }
      if (isset($context->description)) {
        $desc .= '<div class="description">' . filter_xss_admin($context->description) . '</div>';
      }
      $output .= '<td>' . $desc . '</td>';
      $output .= '</tr>';
      $titles[$id] = $context->identifier;
    }
  }

  // First, make a list of arguments. Arguments are pretty simple.
  if (!empty($object->arguments)) {
    foreach ($object->arguments as $argument) {
      $output .= '<tr>';
      $output .= '<td valign="top"><em>' . t('Argument @count', array('@count' => $count)) . '</em></td>';
      $desc = check_plain($argument['identifier']);
      if (isset($argument['keyword'])) {
        $desc .= '<div class="description">' . t('Keyword: %@keyword', array('@keyword' => $argument['keyword']));
        if (isset($contexts[ctools_context_id($argument, 'argument')])) {
          foreach (ctools_context_get_converters('%' . $argument['keyword'] . ':', $contexts[ctools_context_id($argument, 'argument')]) as $keyword => $title) {
            $desc .= '<br />' . t('@keyword --&gt; @title', array('@keyword' => $keyword, '@title' => $title));
          }
        }
        $desc .= '</div>';
      }
      $output .= '<td>' . $desc . '</td>';
      $output .= '</tr>';
      $titles[ctools_context_id($argument, 'argument')] = $argument['identifier'];
      $count++;
    }
  }

  $count = 1;
  // Then, make a nice list of contexts.
  if (!empty($object->contexts)) {
    foreach ($object->contexts as $context) {
      $output .= '<tr>';
      $output .= '<td valign="top"><em>' . t('Context @count', array('@count' => $count)) . '</em></td>';
      $desc = check_plain($context['identifier']);
      if (isset($context['keyword'])) {
        $desc .= '<div class="description">' . t('Keyword: %@keyword', array('@keyword' => $context['keyword']));
        foreach (ctools_context_get_converters('%' . $context['keyword'] . ':', $contexts[ctools_context_id($context, 'context')]) as $keyword => $title) {
          $desc .= '<br />' . t('@keyword --&gt; @title', array('@keyword' => $keyword, '@title' => $title));
        }
        $desc .= '</div>';
      }
      $output .= '<td>' . $desc . '</td>';
      $output .= '</tr>';
      $titles[ctools_context_id($context)] = $context['identifier'];
      $count++;
    }
  }

  // And relationships
  if (!empty($object->relationships)) {
    foreach ($object->relationships as $relationship) {
      $output .= '<tr>';
      if (is_array($relationship['context'])) {
        $rtitles = array();
        foreach ($relationship['context'] as $cid) {
          $rtitles[$cid] = $titles[$cid];
        }
        $title = implode(' + ', $rtitles);
      }
      else {
        $title = $titles[$relationship['context']];
      }
      $output .= '<td valign="top"><em>' . t('From "@title"', array('@title' => $title)) . '</em></td>';
      $desc = check_plain($relationship['identifier']);
      if (isset($relationship['keyword'])) {
        $desc .= '<div class="description">' . t('Keyword: %@keyword', array('@keyword' => $relationship['keyword']));
        foreach (ctools_context_get_converters('%' . $relationship['keyword'] . ':', $contexts[ctools_context_id($relationship, 'relationship')]) as $keyword => $title) {
          $desc .= '<br />' . t('@keyword --&gt; @title', array('@keyword' => $keyword, '@title' => $title));
        }
        $desc .= '</div>';
      }
      $output .= '<td>' . $desc . '</td>';
      $output .= '</tr>';
      $titles[ctools_context_id($relationship, 'relationship')] = $relationship['identifier'];
      $count++;
    }
  }

  $head = '';
  if ($header) {
    if ($description) {
      $header .= '<div class="description">' . $description . '</div>';
    }
    $head .= '<thead><tr>';
    $head .= '<th colspan="2">' . $header . '</th>';
    $head .= '</tr></thead>';
  }

  return $output ? "<table>$head<tbody>$output</tbody></table>\n" : "<table>$head</table>\n";
}

/**
 * ctools_context_list() but not in a table format because tabledrag
 * won't let us have tables within tables and still drag.
 */
function theme_ctools_context_list_no_table($object) {
  ctools_add_css('context');
  $titles = array();
  $output = '';
  $count  = 1;
  // Describe 'built in' contexts.
  if (!empty($object->base_contexts)) {
    foreach ($object->base_contexts as $id => $context) {
      $output .= '<div class="ctools-context-holder clear-block">';
      $output .= '<div class="ctools-context-title">' . t('Built in context') . '</div>';
      $desc = check_plain($context->identifier);
      if (isset($context->keyword)) {
        $desc .= '<div class="description">' . t('Keyword: %@keyword', array('@keyword' => $context->keyword)) . '</div>';
      }
      if (isset($context->description)) {
        $desc .= '<div class="description">' . filter_xss_admin($context->description) . '</div>';
      }
      $output .= '<div class="ctools-context-content">' . $desc . '</div>';
      $output .= '</div>';
      $titles[$id] = $context->identifier;
      $count++;
    }
  }

  // First, make a list of arguments. Arguments are pretty simple.
  if (!empty($object->arguments)) {
    foreach ($object->arguments as $argument) {
      $output .= '<div class="ctools-context-holder clear-block">';
      $output .= '<div class="ctools-context-title">' . t('Argument @count', array('@count' => $count)) . '</div>';
      $desc = check_plain($argument['identifier']);
      if (isset($argument['keyword'])) {
        $desc .= '<div class="description">' . t('Keyword: %@keyword', array('@keyword' => $argument['keyword'])) . '</div>';
      }
      $output .= '<div class="ctools-context-content">' . $desc . '</div>';
      $output .= '</div>';
      $titles[ctools_context_id($argument, 'argument')] = $argument['identifier'];
      $count++;
    }
  }
  $count = 1;
  // Then, make a nice list of contexts.
  if (!empty($object->contexts)) {
    foreach ($object->contexts as $context) {
      $output .= '<div class="ctools-context-holder clear-block">';
      $output .= '<div class="ctools-context-title">' . t('Context @count', array('@count' => $count)) . '</div>';
      $desc = check_plain($context['identifier']);
      if (isset($context['keyword'])) {
        $desc .= '<div class="description">' . t('Keyword: %@keyword', array('@keyword' => $context['keyword'])) . '</div>';
      }
      $output .= '<div class="ctools-context-content">' . $desc . '</div>';
      $output .= '</div>';
      $titles[ctools_context_id($context)] = $context['identifier'];
      $count++;
    }
  }
  // And relationships
  if (!empty($object->relationships)) {
    foreach ($object->relationships as $relationship) {
      $output .= '<div class="ctools-context-holder clear-block">';
      if (is_array($relationship['context'])) {
        $rtitles = array();
        foreach ($relationship['context'] as $cid) {
          $rtitles[$cid] = $titles[$cid];
        }
        $title = implode(' + ', $rtitles);
      }
      else {
        $title = $titles[$relationship['context']];
      }

      $output .= '<div class="ctools-context-title">' . t('From "@title"', array('@title' => $title)) . '</div>';
      $desc = check_plain($relationship['identifier']);
      if (isset($relationship['keyword'])) {
        $desc .= '<div class="description">' . t('Keyword: %@keyword', array('@keyword' => $relationship['keyword'])) . '</div>';
      }
      $output .= '<div class="ctools-context-content">' . $desc . '</div>';
      $output .= '</div>';
      $titles[ctools_context_id($relationship, 'relationship')] = $relationship['identifier'];
      $count++;
    }
  }

  return $output;
}


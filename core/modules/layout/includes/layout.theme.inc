<?php

/**
 * @file
 * Preprocess functions for page manager editing templates.
 */

/**
 * Preprocess the page manager edit page.
 */
function template_preprocess_layout_edit_page(&$vars) {

  return;
  $task = $vars['page']->task;

  $page = &$vars['page'];

  $vars['locked'] = '';
  $vars['changed'] = '';
  if (!empty($page->locked)) {
    $vars['locked'] = theme('page_manager_lock', array('page' => $page));
    $vars['changed'] = theme('page_manager_changed', array('text' => t('Locked'), 'description' => t('This page is being edited by another user and you cannot make changes to it.')));
  }
  else if (!empty($page->new)) {
    $vars['changed'] = theme('page_manager_changed', array('text' => t('New'), 'description' => t('This page is newly created and has not yet been saved to the database. It will not be available until you save it.')));
  }
  else if (!empty($page->changed)) {
    $vars['changed'] = theme('page_manager_changed', array('text' => t('Changed'), 'description' => t('This page has been modified, but these modifications are not yet live. While modifying this page, it is locked from modification by other users.')));
  }

  $form_state = array(
    'page' => &$vars['page'],
  );

  $active = $vars['content']['active'];
  if ($active[0] == 'handlers' && isset($vars['operations'][$active[1]])) {
    $vars['operations']['secondary'] = $vars['operations'][$active[1]];
  }
}

/**
 * Turn the rearrange form into a table with table sorting displayed.
 */
function theme_layout_reorder_form($variables) {
  $form = &$variables['form'];

  // Assemble the data for a table from everything in $form['handlers']
  foreach (element_children($form['layouts']) as $layout_name) {
    // provide a reference shortcut.
    $element = &$form['layouts'][$layout_name];
    if (isset($element['title'])) {
      $row = array();
      $row[] = array(
        'data' => render($element['title']),
        'class' => array('layout'),
      );
      $element['weight']['#attributes']['class'][] = 'weight';
      $row[] = render($element['weight']);
      $rows[] = array('data' => $row, 'class' => array('draggable'));
    }
  }

  $header = array(
    array('data' => t('Layout'), 'class' => array('layout')),
    t('Weight'),
  );

  drupal_add_tabledrag('layout-reorder', 'order', 'sibling', 'weight');

  $output = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'layout-reorder')));
  $output .= drupal_render_children($form);
  return $output;
}

/**
 * Outputs information about a layout. Used in the layout listing page.
 *
 * @param $variables
 *   An array with a single value: "layout", which contains the layout object.
 *
 * @return string
 */
function theme_layout_info($variables) {
  $layout = $variables['layout'];

  if ($layout->storage === LAYOUT_STORAGE_DEFAULT) {
    $storage = t('Module-provided default');
  }
  elseif ($layout->storage === LAYOUT_STORAGE_OVERRIDE) {
    $storage = t('User overriding default');
  }
  else {
    $storage = t('User-created');
  }

  $output = '';
  $output .= '<div class="layout-title">' . check_plain($layout->title) . '</div>';
  $output .= '<div class="layout-name">' . check_plain($layout->name) . '</div>';
  $output .= '<div class="layout-storage">' . $storage . '</div>';
  return $output;
}

/**
 * Outputs an individual option label for selecting a layout.
 */
function theme_layout_option($variables) {
  $layout_info = $variables['layout_info'];
  $icon_path = drupal_get_path('module', $layout_info['module']);
  $icon_path .= (isset($layout_info['file path']) ? '/' . $layout_info['file path'] : '');
  $icon_path .= '/' . $layout_info['icon'];

  $output = '';
  $output .= '<div class="layout-icon">';
  $output .= theme('image', array(
    'uri' => $icon_path,
    'attributes' => array('title' => $layout_info['title']),
  ));
  $output .= '<div class="caption">' . check_plain($layout_info['title']) . '</div>';
  $output .= '</div>';
  return $output;
}

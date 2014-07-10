<?php

/**
 * @file
 * Preprocess functions for page manager editing templates.
 */

/**
 * Preprocess the main Layout content edit page.
 */
function template_preprocess_layout_content_form(&$variables) {
  global $user;
  $layout = &$variables['form']['#layout'];

  if ($layout->locked && $user->uid !== $layout->locked->uid) {
    $account = user_load($layout->locked->uid);
    $lock_user = theme('username', array('account' => $account));
    $lock_age = format_interval(REQUEST_TIME - $layout->locked->updated);
    $lock_break_url = url('admin/structure/layouts/manage/' . $layout . '/break-lock', array('query' => array('token' => backdrop_get_token($layout->name))));

    $variables['locked'] = t('This layout is being edited by user !user, and is therefore locked from editing by others. This lock is !age old. Click here to <a href="!break">break this lock</a>.',  array('!user' => $lock_user, '!age' => $lock_age, '!break' => $lock_break_url));
  }
  else if (!empty($layout->changed)) {
    $variables['locked'] = t('This layout has been modified, but these modifications are not yet live. While modifying this layout, it is locked from modification by other users.');
  }
  else {
    $variables['locked'] = NULL;
  }

  // TODO Fill in the content
  $variables['content'] = NULL;
  $variables['operations'] = NULL;
}

/**
 * Turn the rearrange form into a table with table sorting displayed.
 */
function theme_layout_reorder_layouts($variables) {
  $element = $variables['element'];

  // Assemble the data for a table from everything in $form['handlers']
  foreach (element_children($element) as $layout_name) {
    if (isset($element[$layout_name]['title'])) {
      $row = array();
      $row[] = array(
        'data' => render($element[$layout_name]['title']),
        'class' => array('layout'),
      );
      $element[$layout_name]['weight']['#attributes']['class'][] = 'weight';
      $row[] = render($element[$layout_name]['weight']);
      $rows[] = array('data' => $row, 'class' => array('draggable'));
    }
  }

  $header = array(
    array('data' => t('Layout'), 'class' => array('layout')),
    t('Weight'),
  );

  backdrop_add_tabledrag('layout-reorder', 'order', 'sibling', 'weight');

  $output = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'layout-reorder')));
  $output .= backdrop_render_children($element);
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
  $icon_path = $layout_info['path'] . '/' . $layout_info['icon'];

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

/**
 * Output the arguments table for layout_menu_item_arguments_form().
 */
function theme_layout_menu_item_arguments_table($variables) {
  $element = $variables['element'];
  $path = $element['#layout_path'];
  $parts = explode('/', $path);
  $header = array(
    t('Argument position'),
    t('Context'),
    t('Operations'),
  );
  $rows = array();
  foreach (element_children($element) as $key) {
    $argument_parts = $parts;
    $argument_parts[$key] = '<strong>' . $argument_parts[$key] . '</strong>';
    $row = array();
    $row[] = t('Position @index:', array('@index' => $key + 1)) . ' ' . implode('/', $argument_parts);
    $row[] = backdrop_render($element[$key]['context']);
    $row[] = backdrop_render($element[$key]['operations']);
    $rows[] = $row;
  }
  return theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'layout-arguments')));
}

/**
 * Output the table for layout_settings_form() contexts.
 */
function theme_layout_settings_context_table($variables) {
  $element = $variables['element'];
  $path = $element['#layout_path'];
  $parts = explode('/', $path);
  $header = array(
    t('Conext'),
    t('Type'),
  );
  $rows = array();
  foreach (element_children($element) as $key) {
    $argument_parts = $parts;
    $argument_parts[$key] = '<strong>' . $argument_parts[$key] . '</strong>';
    $row = array();
    $row[] = t('Position @index:', array('@index' => $key + 1)) . ' ' . implode('/', $argument_parts);
    $row[] = backdrop_render($element[$key]['plugin']);
    $rows[] = $row;
  }
  return theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'layout-context-table')));
}


/**
 * Clean up the panel pane variables for the template.
 */
function template_preprocess_layout_block(&$variables) {
  $content = &$variables['content'];

  $variables['classes_array'] = array();
  $variables['title_prefix'] = '';
  $variables['title_suffix'] = '';

  if (module_exists('contextual') && user_access('access contextual links')) {
    $links = array();
    // These are specified by the content.
    if (!empty($content->admin_links)) {
      $links += $content->admin_links;
    }

    // Take any that may have been in the render array we were given and
    // move them up so they appear outside the pane properly.
    if (is_array($content->content) && isset($content->content['#contextual_links'])) {
      $element = array(
        '#type' => 'contextual_links',
        '#contextual_links' => $content->content['#contextual_links'],
      );
      unset($content->content['#contextual_links']);

      $element = contextual_pre_render_links($element);
      $links += $element['#links'];
    }

    if ($links) {
      $build = array(
        '#prefix' => '<div class="contextual-links-wrapper">',
        '#suffix' => '</div>',
        '#theme' => 'links__contextual',
        '#links' => $links,
        '#attributes' => array('class' => array('contextual-links')),
        '#attached' => array(
          'library' => array(array('contextual', 'contextual-links')),
        ),
      );
      $variables['classes_array'][] = 'contextual-links-region';
      $variables['title_prefix'] = drupal_render($build);
    }
  }

  // Basic classes.
  $variables['classes_array'][] = 'block';

  // Add custom classes if sent in.
  if (!empty($content->content)) {
    if (!empty($content->css_class)) {
      $variables['classes_array'] = array_merge(explode(' ', $content->css_class));
    }
  }

  // Set up some placeholders for constructing template file names.
  $base = 'layout_block';
  $delimiter = '__';

  // Add template file suggestion for content type and sub-type.
  $variables['theme_hook_suggestions'][] = $base . $delimiter . strtr($content->type, '-', '_');
  $variables['theme_hook_suggestions'][] = $base . $delimiter . strtr($content->type, '-', '_') . $delimiter . strtr($content->subtype, '-', '_');

  $variables['title'] = !empty($content->title) ? $content->title : '';

  $variables['content'] = !empty($content->content) ? $content->content : '';
}

/**
 * Clean up the panel pane variables for the template.
 */
function template_preprocess_layout_block_dynamic(&$variables) {
  // Re-use all the preprocessing provided by the default block template.
  template_preprocess_layout_block($variables);
  $variables['attributes_array'] = array();
  $variables['title_attributes_array'] = array();
  $variables['content_attributes_array'] = array();
  $variables['title_attributes_array']['class'][] = 'block-title';
}

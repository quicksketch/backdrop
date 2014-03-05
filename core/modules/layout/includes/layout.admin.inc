<?php

/**
 * @file
 * Administrative functions for Layout module.
 *
 * This provides the UI to list, create, edit and delete layouts.
 */

/**
 * Output a list of pages that are managed.
 */
function layout_list_page() {
  $layouts = layout_get_all_layouts();

  // Group layouts by path.
  foreach ($layouts as $layout) {
    $path_groups[$layout->path][] = $layout;
  }

  // Assmeble the rows of the table.
  $rows = array();
  foreach ($path_groups as $group) {
    // Print a row for rearranging a group of layouts.
    if (count($group) > 1) {
      $operations = array(
        '#type' => 'dropbutton',
        '#links' => array(
          'title' => t('Rearrange'),
          'href' => 'admin/structure/manage/layout/' . $layout->path . '/rearrange',
        ),
      );
      $row = array();
      $row[] = theme('layout_info', array('layout' => $layout));
      $row[] = check_plain($layout->path);
      $row[] = drupal_render($operations);
      $rows[] = $row;
    }
    foreach ($group as $layout) {
      $operations = array(
        '#type' => 'dropbutton',
        '#links' => _layout_get_operations($layout),
      );
      $row = array();
      $row[] = theme('layout_info', array('layout' => $layout));
      $row[] = strpos($layout->path, '%') === FALSE ? l($layout->path, $layout->path) : check_plain($layout->path);
      $row[] = drupal_render($operations);
      $rows[] = $row;
    }
  }

  $header = array(
    array('data' => t('Name'), 'class' => array('layout-name')),
    array('data' => t('Path'), 'class' => array('layout-path')),
    array('data' => t('Operations'), 'class' => array('layout-operations')),
  );

  return theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => 'layout-list'),
  ));
}

/**
 * Given a layout, return a list of operations that can be performed on it.
 */
function _layout_get_operations(Layout $layout) {
  $links = array();
  if ($layout->disabled) {
    $links['enable'] = array(
      'title' => t('Enable'),
      'href' => 'admin/structure/layouts/manage/' . $layout->name . '/enable',
      'query' => array('token' => drupal_get_token($layout->name)),
    );
  }
  $links['edit'] = array(
    'title' => t('Edit'),
    'href' => 'admin/structure/layouts/manage/' . $layout->name,
  );
  $links['clone'] = array(
    'title' => t('Clone'),
    'href' => 'admin/structure/layouts/manage/' . $layout->name . '/clone',
  );
  if (!$layout->disabled) {
    $links['disable'] = array(
      'title' => t('Disable'),
      'href' => 'admin/structure/layouts/manage/' . $layout->name . '/disable',
      'query' => array('token' => drupal_get_token($layout->name)),
    );
  }
  if ($layout->storage == LAYOUT_STORAGE_NORMAL) {
    $links['delete'] = array(
      'title' => t('Delete'),
      'href' => 'admin/structure/layouts/manage/' . $layout->name . '/delete',
    );
  }
  elseif ($layout->storage == LAYOUT_STORAGE_OVERRIDE) {
    $links['revert'] = array(
      'title' => t('Revert'),
      'href' => 'admin/structure/layouts/manage/' . $layout->name . '/revert',
    );
  }
  return $links;
}

/**
 * Menu callback; Display the form for adding a new layout.
 */
function layout_add_page() {
  if (isset($_SESSION['layout_new_name'])) {
    $layout = layout_cache_load($_SESSION['layout_new_name']);
  }
  if (empty($layout)) {
    $config = array(
      'is_new' => TRUE,
    );
    $layout = new Layout($config);
  }
  return drupal_get_form('layout_settings_form', $layout);
}

/**
 * Render the settings form for layout.
 */
function layout_settings_form($form, &$form_state, Layout $layout) {
  $form_state['layout'] = &$layout;
  $form['#tree'] = TRUE;

  $form['#attached']['css'][] = drupal_get_path('module', 'layout') . '/css/layout-admin.css';

  $form['title'] = array(
    '#title' => t('Layout title'),
    '#type' => 'textfield',
    '#maxlength' => 128,
    '#default_value' => $layout->title,
  );
  $form['name'] = array(
    '#type' => 'machine_name',
    '#machine_name' => array(
      'source' => array('title'),
      'exists' => 'layout_load',
    ),
    '#maxlength' => 128,
    '#default_value' => $layout->name,
  );
  $form['layout'] = array(
    '#title' => t('Layout'),
    '#type' => 'radios',
    '#default_value' => $layout->layout,
    '#options' => array(),
    '#wrapper_attributes' => array('class' => array('clearfix', 'layout-options')),
  );
  // Get the list of layout options.
  $all_layout_info = layout_get_layout_info();
  foreach ($all_layout_info as $layout_name => $layout_info) {
    $form['layout']['#options'][$layout_name] = theme('layout_option', array('layout_info' => $layout_info));
  }

  $form['path'] = array(
    '#title' => t('Path'),
    '#type' => 'textfield',
    '#default_value' => $layout->path,
    '#description' => t('Use the "%" (percent) symbol to indicate a wildcard in the path. i.e. "node/%" or "node/%/edit".'),
  );
  $form['path_update'] = array(
    '#type' => 'submit',
    '#value' => t('Check path'),
    '#validate' => array(
      'layout_settings_form_validate',
    ),
    '#submit' => array(
      'layout_settings_form_update_layout',
      'layout_settings_form_path_check'
    ),
    '#limit_validation' => array('path'),
  );
  $form['context'] = array(
    '#title' => t('Contexts'),
    '#theme_wrappers' => array('form_element'),
    '#description' => t('Contexts define what blocks are available within this layout. Most wildcards in the path will be associated with a context automatically.')
  );

  $all_context_info = layout_get_context_info();
  foreach ($all_context_info as $context_name => $context_info) {
    $context_options[$context_name] = $context_info['title'];
  }

  foreach ($layout->context as $context_key => $layout_context) {
    if ($layout_context->locked) {
      $form['context']['active'][$context_key]['label'] = array(
        '#type' => 'item',
        '#title' => t('Position @count', array('@count' => $layout_context->position + 1)),
        '#markup' => $layout_context->humanLabel(),
      );
    }
    elseif ($layout_context->required) {
      $form['context']['active'][$context_key]['type'] = array(
        '#type' => 'select',
        '#title' => t('Position @count', array('@count' => $layout_context->position + 1)),
        '#options' => $context_options,
        '#default_value' => $layout_context->type(),
        '#description' => t('The wildcard in position @count of the path cannot be automatically determined. Select a context or use a string pass-through to ignore this wildcard.', array('@count' => $layout_context->position + 1)),
      );
    }
    else {
      $extra_context_count = !isset($extra_context_count) ? 1 : $extra_context_count + 1;
      $form['context']['active'][$context_key]['label'] = array(
        '#type' => 'item',
        '#title' => t('Additional context @count', array('@count' => $extra_context_count)),
        '#markup' => $layout_context->humanLabel(),
      );
      $form['context']['active'][$context_key]['configure'] = array(
        '#type' => 'submit',
        '#value' => t('Configure'),
        '#layout_form' => 'layout_context_form',
        '#layout_form_arguments' => array($context_key),
        '#validate' => array(
          'layout_settings_form_validate',
        ),
        '#submit' => array(
          'layout_settings_form_update_layout',
          'layout_settings_form_open_child_form',
        ),
      );
      $form['context']['active'][$context_key]['delete'] = array(
        '#type' => 'submit',
        '#value' => t('Delete'),
        '#validate' => array(
          'layout_settings_form_validate',
        ),
        '#submit' => array(
          'layout_settings_form_update_layout',
          'layout_settings_form_context_remove',
        ),
      );
    }
  }
  $form['context']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add context'),
    '#validate' => array(
      'layout_settings_form_validate',
    ),
    '#submit' => array(
      'layout_settings_form_update_layout',
      'layout_settings_form_context_add',
    ),
    '#limit_validation' => array('context'),
  );

  $form['access'] = array(
    '#title' => t('Conditions'),
    '#theme_wrappers' => array('form_element'),
    '#description' => t('Conditions allow this layout to selectively apply to different situations, such as different types of content at the same wildcard path.'),
  );
  foreach ($layout->access as $access_key => $layout_access) {
    $form['access']['active'][$access_key]['label'] = array(
      '#markup' => $layout_access->humanLabel(),
    );
    $form['access']['active'][$access_key]['remove'] = array(
      '#type' => 'submit',
      '#value' => t('Remove'),
      '#validate' => array(
        'layout_settings_form_validate',
      ),
      '#submit' => array(
        'layout_settings_form_update_layout',
        'layout_settings_form_condition_remove',
      ),
      '#limit_validation' => array('conditions'),
    );
  }
  $form['access']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add condition'),
    '#validate' => array(
      'layout_settings_form_validate',
    ),
    '#submit' => array(
      'layout_settings_form_update_layout',
      'layout_settings_form_condition_add',
    ),
    '#limit_validation' => array('conditions'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => empty($layout->is_new) ? t('Save settings') : t('Create layout'),
    '#validate' => array(
      'layout_settings_form_validate',
    ),
    '#submit' => array(
      'layout_settings_form_save_submit',
    ),
  );
  if (isset($_SESSION['layout_new_name'])) {
    $form['actions']['reset'] = array(
      '#type' => 'submit',
      '#value' => t('Reset'),
      '#limit_validation' => array('actions', 'reset'),
      '#validate' => array(),
      '#submit' => array(
        'layout_settings_form_reset',
      ),
    );
    $form['message'] = array(
      '#markup' => '<div class="messages warning">' . t('This layout has unsaved changes. Click "@save" to make changes permanent or "@reset" to discard changes.', array('@save' => $form['actions']['submit']['#value'], '@reset' => $form['actions']['reset']['#value'])) . '</div>',
      '#weight' => -100,
    );
  }

  return $form;
}

/**
 * Validates layout_settings_form(), ensuring a valid path.
 */
function layout_settings_form_validate(&$form, &$form_state) {
  $layout = $form_state['layout'];
  $name = !empty($form_state['values']['name']) ? $form_state['values']['name'] : $layout->name;
  if (empty($name)) {
    form_error($form['name'], t('Name is required.'));
  }

  $path = $form_state['values']['path'];
  if (empty($form_state['values']['path'])) {
    form_error($form['path'], t('Path is required.'));
    // Stop validation here if there is no path.
    return;
  }

  // Remove trailing and preceding slashes.
  $path = $form_state['values']['path'] = trim($path, '/');

  if (strpos($path, '%') === 0) {
    form_error($form['path'], t('The first part of a path may not be a wildcard.'));
  }

  // Ensure the path is not already an alias to something else.
  if (strpos($path, '%') === FALSE) {
    $alias = db_query('SELECT * FROM {url_alias} WHERE alias = :path', array(':path' => $path))->fetchObject();
    if ($alias) {
      $error = t('That path is currently assigned to be an alias for "@alias".', array('@alias' => $alias->source));
      if (user_access('administer url aliases')) {
        $error .= ' ' . t('<a href="!url">Delete the alias first</a>, then save this layout.', array('!url' => url('admin/config/search/path/delete/' . $alias->pid, array('query' => array('destination' => $_GET['q'])))));
      }
      form_error($form['path'], $error);
    }
  }

  // Ensure path is properly formed.
  $parts = explode('/', $path);
  foreach ($parts as $part) {
    $wildcard_position = strpos($part, '%');
    if ($wildcard_position !== FALSE && ($wildcard_position > 0 || strlen($part))) {
      form_error($form['path'], t('Wildcard placeholders must be the only character in their part of the path i.e. "@part" should be "%".', array('@part' => $part)));
    }
  }
}

/**
 * Submit handler for layout_settings_form() that updates available contexts.
 */
function layout_settings_form_path_check($form, &$form_state) {
  $layout = $form_state['layout'];

  // Get the list of new required contexts that are associate with the path.
  module_load_include('inc', 'layout', 'includes/layout.context');
  $required_contexts = layout_context_required_by_path($form_state['values']['path']);

  // Remove the existing required contexts that no longer apply.
  foreach ($layout->context as $context_key => $context) {
    if ($context->required && !array_key_exists($context_key, $required_contexts)) {
      unset($layout->context[$context_key]);
    }
  }
  // Add new required contexts.
  foreach ($required_contexts as $context_key => $required_context) {
    if (!isset($layout->context[$context_key])) {
      $layout->context[$context_key] = $required_context;
    }
  }
  ksort($layout->context);

  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for layout_settings_form() that lets the user add a context.
 */
function layout_settings_form_context_add($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for layout_settings_form() that removes a context.
 */
function layout_settings_form_context_remove($form, &$form_state) {
  $removed_context = $form_state['clicked_button']['#array_parents'][1];
  unset($form_state['layout']->context[$removed_context]);
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for layout_settings_form() that lets the user add a context.
 */
function layout_settings_form_condition_add($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for layout_settings_form() that removes an access item.
 */
function layout_settings_form_condition_remove($form, &$form_state) {
  $removed_condition = $form_state['clicked_button']['#array_parents'][1];
  unset($form_state['layout']->access[$removed_condition]);
}

/**
 * Submit handler for layout_settings_form() that saves in-progress values.
 */
function layout_settings_form_update_layout($form, &$form_state) {
  $layout = $form_state['layout'];

  $layout->title = $form_state['values']['title'];
  $layout->name = $form_state['values']['name'];
  $layout->layout = $form_state['values']['layout'];
  $layout->path = $form_state['values']['path'];

  if (!empty($layout->is_new)) {
    $_SESSION['layout_new_name'] = $layout->name;
  }
  else {
    if ($form_state['values']['name'] !== $layout->name) {
      $layout->original_name = $form_state['values']['name'];
    }
  }
  layout_set_layout_cache($layout);
}

/**
 * Submit handler for layout_settings_form() that resets in-progress changes.
 */
function layout_settings_form_reset($form, &$form_state) {
  $layout = $form_state['layout'];
  if (isset($_SESSION['layout_new_name'])) {
    unset($_SESSION['layout_new_name']);
  }
  layout_clear_layout_cache($layout->name);
  drupal_set_message(t('Layout changes discarded.'));
  if (empty($layout->is_new)) {
    $form_state['redirect'] = 'admin/structure/layouts';
  }
}

/**
 * Submit handler for layout_settings_form() that saves in-progress changes.
 */
function layout_settings_form_save_submit($form, &$form_state) {
  $layout = $form_state['layout'];
  if (isset($_SESSION['layout_new_name'])) {
    unset($_SESSION['layout_new_name']);
  }
  layout_clear_layout_cache($layout->name);
  $layout->save();
  drupal_set_message(t('Layout saved.'));
  $form_state['redirect'] = 'admin/structure/layouts';
}

/**
 * Entry point to edit a single operation for a page.
 *
 * @param $js
 *   Whether or not the page was called via javascript.
 * @param $page
 *   The cached page that is being edited.
 * @param ...
 *   A number of items used to drill down into the actual operation called.
 */
function layout_edit_page_operation() {
  $args = func_get_args();
  $js = array_shift($args);
  $page = array_shift($args);

  $operations = layout_get_operations($page);
  $content = layout_get_operation_content($js, $page, $args, $operations);

  // If the operation requested we go somewhere else afterward, oblige it.
  if (isset($content['new trail'])) {
    $args = $content['new trail'];
    // Get operations again, for the operation may have changed their availability.
    $operations = layout_get_operations($page);
    $content = layout_get_operation_content($js, $page, $args, $operations);
  }

  // Rendering the content may have been a form submission that changed the
  // operations, such as renaming or adding a handler. Thus we get a new set
  // of operations.
  $operations = layout_get_operations($page);
  $rendered_operations = layout_render_operations($page, $operations, $args, array('class' => array('operations-main')), 'nav');

  // Since this form should never be submitted to this page, process it late so
  // that we can be sure it notices changes.
  $form_state = array('page' => &$page);
  $built_form = drupal_build_form('layout_save_page_form', $form_state);
  $form = drupal_render($built_form);

  $output = theme('layout_edit_page', array('page' => $page, 'save' => $form, 'operations' => $rendered_operations, 'content' => $content));

  if ($js) {
    $commands = array();
    $commands[] = ajax_command_replace('#page-manager-edit', $output);

    print ajax_render($commands);
    ajax_footer();
    return;
  }

  drupal_set_title($page->subtask['admin title']);
  return $output;
}

/**
 * Take the operations array from a task and expand it.
 *
 * This allows some of the operations to be dynamic, based upon settings
 * on the task or the task's handlers. Each operation should have a type. In
 * addition to all the types allowed in layout_render_operations, these
 * types will be dynamically replaced with something else:
 * - 'handlers': An automatically created group that contains all the task's
 *   handlers and appropriate links.
 * - 'function': A callback (which will be placed in the 'function' parameter
 *   that should return an array of operations. This can be used to provide
 *   additional, dynamic links if needed.
 */
function layout_get_operations($page, $operations = NULL) {
  if (!isset($operations)) {
    // All tasks have at least these 2 ops:
    $operations = array(
      'summary' => array(
        'title' => t('Summary'),
        'description' => t('Get a summary of the information about this page.'),
        'path' => 'admin/structure/pages/edit',
        'ajax' => FALSE,
        'no operations' => TRUE,
        'form info' => array(
          'no buttons' => TRUE,
        ),
        'form' => 'layout_page_summary',
      ),
      'actions' => array(
        'type' => 'group',
        'title' => '',
        'class' => array('operations-actions'),
        'location' => 'primary',
        'children' => array(),
      ),
    );

    if (isset($page->subtask['operations'])) {
      $operations += $page->subtask['operations'];
      // add actions separately.
      if (!empty($page->subtask['operations']['actions'])) {
        $operations['actions']['children'] += $page->subtask['operations']['actions']['children'];
      }
    }
    $operations['handlers'] = array('type' => 'handlers');
  }

  $result = array();
  foreach ($operations as $id => $operation) {
    if (empty($operation['type'])) {
      $operation['type'] = 'operation';
    }
    switch ($operation['type']) {
      case 'handlers':
        $result[$id] = layout_get_handler_operations($page);
        break;
      case 'function':
        if (function_exists($operation['function'])) {
          $retval = $function($page, $operation);
          if (is_array($retval)) {
            $result[$id] = $retval;
          }
        }
        break;
      default:
        $result[$id] = $operation;
    }
  }

  if (!empty($page->subtask['enable callback']) && !empty($page->subtask['disabled']) && empty($result['actions']['children']['enable'])) {
    $result['actions']['children']['enable'] = array(
      'title' => t('Enable'),
      'description' => t('Activate this page so that it will be in use in your system.'),
      'form' => 'layout_enable_form',
      'ajax' => FALSE,
      'silent' => TRUE,
      'no update and save' => TRUE,
      'form info' => array(
        'finish text' => t('Enable'),
      ),
    );
  }

  if (!empty($page->subtask['enable callback']) && empty($page->subtask['disabled']) && empty($result['actions']['children']['disable'])) {
    $result['actions']['children']['disable'] = array(
      'title' => t('Disable'),
      'description' => t('De-activate this page. The data will remain but the page will not be in use on your system.'),
      'form' => 'layout_disable_form',
      'ajax' => FALSE,
      'silent' => TRUE,
      'no update and save' => TRUE,
      'form info' => array(
        'finish text' => t('Disable'),
      ),
    );
  }

  $result['actions']['children']['add'] = array(
    'title' => t('Add variant'),
    'description' => t('Add a new variant to this page.'),
    'form' => 'layout_handler_add',
    'ajax' => FALSE,
    'silent' => TRUE, // prevents a message about updating and prevents this item from showing as changed.
    'no update and save' => TRUE, // get rid of update and save button which is bad here.
    'form info' => array(
      'finish text' => t('Create variant'),
    ),
  );

  // Restrict variant import to users who can already execute arbitrary PHP
  if (user_access('use PHP for settings')) {
    $result['actions']['children']['import'] = array(
      'title' => t('Import variant'),
      'description' => t('Add a new variant to this page from code exported from another page.'),
      'form' => 'layout_handler_import',
    );
  }

  if (count($page->handlers) > 1) {
    $result['actions']['children']['rearrange'] = array(
      'title' => t('Reorder variants'),
      'ajax' => FALSE,
      'description' => t('Change the priority of the variants to ensure that the right one gets selected.'),
      'form' => 'layout_handler_rearrange',
    );
  }

  // This is a special operation used to configure a new task handler before
  // it is added.
  if (isset($page->new_handler)) {
    $plugin = layout_get_task_handler($page->new_handler->handler);
    $result['actions']['children']['configure'] = array(
      'title' => t('Configure'),
      'description' => t('Configure a newly created variant prior to actually adding it to the page.'),
      'ajax' => FALSE,
      'no update and save' => TRUE, // get rid of update and save button which is bad here.
      'form info' => array(
        // We use our own cancel and finish callback to handle the fun stuff.
        'finish callback' => 'layout_handler_add_finish',
        'cancel callback' => 'layout_handler_add_cancel',
        'show trail' => TRUE,
        'show back' => TRUE,
        'finish text' => t('Create variant'),
      ),
      'form' => array(
        'forms' => $plugin['forms'],
      ),
    );

    foreach ($page->forms as $id) {
      if (isset($plugin['add features'][$id])) {
        $result['actions']['children']['configure']['form']['order'][$id] = $plugin['add features'][$id];
      }
      else if (isset($plugin['required forms'][$id])) {
        $result['actions']['children']['configure']['form']['order'][$id] = $plugin['required forms'][$id];
      }
    }
  }

  if ($page->locked) {
    $result['actions']['children']['break-lock'] = array(
      'title' => t('Break lock'),
      'description' => t('Break the lock on this page so that you can edit it.'),
      'form' => 'layout_break_lock',
      'ajax' => FALSE,
      'no update and save' => TRUE, // get rid of update and save button which is bad here.
      'form info' => array(
        'finish text' => t('Break lock'),
      ),
      'even locked' => TRUE, // show button even if locked
      'silent' => TRUE,
    );
  }

  drupal_alter('layout_operations', $result, $page);
  return $result;
}

/**
 * Collect all the operations related to task handlers (variants) and
 * build a menu.
 */
function layout_get_handler_operations(&$page) {
  ctools_include('export');
  $group = array(
    'type' => 'group',
    'class' => array('operations-handlers'),
    'title' => t('Variants'),
  );

  $operations = array();

  // If there is only one variant, let's not have it collapsible.
  $collapsible = count($page->handler_info) != 1;
  foreach ($page->handler_info as $id => $info) {
    if ($info['changed'] & layout_CHANGED_DELETED) {
      continue;
    }
    $handler = $page->handlers[$id];
    $plugin = layout_get_task_handler($handler->handler);

    $operations[$id] = array(
      'type' => 'group',
      'class' => array('operations-handlers-' . $id),
      'title' => layout_get_handler_title($plugin, $handler, $page->task, $page->subtask_id),
      'collapsible' => $collapsible,
      'children' => array(),
    );

    $operations[$id]['children']['actions'] = array(
      'type' => 'group',
      'class' => array('operations-handlers-actions-' . $id),
      'title' => t('Variant operations'),
      'children' => array(),
      'location' => $id,
    );

    // There needs to be a 'summary' item here for variants.
    $operations[$id]['children']['summary'] = array(
      'title' => t('Summary'),
      'description' => t('Get a summary of the information about this variant.'),
      'form info' => array(
        'no buttons' => TRUE,
      ),
      'form' => 'layout_handler_summary',
    );

    if ($plugin && isset($plugin['operations'])) {
      $operations[$id]['children'] += $plugin['operations'];
    }

    $actions = &$operations[$id]['children']['actions']['children'];

    $actions['clone'] = array(
      'title' => t('Clone'),
      'description' => t('Make an exact copy of this variant.'),
      'form' => 'layout_handler_clone',
    );
    $actions['export'] = array(
      'title' => t('Export'),
      'description' => t('Export this variant into code to import into another page.'),
      'form' => 'layout_handler_export',
    );
    if ($handler->export_type == (EXPORT_IN_CODE | EXPORT_IN_DATABASE)) {
      $actions['delete'] = array(
        'title' => t('Revert'),
        'description' => t('Remove all changes to this variant and revert to the version in code.'),
        'form' => 'layout_handler_delete',
        'no update and save' => TRUE,
        'form info' => array(
          'finish text' => t('Revert'),
        ),
      );
    }
    else if ($handler->export_type != EXPORT_IN_CODE) {
      $actions['delete'] = array(
        'title' => t('Delete'),
        'description' => t('Remove this variant from the page completely.'),
        'form' => 'layout_handler_delete',
        'form info' => array(
          'finish text' => t('Delete'),
          'save text' => t('Delete and save'),
        ),
      );
    }
    if (!empty($handler->disabled)) {
      $actions['enable'] = array(
        'title' => t('Enable'),
        'description' => t('Activate this variant so that it will be in use in your system.'),
        'form' => 'layout_handler_enable',
        'silent' => TRUE,
        'form info' => array(
          'finish text' => t('Enable'),
          'save text' => t('Enable and save'),
        ),
      );
    }
    else {
      $actions['disable'] = array(
        'title' => t('Disable'),
        'description' => t('De-activate this variant. The data will remain but the variant will not be in use on your system.'),
        'form' => 'layout_handler_disable',
        'silent' => TRUE,
        'form info' => array(
          'finish text' => t('Disable'),
          'save text' => t('Disable and save'),
        ),
      );
    }

    drupal_alter('layout_variant_operations', $operations[$id], $handler);
  }
  if (empty($operations)) {
    $operations['empty'] = array(
      'type' => 'text',
      'title' => t('No variants'),
    );
  }

  $group['children'] = $operations;
  return $group;
}

/**
 * Get an operation from a trail.
 *
 * @return array($operation, $active, $args)
 */
function layout_get_operation($operations, $trail) {
  $args = $trail;
  $stop = FALSE;
  $active = array();
  $titles = array();
  // Drill down into operations array:
  while (!$stop) {
    $check = reset($args);
    $stop = TRUE;
    if (is_array($operations)) {
      if (isset($operations[$check])) {
        $active[] = $check;
        $operation = array_shift($args);
        // check to see if this operation has children. If so, we continue.
        if (!isset($operations[$check]['children'])) {
          $operations = $operations[$check];
        }
        else {
          $titles[] = $operations[$check]['title'];
          $operations = $operations[$check]['children'];
          // continue only if the operation hs children.
          $stop = FALSE;
        }
      }
    }
  }

  return array($operations, $active, $args, $titles);
}

/**
 * Fetch the content for an operation.
 *
 * First, this drills down through the arguments to find the operation, and
 * turns whatever it finds into the active trail which is then used to
 * hilite where we are when rendering the operation list.
 *
 * The arguments are discovered from the URL, and are an exact match for where
 * the operation is in the hierarchy. For example, handlers/foo/settings will
 * be the operation to edit the settings for the handler named foo. This comes
 * in as an array ('handlers', 'foo', 'settings') and is used to find where the
 * data for that operation is in the array.
 */
function layout_get_operation_content($js, &$page, $trail, $operations) {
  list($operation, $active, $args, $titles) = layout_get_operation($operations, $trail);
  // Once we've found the operation, send it off to render.
  if ($operation) {
    $content = _layout_get_operation_content($js, $page, $active, $operation, $titles, $args);
  }

  if (empty($content)) {
    $content = _layout_get_operation_content($js, $page, array('summary'), $operations['summary']);
  }

  return $content;
}

/**
 * Provide a simple form for saving the page manager info out of the cache.
 */
function layout_save_page_form($form, &$form_state) {
  if (!empty($form_state['page']->changed)) {
    $form['markup'] = array(
      '#markup' => '<div class="changed-notification">' . t('You have unsaved changes to this page. You must select Save to write them to the database, or Cancel to discard these changes. Please note that if you have changed any form, you must submit that form before saving.') . '</div>',
    );

    // Always make sure we submit back to the proper page.
    $form['#action'] = url('admin/structure/pages/edit/' . $form_state['page']->task_name);
    $form['save'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#submit' => array('layout_save_page_form_submit'),
    );

    $form['cancel'] = array(
      '#type' => 'submit',
      '#value' => t('Cancel'),
      '#submit' => array('layout_save_page_form_cancel'),
    );
    return $form;
  }
}

// --------------------------------------------------------------------------
// Handler (variant) related forms.

/**
 * Add a new task handler.
 */
function layout_handler_add($form, &$form_state) {
  // Get a list of possible task handlers for this task.
  return layout_handler_add_form($form, $form_state);
}

/**
 * Handler related forms.
 */
function layout_handler_add_submit(&$form, &$form_state) {
  $cache = $form_state['page'];
  $plugin = layout_get_task_handler($form_state['values']['handler']);

  // Create a new handler.
  $handler = layout_new_task_handler($plugin);
  if (!empty($form_state['values']['title'])) {
    $handler->conf['title'] = $form_state['values']['title'];
  }
  else {
    $handler->conf['title'] = $plugin['title'];
  }
  $cache->new_handler = $handler;

  // Figure out which forms to present them with
  $cache->forms = array();

  $features = $form_state['values']['features'];
  if (isset($features[$form_state['values']['handler']])) {
    $cache->forms = array_merge($cache->forms, array_keys(array_filter($features[$form_state['values']['handler']])));
  }

  if (isset($plugin['required forms'])) {
    $cache->forms = array_merge($cache->forms, array_keys($plugin['required forms']));
  }

  $form_state['no_rerender'] = TRUE;
  if (!empty($cache->forms)) {
    // Tell the form to go to the config page.
    drupal_set_message(t('Before this variant can be added, it must be configured. When you are finished, click "Create variant" at the end of this wizard to add this to your page.'));
    $form_state['new trail'] = array('actions', 'configure');
  }
  else {
    // It has no forms at all. Add the variant and go to its first operation.
    layout_handler_add_finish($form_state);
  }
}

/**
 * Finish the add process and make the new handler official.
 */
function layout_handler_add_finish(&$form_state) {
  $page = &$form_state['page'];
  $handler = $page->new_handler;
  layout_handler_add_to_page($page, $handler);

  // Remove the temporary page.
  unset($page->new_handler);
  unset($page->forms);

  // Set the new destination
  $plugin = layout_get_task_handler($handler->handler);
  if (!empty($plugin['add finish'])) {
    $location = $plugin['add finish'];
  }
  else {
    $keys = array_keys($plugin['operations']);
    $location = reset($keys);
  }

  $form_state['new trail'] = array('handlers', $handler->name, $location);

  // Pass through.
  layout_edit_page_finish($form_state);
}

/**
 * Rearrange the order of variants.
 */
function layout_handler_import($form, &$form_state) {
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Variant name'),
    '#description' => t('Enter the name of the new variant.'),
  );

  if (user_access('use PHP for settings')) {
    $form['object'] = array(
      '#type' => 'textarea',
      '#title' => t('Paste variant code here'),
      '#rows' => 15,
    );
  }
  // Users ordinarily can't get here without the PHP block visibility perm.
  // In case they somehow do, though, disable the form widget for extra safety.
  else {
    $form['shoveoff'] = array(
      '#markup' => '<div>' . t('You do not have sufficient permissions to perform this action.') . '</div>',
    );
  }

  return $form;
}

/**
 * Make sure that an import actually provides a handler.
 */
function layout_handler_import_validate($form, &$form_state) {
  if (!user_access('use PHP for settings')) {
    form_error($form['shoveoff'], t('You account permissions do not permit you to import.'));
    return;
  }
  ob_start();
  eval($form_state['values']['object']);
  ob_end_clean();

  if (empty($handler)) {
    $errors = ob_get_contents();
    if (empty($errors)) {
      $errors = t('No variant found.');
    }

    form_error($form['object'], t('Unable to get a variant from the import. Errors reported: @errors', array('@errors' => $errors)));
  }

  $form_state['handler'] = $handler;
}

/**
 * Clone an existing task handler into a new handler.
 */
function layout_handler_import_submit(&$form, &$form_state) {
  $handler = $form_state['handler'];

  layout_handler_add_to_page($form_state['page'], $handler, $form_state['values']['title']);

  $plugin = layout_get_task_handler($handler->handler);
  // It has no forms at all. Add the variant and go to its first operation.
  $keys = array_keys($plugin['operations']);
  $form_state['new trail'] = array('handlers', $handler->name, reset($keys));
}

/**
 * Rearrange the order of variants.
 */
function layout_handler_rearrange($form, &$form_state) {
  $page = $form_state['page'];

  $form['handlers'] = array('#tree' => TRUE);

  foreach ($page->handler_info as $id => $info) {
    if ($info['changed'] & layout_CHANGED_DELETED) {
      continue;
    }
    $handler = $page->handlers[$id];
    $plugin = layout_get_task_handler($handler->handler);

    $form['handlers'][$id]['title'] = array(
      '#markup' => layout_get_handler_title($plugin, $handler, $page->task, $page->subtask_id),
    );

    $form['handlers'][$id]['weight'] = array(
      '#type' => 'weight',
      '#default_value' => $info['weight'],
      '#delta' => 30,
    );
  }

  return $form;
}

function layout_handler_rearrange_submit(&$form, &$form_state) {
  $handler_info = &$form_state['page']->handler_info;

  foreach ($form_state['values']['handlers'] as $id => $info) {
    if ($handler_info[$id]['weight'] = $info['weight']) {
      $handler_info[$id]['weight'] = $info['weight'];
      $handler_info[$id]['changed'] |= layout_CHANGED_MOVED;
    }
  }

  // Sort the new cache.
  uasort($handler_info, '_layout_handler_sort');

}

/**
 * Used as a callback to uasort to sort the task cache by weight.
 *
 * The 'name' field is used as a backup when weights are the same, which
 * can happen when multiple modules put items out there at the same
 * weight.
 */
function _layout_handler_sort($a, $b) {
  if ($a['weight'] < $b['weight']) {
    return -1;
  }
  elseif ($a['weight'] > $b['weight']) {
    return 1;
  }
  elseif ($a['name'] < $b['name']) {
    return -1;
  }
  elseif ($a['name'] > $b['name']) {
    return 1;
  }
}

/**
 * Rearrange the order of variants.
 */
function layout_handler_delete($form, &$form_state) {
  if ($form_state['handler']->type == t('Overridden')) {
    $text = t('Reverting the variant will delete the variant that is in the database, reverting it to the original default variant. This deletion will not be made permanent until you click Save.');
  }
  else {
    $text = t('Are you sure you want to delete this variant? This deletion will not be made permanent until you click Save.');
  }
  $form['markup'] = array(
    '#markup' => '<p>' . $text . '</p>',
  );

  return $form;
}

/**
 * Submit handler to delete a view.
 */
function layout_handler_delete_submit(&$form, &$form_state) {
  $form_state['page']->handler_info[$form_state['handler_id']]['changed'] |= layout_CHANGED_DELETED;
  $form_state['new trail'] = array('summary');
}

/**
 * Entry point to export a page.
 */
function layout_handler_export($form, &$form_state) {
  $export = layout_export_task_handler($form_state['handler']);

  $lines = substr_count($export, "\n");
  $form['code'] = array(
    '#type' => 'textarea',
    '#default_value' => $export,
    '#rows' => $lines,
  );

  unset($form['buttons']);
  return $form;
}

/**
 * Rearrange the order of variants.
 */
function layout_handler_clone($form, &$form_state) {
  // This provides its own button because it does something totally different.
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Variant name'),
    '#description' => t('Enter the name of the new variant.'),
  );

  return $form;
}

/**
 * Clone an existing task handler into a new handler.
 */
function layout_handler_clone_submit(&$form, &$form_state) {
  $export = layout_export_task_handler($form_state['handler']);
  ob_start();
  eval($export);
  ob_end_clean();

  layout_handler_add_to_page($form_state['page'], $handler, $form_state['values']['title']);

  $plugin = layout_get_task_handler($handler->handler);
  // It has no forms at all. Add the variant and go to its first operation.
  $keys = array_keys($plugin['operations']);
  $form_state['new trail'] = array('handlers', $handler->name, reset($keys));
}

/**
 * Form to enable a handler.
 */
function layout_handler_enable($form, &$form_state) {
  $form['markup'] = array(
    '#markup' => t('This variant is currently disabled. Enabling it will make it available in your system. This will not take effect until you save this page.'),
  );

  return $form;
}

/**
 * Enable the page after it has been confirmed.
 */
function layout_handler_enable_submit(&$form, &$form_state) {
  $form_state['handler']->disabled = FALSE;
  $form_state['page']->handler_info[$form_state['handler_id']]['disabled'] = FALSE;
  $form_state['page']->handler_info[$form_state['handler_id']]['changed'] |= layout_CHANGED_STATUS;
  $form_state['new trail'] = array('handlers', $form_state['handler_id'], 'actions', 'disable');
}

/**
 * Form to disable a page.
 */
function layout_handler_disable($form, &$form_state) {
  $form['markup'] = array(
    '#markup' => t('This variant is currently enabled. Disabling it will make it unavailable in your system, and it will not be used. This will not take effect until you save this page.'),
  );

  return $form;
}

/**
 * Form to disable a page.
 */
function layout_handler_summary($form, &$form_state) {
  $handler = $form_state['handler'];
  $page = $form_state['page'];
  $plugin = layout_get_task_handler($handler->handler);

  $form['markup'] = array(
    '#markup' => layout_get_handler_summary($plugin, $handler, $page, FALSE),
  );

  return $form;
}

/**
 * Disable the page after it has been confirmed.
 */
function layout_handler_disable_submit(&$form, &$form_state) {
  $form_state['handler']->disabled = TRUE;
  $form_state['page']->handler_info[$form_state['handler_id']]['disabled'] = TRUE;
  $form_state['page']->handler_info[$form_state['handler_id']]['changed'] |= layout_CHANGED_STATUS;
  $form_state['new trail'] = array('handlers', $form_state['handler_id'], 'actions', 'enable');
}

/**
 * Break the lock on a page so that it can be edited.
 */
function layout_break_lock($form, &$form_state) {
  $form['markup'] = array(
    '#markup' => t('Breaking the lock on this page will <strong>discard</strong> any pending changes made by the locking user. Are you REALLY sure you want to do this?')
  );

  return $form;
}

/**
 * Submit to break the lock on a page.
 */
function layout_break_lock_submit(&$form, &$form_state) {
  $page = &$form_state['page'];
  $form_state['page']->locked = FALSE;
  ctools_object_cache_clear_all('layout_page', $page->task_name);
  $form_state['do not cache'] = TRUE;
  drupal_set_message(t('The lock has been cleared and all changes discarded. You may now make changes to this page.'));

  $form_state['new trail'] = array('summary');
}

/**
 * Form to enable a page.
 */
function layout_enable_form($form, &$form_state) {
  $form['markup'] = array(
    '#markup' => t('Enabling this page will immediately make it available in your system (there is no need to wait for a save.)'),
  );

  return $form;
}

/**
 * Enable the page after it has been confirmed.
 */
function layout_enable_form_submit(&$form, &$form_state) {
  $page = &$form_state['page'];
  if ($function = ctools_plugin_get_function($page->subtask, 'enable callback')) {
    $result = $function($page, FALSE);
    menu_rebuild();
  }
  $form_state['new trail'] = array('actions', 'disable');

  // We don't want to cause this to cache if it wasn't already. If it was
  // cached, however, we want to update the enabled state.
  if (empty($form_state['page']->changed)) {
    $form_state['do not cache'] = TRUE;
  }
}

/**
 * Form to disable a page.
 */
function layout_disable_form($form, &$form_state) {
  $form['markup'] = array(
    '#markup' => t('Disabling this page will immediately make it unavailable in your system (there is no need to wait for a save.)'),
  );

  return $form;
}

/**
 * Disable the page after it has been confirmed.
 */
function layout_disable_form_submit(&$form, &$form_state) {
  $page = &$form_state['page'];
  if ($function = ctools_plugin_get_function($page->subtask, 'enable callback')) {
    $result = $function($page, TRUE);
    menu_rebuild();
    $form_state['new trail'] = array('actions', 'enable');

    // We don't want to cause this to cache if it wasn't already. If it was
    // cached, however, we want to update the enabled state.
    if (empty($form_state['page']->changed)) {
      $form_state['do not cache'] = TRUE;
    }
  }
}

/**
 * Print the summary information for a page.
 */
function layout_page_summary($form, &$form_state) {
  $page = $form_state['page'];

  $output = '';

  /*
    if (isset($form_state['subtask']['admin title'])) {
      $form_state['title'] = $form_state['subtask']['admin title'];
    }
  */

  if (isset($form_state['subtask']['admin description'])) {
    $output .= '<div class="description">' . $form_state['subtask']['admin description'] . '</div>';
  }

  $output .= layout_get_page_summary($page->task, $page->subtask);

  if (!empty($page->handlers)) {
    foreach ($page->handler_info as $id => $info) {
      if ($info['changed'] & layout_CHANGED_DELETED) {
        continue;
      }

      $handler = $page->handlers[$id];
      $plugin = layout_get_task_handler($handler->handler);

      $output .= '<div class="handler-summary">';
      $output .= layout_get_handler_summary($plugin, $handler, $page);
      $output .= '</div>';

    }
  }
  else {
    $output .= '<p>' . t('This page has no variants and thus no output of its own.') . '</p>';
  }

  $links = array(
    array(
      'title' => ' &raquo; ' . t('Add a new variant'),
      'href' => layout_edit_url($page->task_name, array('actions', 'add')),
      'html' => TRUE,
    ),
  );

  $output .= '<div class="links">' . theme('links', array('links' => $links)) . '</div>';
  $form['markup'] = array(
    '#markup' => $output,
  );

  return $form;
}

/**
 * Menu callback to enable or disable a page
 */
function layout_enable_page($disable, $js, $page) {
  if (!isset($_GET['token']) || !drupal_valid_token($_GET['token'], $page->task_name)) {
    return MENU_ACCESS_DENIED;
  }
  if ($page->locked) {
    if ($disable) {
      drupal_set_message(t('Unable to disable due to lock.'));
    }
    else {
      drupal_set_message(t('Unable to enable due to lock.'));
    }
  }
  else {
    if ($function = ctools_plugin_get_function($page->subtask, 'enable callback')) {
      $result = $function($page, $disable);
      menu_rebuild();

      // We want to re-cache this if it's changed so that status is properly
      // updated on the changed form.
      if (!empty($page->changed)) {
        layout_set_page_cache($page);
      }
    }
  }

  // For now $js is not actually in use on this.
  drupal_goto('admin/structure/pages');
}

/**
 * Create a menu item for page manager pages.
 *
 * @param $menu
 *   The configuration to use. It will contain a type, and depending on the
 *   type may also contain weight, title and name. These are presumed to have
 *   been configured from the UI.
 * @param $access_arguments
 *   Arguments that go with ctools_access_menu; it should be loaded with
 *   the access plugin type, settings, and positions of any arguments that
 *   may produce contexts.
 * @param $page_arguments
 *   This should be seeded with the subtask name for easy loading and like
 *   the access arguments above should contain positions of arguments so
 *   that the menu system passes contexts through.
 * @param $load_arguments
 *   Arguments to send to the arg loader; should be the subtask id and '%index'.
 */
function layout_custom_page_menu_item($menu, $access_arguments, $page_arguments, $load_arguments) {
  $item = array(
    'access callback' => 'ctools_access_menu',
    'access arguments' => $access_arguments,
    'page callback' => 'layout_page_execute',
    'page arguments' => $page_arguments,
    'load arguments' => $load_arguments,
    'file' => 'plugins/tasks/page.inc',
  );

  if (isset($menu['title'])) {
    $item['title'] = $menu['title'];
  }
  if (isset($menu['weight'])) {
    $item['weight'] = $menu['weight'];
  }

  if (empty($menu['type'])) {
    $menu['type'] = 'none';
  }

  switch ($menu['type']) {
    case 'none':
    default:
      $item['type'] = MENU_CALLBACK;
      break;

    case 'normal':
      $item['type'] = MENU_NORMAL_ITEM;
      // Insert item into the proper menu
      $item['menu_name'] = $menu['name'];
      break;

    case 'tab':
      $item['type'] = MENU_LOCAL_TASK;
      break;

    case 'action':
      $item['type'] = MENU_LOCAL_ACTION;
      break;

    case 'default tab':
      $item['type'] = MENU_DEFAULT_LOCAL_TASK;
      break;
  }

  return $item;
}

/**
 * Page callback to add a subtask.
 */
function layout_page_add_subtask($task_name = NULL, $step = NULL) {
  ctools_include('context');
  $task = layout_get_task('page');
  $task_handler_plugins = layout_get_task_handler_plugins($task);
  if (empty($task_handler_plugins)) {
    drupal_set_message(t('There are currently no variants available and a page may not be added. Perhaps you need to install the Panels module to get a variant?'), 'error');
    return ' ';
  }

  $form_info = array(
    'id' => 'layout_add_page',
    'show trail' => TRUE,
    'show back' => TRUE,
    'show return' => FALSE,
    'next callback' => 'layout_page_add_subtask_next',
    'finish callback' => 'layout_page_add_subtask_finish',
    'return callback' => 'layout_page_add_subtask_finish',
    'cancel callback' => 'layout_page_add_subtask_cancel',
    'add order' => array(
      'basic' => t('Basic settings'),
      'argument' => t('Argument settings'),
      'access' => t('Access control'),
      'menu' => t('Menu settings'),
    ),
    'forms' => array(
      'basic' => array(
        'form id' => 'layout_page_form_basic',
      ),
      'access' => array(
        'form id' => 'layout_page_form_access',
      ),
      'menu' => array(
        'form id' => 'layout_page_form_menu',
      ),
      'argument' => array(
        'form id' => 'layout_page_form_argument',
      ),
    ),
  );

  if ($task_name) {
    $page = layout_get_page_cache($task_name);
    if (empty($page)) {
      return drupal_not_found();
    }

    $form_info['path'] = "admin/structure/pages/add/$task_name/%step";
  }
  else {
    $new_page = layout_page_new();
    $new_page->name = NULL;

    $page = new stdClass();
    layout_page_new_page_cache($new_page, $page);
    $form_info['path'] = 'admin/structure/pages/add/%task_name/%step';
  }

  if ($step && $step != 'basic') {
    $handler_plugin = layout_get_task_handler($page->handler);

    $form_info['forms'] += $handler_plugin['forms'];

    if (isset($page->forms)) {
      foreach ($page->forms as $id) {
        if (isset($form_info['add order'][$id])) {
          $form_info['order'][$id] = $form_info['add order'][$id];
        }
        else if (isset($handler_plugin['add features'][$id])) {
          $form_info['order'][$id] = $handler_plugin['add features'][$id];
        }
        else if (isset($handler_plugin['required forms'][$id])) {
          $form_info['order'][$id] = $handler_plugin['required forms'][$id];
        }
      }
    }
    else {
      $form_info['order'] = $form_info['add order'];
    }

    // This means we just submitted our form from the default list
    // of steps, which we've traded in for a newly generated list of
    // steps above. We need to translate this 'next' step into what
    // our questions determined would be next.
    if ($step == 'next') {
      $keys = array_keys($form_info['order']);
      // get rid of 'basic' from the list of forms.
      array_shift($keys);
      $step = array_shift($keys);

      // If $step == 'basic' at this point, we were not presented with any
      // additional forms at all. Let's just save and go!
      if ($step == 'basic') {
        layout_save_page_cache($page);
        // Redirect to the new page's task handler editor.
        drupal_goto(layout_edit_url($page->task_name));
      }
    }
  }
  else {
    $form_info['show trail'] = FALSE;
    $form_info['order'] = array(
      'basic' => t('Basic settings'),
      'next' => t('A meaningless second page'),
    );
  }

  ctools_include('wizard');
  $form_state = array(
    'task' => $task,
    'subtask' => $page->subtask,
    'page' => &$page,
    'type' => 'add',
    'task_id' => 'page',
    'task_name' => $page->task_name,
    'creating' => TRUE,
  );

  if (!empty($page->handlers)) {
    $keys = array_keys($page->handlers);
    $key = array_shift($keys);
    $form_state['handler'] = &$page->handlers[$key];
    $form_state['handler_id'] = $key;
  }

  $output = ctools_wizard_multistep_form($form_info, $step, $form_state);

  if (!$output) {
    // redirect.
    drupal_redirect_form(array(), $form_state['redirect']);
  }

  return $output;
}

/**
 * Callback generated when the add page process is finished.
 */
function layout_page_add_subtask_finish(&$form_state) {
  $page = &$form_state['page'];
  // Update the cache with changes.
  layout_set_page_cache($page);

  $handler = $form_state['handler'];
  $handler_plugin = layout_get_task_handler($handler->handler);

  // Redirect to the new page's task handler editor.
  if (isset($handler_plugin['add finish'])) {
    $form_state['redirect'] = layout_edit_url($page->task_name, array('handlers', $handler->name, $handler_plugin['add finish']));
  }
  else {
    $form_state['redirect'] = layout_edit_url($page->task_name);
  }
  return;
}

/**
 * Callback generated when the 'next' button is clicked.
 *
 * All we do here is store the cache.
 */
function layout_page_add_subtask_next(&$form_state) {
  if (empty($form_state['task_name']) || $form_state['task_name'] == 'page') {
    // We may not have known the path to go next, because we didn't yet know the
    // task name. This fixes that.
    $form_state['form_info']['path'] = str_replace('%task_name', $form_state['page']->task_name, $form_state['form_info']['path']);

    $form_state['redirect'] = ctools_wizard_get_path($form_state['form_info'], $form_state['clicked_button']['#next']);
  }

  // Update the cache with changes.
  layout_set_page_cache($form_state['page']);
}

/**
 * Callback generated when the 'cancel' button is clicked.
 *
 * All we do here is clear the cache.
 */
function layout_page_add_subtask_cancel(&$form_state) {
  // Wipe all our stored changes.
  if (isset($form_state['page']->task_name)) {
    layout_clear_page_cache($form_state['page']->task_name);
  }
}

/**
 * Store the values from the basic settings form.
 */
function layout_page_form_basic_submit(&$form, &$form_state) {
  $page = &$form_state['page']->subtask['subtask'];
  $cache = &$form_state['page'];

  // If this is a new thing, then we have to do a bunch of setup to create
  // the cache record with the right ID and some basic data that we could
  // not know until we asked the user some questions.
  if (!isset($page->pid) && !empty($form_state['creating'])) {
    // Update the data with our new name.
    $page->name = $form_state['values']['name'];
    $form_state['page']->task_name = layout_make_task_name($form_state['task_id'], $page->name);
    $cache->handler = $form_state['values']['handler'];
    $cache->subtask_id = $page->name;
    $plugin = layout_get_task_handler($cache->handler);

    // If they created and went back, there might be old, dead handlers
    // that are not going to be added.
    //
    // Remove them:
    $cache->handlers = array();
    $cache->handler_info = array();

    // Create a new handler.
    $handler = layout_new_task_handler($plugin);
    $title = !empty($form_state['values']['title']) ? $form_state['values']['title'] : $plugin['title'];
    layout_handler_add_to_page($cache, $handler, $title);

    // Figure out which forms to present them with
    $cache->forms = array();
    $cache->forms[] = 'basic'; // This one is always there.
    if (!empty($form_state['arguments'])) {
      $cache->forms[] = 'argument';
    }

    $features = $form_state['values']['features'];
    $cache->forms = array_merge($cache->forms, array_keys(array_filter($features['default'])));
    if (isset($features[$form_state['values']['handler']])) {
      $cache->forms = array_merge($cache->forms, array_keys(array_filter($features[$form_state['values']['handler']])));
    }

    if (isset($plugin['required forms'])) {
      $cache->forms = array_merge($cache->forms, array_keys($plugin['required forms']));
    }
  }

  $page->admin_title = $form_state['values']['admin_title'];
  $cache->subtask['admin title'] = check_plain($form_state['values']['admin_title']);

  $page->admin_description = $form_state['values']['admin_description'];
  $cache->subtask['admin description'] = filter_xss_admin($form_state['values']['admin_description']);

  if ($page->path != $form_state['values']['path']) {
    $page->path = $form_state['values']['path'];
    layout_page_recalculate_arguments($page);
    $cache->path_changed = TRUE;
  }

  $page->make_frontpage = !empty($form_state['values']['frontpage']);
  $page->conf['admin_paths'] = !empty($form_state['values']['admin_paths']);
}

/**
 * Form to handle menu item controls.
 */
function layout_page_form_menu($form, &$form_state) {
  ctools_include('dependent');
  $form['menu'] = array(
    '#prefix' => '<div class="clearfix">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
  );

  $menu = $form_state['page']->subtask['subtask']->menu;
  if (empty($menu)) {
    $menu = array(
      'type' => 'none',
      'title' => '',
      'weight' => 0,
      'name' => 'navigation',
      'parent' => array(
        'type' => 'none',
        'title' => '',
        'weight' => 0,
        'name' => 'navigation',
      ),
    );
  }

  $form['menu']['type'] = array(
    '#title' => t('Type'),
    '#type' => 'radios',
    '#options' => array(
      'none' => t('No menu entry'),
      'normal' => t('Normal menu entry'),
      'tab' => t('Menu tab'),
      'default tab' => t('Default menu tab'),
      'action' => t('Local action'),
    ),
    '#default_value' => $menu['type'],
  );

  $form['menu']['title'] = array(
    '#title' => t('Title'),
    '#type' => 'textfield',
    '#default_value' => $menu['title'],
    '#description' => t('If set to normal or tab, enter the text to use for the menu item.'),
    '#dependency' => array('radio:menu[type]' => array('normal', 'tab', 'default tab', 'action')),
  );

  list($major, $minor) = explode('.', VERSION, 2);

  // Only display the menu selector if menu module is enabled.
  if (module_exists('menu')) {
    $form['menu']['name'] = array(
      '#title' => t('Menu'),
      '#type' => 'select',
      '#options' => menu_get_menus(),
      '#default_value' => $menu['name'],
      '#description' => t('Insert item into an available menu.'),
      '#dependency' => array('radio:menu[type]' => array('normal')),
    );
  }
  else {
    $form['menu']['name'] = array(
      '#type' => 'value',
      '#value' => $menu['name'],
    );
    $form['menu']['markup'] = array(
      '#value' => t('Menu selection requires the activation of menu module.'),
    );
  }
  $form['menu']['weight'] = array(
    '#title' => t('Weight'),
    '#type' => 'textfield',
    '#default_value' => isset($menu['weight']) ? $menu['weight'] : 0,
    '#description' => t('The lower the weight the higher/further left it will appear.'),
    '#dependency' => array('radio:menu[type]' => array('normal', 'tab', 'default tab', 'action')),
  );

  $form['menu']['parent']['type'] = array(
    '#prefix' => '<div id="edit-menu-parent-type-wrapper">',
    '#suffix' => '</div>',
    '#title' => t('Parent menu item'),
    '#type' => 'radios',
    '#options' => array('none' => t('No menu entry'), 'normal' => t('Normal menu item'), 'tab' => t('Menu tab')),
    '#default_value' => $menu['parent']['type'],
    '#description' => t('When providing a menu item as a default tab, Drupal needs to know what the parent menu item of that tab will be. Sometimes the parent will already exist, but other times you will need to have one created. The path of a parent item will always be the same path with the last part left off. i.e, if the path to this view is <em>foo/bar/baz</em>, the parent path would be <em>foo/bar</em>.'),
    '#dependency' => array('radio:menu[type]' => array('default tab')),
  );
  $form['menu']['parent']['title'] = array(
    '#title' => t('Parent item title'),
    '#type' => 'textfield',
    '#default_value' => $menu['parent']['title'],
    '#description' => t('If creating a parent menu item, enter the title of the item.'),
    '#dependency' => array('radio:menu[type]' => array('default tab'), 'radio:menu[parent][type]' => array('normal', 'tab')),
    '#dependency_count' => 2,
  );
  // Only display the menu selector if menu module is enabled.
  if (module_exists('menu')) {
    $form['menu']['parent']['name'] = array(
      '#title' => t('Parent item menu'),
      '#type' => 'select',
      '#options' => menu_get_menus(),
      '#default_value' => $menu['parent']['name'],
      '#description' => t('Insert item into an available menu.'),
      '#dependency' => array('radio:menu[type]' => array('default tab'), 'radio:menu[parent][type]' => array('normal')),
      '#dependency_count' => 2,
    );
  }
  else {
    $form['menu']['parent']['name'] = array(
      '#type' => 'value',
      '#value' => $menu['parent']['name'],
    );
  }
  $form['menu']['parent']['weight'] = array(
    '#title' => t('Tab weight'),
    '#type' => 'textfield',
    '#default_value' => $menu['parent']['weight'],
    '#size' => 5,
    '#description' => t('If the parent menu item is a tab, enter the weight of the tab. The lower the number, the more to the left it will be.'),
    '#dependency' => array('radio:menu[type]' => array('default tab'), 'radio:menu[parent][type]' => array('tab')),
    '#dependency_count' => 2,
  );

  return $form;
}

/**
 * Validate handler for the menu form for add/edit page task.
 */
function layout_page_form_menu_validate(&$form, &$form_state) {
  // If setting a 'normal' menu entry, make sure that any placeholders
  // support the to_arg stuff.

  if ($form_state['values']['menu']['type'] == 'normal') {
    $page = $form_state['page']->subtask['subtask'];

    foreach (explode('/', $page->path) as $bit) {
      if (!isset($bit) || $bit === '') {
        continue;
      }

      if ($bit[0] == '%') {
        // Chop off that %.
        $name = substr($bit, 1);

        // Check to see if the argument plugin allows to arg:
        if (!empty($page->arguments[$name])) {
          ctools_include('context');
          $plugin = ctools_get_argument($page->arguments[$name]['name']);
          if (!empty($plugin['path placeholder to_arg'])) {
            continue;
          }
        }

        form_error($form['menu']['type'], t('Paths with non optional placeholders cannot be used as normal menu items unless the selected argument handler provides a default argument to use for the menu item.'));
        return;
      }
    }
  }
}

/**
 * Submit handler for the menu form for add/edit page task.
 */
function layout_page_form_menu_submit(&$form, &$form_state) {
  $form_state['page']->subtask['subtask']->menu = $form_state['values']['menu'];
  $form_state['page']->path_changed = TRUE;
}

/**
 * Form to handle menu item controls.
 */
function layout_page_form_access($form, &$form_state) {
  ctools_include('context');
  $form_state['module'] = 'layout_page';
  $form_state['callback argument'] = $form_state['page']->task_name;
  $form_state['access'] = $form_state['page']->subtask['subtask']->access;
  $form_state['no buttons'] = TRUE;
  $form_state['contexts'] = array();

  // Load contexts based on argument data:
  if ($arguments = _layout_page_get_arguments($form_state['page']->subtask['subtask'])) {
    $form_state['contexts'] = ctools_context_get_placeholders_from_argument($arguments);
  }

  ctools_include('context-access-admin');
  $form = ctools_access_admin_form($form, $form_state);

  return $form;
}

/**
 * Submit handler to deal with access control changes.
 */
function layout_page_form_access_submit(&$form, &$form_state) {
  $form_state['page']->subtask['subtask']->access['logic'] = $form_state['values']['logic'];
  $form_state['page']->path_changed = TRUE;
}

/**
 * Basic settings form for a page manager page.
 */
function layout_page_argument_form_settings($form, &$form_state) {
  $page = &$form_state['page']->subtask['subtask'];
  $keyword = &$form_state['keyword'];

  if (isset($page->temporary_arguments[$keyword])) {
    $conf = $page->temporary_arguments[$keyword];
  }
  else if (isset($page->arguments[$keyword])) {
    $conf = $page->temporary_arguments[$keyword] = $page->arguments[$keyword];
  }

  if (!isset($conf)) {
    // This should be impossible and thus never seen.
    $form['error'] = array('#value' => t('Error: missing argument.'));
    return;
  }

  ctools_include('context');
  $plugin = ctools_get_argument($conf['name']);

  $form['settings'] = array(
    '#tree' => TRUE,
  );

  $form['identifier'] = array(
    '#type' => 'textfield',
    '#title' => t('Context identifier'),
    '#description' => t('This is the title of the context used to identify it later in the administrative process. This will never be shown to a user.'),
    '#default_value' => $conf['identifier'],
  );

  if (!$plugin) {
    // This should be impossible and thus never seen.
    $form['error'] = array('#value' => t('Error: missing or invalid argument plugin %argument.', array('%argument', $argument)));
    return;
  }

  if ($function = ctools_plugin_get_function($plugin, 'settings form')) {
    $function($form, $form_state, $conf['settings']);
  }

  $form_state['plugin'] = $plugin;
  return $form;
}

/**
 * Validate handler for argument settings.
 */
function layout_page_argument_form_settings_validate(&$form, &$form_state) {
  if ($function = ctools_plugin_get_function($form_state['plugin'], 'settings form validate')) {
    $function($form, $form_state);
  }
}

/**
 * Submit handler for argument settings.
 */
function layout_page_argument_form_settings_submit(&$form, &$form_state) {
  if ($function = ctools_plugin_get_function($form_state['plugin'], 'settings form submit')) {
    $function($form, $form_state);
  }

  $page = &$form_state['page']->subtask['subtask'];
  $keyword = &$form_state['keyword'];
  // Copy the form to our temporary location which will get moved again when
  // finished. Yes, finished is always next but finish can happen from other
  // locations so we funnel through that path rather than duplicate.
  $page->temporary_arguments[$keyword]['identifier'] = $form_state['values']['identifier'];
  if (isset($form_state['values']['settings'])) {
    $page->temporary_arguments[$keyword]['settings'] = $form_state['values']['settings'];
  }
  else {
    $page->temporary_arguments[$keyword]['settings'] = array();
  }
}

/**
 * Entry point to clone a page.
 */
function layout_page_form_clone($form, &$form_state) {
  $page = &$form_state['page']->subtask['subtask'];

  // This provides its own button because it does something totally different.
  unset($form['buttons']);

  $form['admin_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Administrative title'),
    '#description' => t('The name of this page. This will appear in the administrative interface to easily identify it.'),
    '#default_value' => $page->admin_title,
  );

  $form['name'] = array(
    '#type' => 'machine_name',
    '#title' => t('Page name'),
    '#machine_name' => array(
      'exists' => 'layout_page_load',
      'source' => array('admin_title'),
    ),
    '#description' => t('Enter the name to the new page It must be unique and contain only alphanumeric characters and underscores.'),
  );

  // path
  $form['path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path'),
    '#description' => t('The URL path to get to this page. You may create named placeholders for variable parts of the path by using %name for required elements and !name for optional elements. For example: "node/%node/foo", "forum/%forum" or "dashboard/!input". These named placeholders can be turned into contexts on the arguments form. You cannot use the same path as the original page.'),
    '#default_value' => $page->path,
  );

  $form['handlers'] = array(
    '#type' => 'checkbox',
    '#title' => t('Clone variants'),
    '#description' => t('If checked all variants associated with the page will be cloned as well. If not checked the page will be cloned without variants.'),
    '#default_value' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Clone'),
  );

  return $form;
}

/**
 * Validate clone page form.
 */
function layout_page_form_clone_validate(&$form, &$form_state) {
  $page = &$form_state['page']->subtask['subtask'];

  $page->old_name = $page->name;
  $page->name = NULL;
  layout_page_form_basic_validate($form, $form_state);
}

/**
 * submit clone page form.
 *
 * Load the page, change the name(s) to protect the innocent, and if
 * requested, load all the task handlers so that they get saved properly too.
 */
function layout_page_form_clone_submit(&$form, &$form_state) {
  $original = $form_state['page']->subtask['subtask'];

  $original->name = $form_state['values']['name'];
  $original->admin_title = $form_state['values']['admin_title'];
  $original->path = $form_state['values']['path'];

  $handlers = !empty($form_state['values']['handlers']) ? $form_state['page']->handlers : FALSE;
  // Export the handler, which is a fantastic way to clean database IDs out of it.
  $export = layout_page_export($original, $handlers);
  ob_start();
  eval($export);
  ob_end_clean();

  $task_name = layout_make_task_name('page', $page->name);
  $cache = new stdClass();

  layout_page_new_page_cache($page, $cache);
  layout_set_page_cache($cache);

  $form_state['redirect'] = layout_edit_url($task_name);
}

/**
 * Entry point to export a page.
 */
function layout_page_form_delete($form, &$form_state) {
  $page = &$form_state['page']->subtask['subtask'];

  if ($page->type == t('Overridden')) {
    $text = t('Reverting the page will delete the page that is in the database, reverting it to the original default page. Any changes you have made will be lost and cannot be recovered.');
  }
  else {
    $text = t('Are you sure you want to delete this page? Deleting a page cannot be undone.');
  }
  $form['markup'] = array(
    '#value' => '<p>' . $text . '</p>',
  );

  if (empty($form_state['page']->locked)) {
    unset($form['buttons']);
    $form['delete'] = array(
      '#type' => 'submit',
      '#value' => $page->type == t('Overridden') ? t('Revert') : t('Delete'),
    );
  }

  return $form;
}

/**
 * Submit handler to delete a view.
 */
function layout_page_form_delete_submit(&$form, &$form_state) {
  $page = $form_state['page']->subtask['subtask'];
  layout_page_delete($page);
  if ($page->type != t('Overridden')) {
    $form_state['redirect'] = 'admin/structure/pages';
    drupal_set_message(t('The page has been deleted.'));
  }
  else {
    $form_state['redirect'] = layout_edit_url($form_state['page']->task_name, array('summary'));
    drupal_set_message(t('The page has been reverted.'));
  }
}

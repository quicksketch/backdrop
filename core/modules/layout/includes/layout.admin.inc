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
    if (count($group) >= 1) {
      $operations = array(
        '#type' => 'dropbutton',
        '#links' => array(
          'title' => t('Rearrange'),
          'href' => 'admin/structure/layouts/layout/' . $layout->path . '/rearrange',
        ),
      );
      $row = array();
      $row[] = theme('layout_name', array('layout' => $layout));
      $row[] = theme('layout_info', array('layout' => $layout));
      $row[] = url($layout->path);
      $row[] = drupal_render($operations);
      $rows[] = $row;
    }
    foreach ($group as $layout) {
      $operations = array(
        '#type' => 'dropbutton',
        '#links' => _layout_get_operations($layout),
      );
      $row = array();
      $row[] = theme('layout_name', array('layout' => $layout));
      $row[] = theme('layout_info', array('layout' => $layout));
      $row[] = url($layout->path);
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
      'href' => 'admin/structure/layouts/layout/' . $layout->name . '/enable',
      'query' => array('token' => drupal_get_token($layout->name)),
    );
  }
  $links['edit'] = array(
    'title' => t('Edit'),
    'href' => 'admin/structure/layouts/layout/' . $layout->name,
  );
  $links['clone'] = array(
    'title' => t('Clone'),
    'href' => 'admin/structure/layouts/layout/' . $layout->name . '/clone',
  );
  if (!$layout->disabled) {
    $links['disable'] = array(
      'title' => t('Disable'),
      'href' => 'admin/structure/layouts/layout/' . $layout->name . '/disable',
      'query' => array('token' => drupal_get_token($layout->name)),
    );
  }
  if ($layout->storage == LAYOUT_STORAGE_NORMAL) {
    $links['delete'] = array(
      'title' => t('Delete'),
      'href' => 'admin/structure/layouts/layout/' . $layout->name . '/delete',
    );
  }
  elseif ($layout->storage == LAYOUT_STORAGE_OVERRIDE) {
    $links['revert'] = array(
      'title' => t('Revert'),
      'href' => 'admin/structure/layouts/layout/' . $layout->name . '/revert',
    );
  }
  return $links;
}

/**
 * Sort tasks into buckets based upon whether or not they have subtasks.
 */
function layout_get_pages($tasks, &$pages, $task_id = NULL) {
  foreach ($tasks as $id => $task) {
    if (empty($task_id) && !empty($task['page operations'])) {
      $pages['operations'] = array_merge($pages['operations'], $task['page operations']);
    }

    // If a type has subtasks, add its subtasks in its own table.
    if (!empty($task['subtasks'])) {
      layout_get_pages(layout_get_task_subtasks($task), $pages, $task['name']);
      continue;
    }

    if (isset($task_id)) {
      $task_name = layout_make_task_name($task_id, $task['name']);
    }
    else {
      $task_name = $task['name'];
    }

    $class = array('page-task-' . $id);
    if (isset($task['row class'])) {
      $class[] = $task['row class'];
    }

    if (!empty($task['disabled'])) {
      $class[] = 'page-manager-disabled';
    }

    $path = array();
    $visible_path = '';
    if (!empty($task['admin path'])) {
      foreach (explode('/', $task['admin path']) as $bit) {
        if (isset($bit[0]) && $bit[0] != '!') {
          $path[] = $bit;
        }
      }

      $path = implode('/', $path);
      if (empty($task['disabled']) && strpos($path, '%') === FALSE) {
        $visible_path = l('/' . $task['admin path'], $path);
      }
      else {
        $visible_path = '/' . $task['admin path'];
      }
    }

    $row = array('data' => array(), 'class' => $class, 'title' => strip_tags($task['admin description']));

    $type = isset($task['admin type']) ? $task['admin type'] : t('System');
    $pages['types'][$type] = $type;
    $row['data']['type'] = array('data' => $type, 'class' => array('page-manager-page-type'));

    $row['data']['name'] = array('data' => $task_name, 'class' => array('page-manager-page-name'));
    $row['data']['title'] = array('data' => $task['admin title'], 'class' => array('page-manager-page-title'));
    $row['data']['path'] = array('data' => $visible_path, 'class' => array('page-manager-page-path'));

    $storage = isset($task['storage']) ? $task['storage'] : t('In code');
    $pages['storages'][$storage] = $storage;
    $row['data']['storage'] = array('data' => $storage, 'class' => array('page-manager-page-storage'));


    $operations = array(
      array(
        'title' => t('Edit'),
        'href' => layout_edit_url($task_name),
      ),
    );

    if (!empty($task['enable callback'])) {
      if (!empty($task['disabled'])) {
        array_unshift($operations, array(
          'title' => t('Enable'),
          'href' => 'admin/structure/pages/nojs/enable/' . $task_name,
          'query' => array('token' => drupal_get_token($task_name)),
        ));
      }
      else {
        $operations[] = array(
          'title' => t('Disable'),
          'href' => 'admin/structure/pages/nojs/disable/' . $task_name,
          'query' => array('token' => drupal_get_token($task_name)),
        );
      }
    }

    $ops = theme('links__ctools_dropbutton', array('links' => $operations, 'attributes' => array('class' => array('links', 'inline'))));

    $row['data']['operations'] = array('data' => $ops, 'class' => array('page-manager-page-operations'));

    $pages['disabled'][$task_name] = !empty($task['disabled']);
    $pages['tasks'][] = $task_name;
    $pages['rows'][$task_name] = $row;
  }
}

function layout_add_page() {
  $config = array(
    'is_new' => TRUE,
  );
  $layout = new Layout($config);
  return drupal_get_form('layout_settings_form', $layout);
}

/**
 * Render the edit page for a a page, custom or system.
 */
function layout_settings_form($form, &$form_state, Layout $layout) {
  $form_state['layout'] = &$layout;
  $form['#tree'] = TRUE;

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
      'exists' => 'layout_cache_load',
    ),
    '#maxlength' => 128,
    '#default_value' => $layout->name,
  );
  $form['layout'] = array(
    '#title' => t('Layout'),
    '#type' => 'radios',
    '#default_value' => $layout->layout,
    '#options' => array(),
    '#wrapper_attributes' => array('class' => array('clearfix')),
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
    '#description' => t('New or existing paths may create a layout. Use the "%" (percent) symbol to indicate a wildcard in the path. i.e. "node/%" or "node/%/edit".'),
  );
  $form['context'] = array(
    '#title' => t('Contexts'),
    '#theme_wrappers' => array('form_element'),
  );

  $all_context_info = layout_get_context_info();
  foreach ($all_context_info as $context_name => $context_info) {
    $context_options[$context_name] = $context_info['title'];
  }
  foreach ($layout->context as $context_key => $layout_context) {
    if ($layout_context->required) {
      $form['context'][$context_key]['active'] += array(
        '#title' => t('Argument @count', array('@count' => $layout_context->position)),
        '#theme_wrappers' => array('form_element'),
        '#markup' => $layout_context->human_description(),
      );
    }
    else {
      if ($layout_context->position) {
        $title = t('Argument @count', array('@count' => $layout_context->position));
      }
      else {
        $title = t('Additional context', array('@count' => $layout_context->position));
      }
      $form['context'][$context_key]['active'] += array(
        '#type' => 'select',
        '#title' => $title,
        '#options' => $context_options,
      );
    }
  }
  $form['context']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add context'),
    '#submit' => array(
      'layout_settings_form_context_add',
    ),
    '#limit_validation' => array('context'),
  );

  $form['access'] = array(
    '#title' => t('Conditions'),
    '#theme_wrappers' => array('form_element'),
  );
  foreach ($layout->access as $access_key => $layout_access) {
    $form['access'][$access_key]['label'] = array(
      '#markup' => $layout_access->human_description(),
    );
    $form['access'][$access_key]['remove'] = array(
      '#type' => 'submit',
      '#value' => t('Remove'),
      '#submit' => array(
        'layout_settings_form_condition_remove',
      ),
      '#limit_validation' => array('conditions'),
    );
  }
  $form['access']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add condition'),
    '#submit' => array(
      'layout_settings_form_condition_add',
    ),
    '#limit_validation' => array('conditions'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => empty($layout->is_new) ? t('Save settings') : t('Create layout'),
  );

  return $form;
}

/**
 * Submit handler for layout_settings_form() that removes an access item.
 */
function layout_settings_form_condition_remove($form, &$form_state) {
  $removed_condition = $form_state['clicked_button']['#array_parents'][1];
  unset($form_state['layout']->access[$removed_condition]);
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
 * Fetch the content for an operation, after it's been discovered from arguments.
 *
 * This system runs through the CTools form wizard. Each operation specifies a form
 * or set of forms that it may use. Operations may also specify wrappers and can
 * set their own next/finish handlers so that they can make additional things happen
 * at the end.
 */
function _layout_get_operation_content($js, &$page, $active, $operation, $titles = array(), $args = array()) {
  if (isset($operation['form'])) {
    $form_info = array(
      'id' => 'layout_page',
      'finish text' => t('Update'),
      'show trail' => FALSE,
      'show back' => FALSE,
      'show return' => FALSE,
      'show cancel' => FALSE,
      'next callback' => 'layout_edit_page_next',
      'finish callback' => 'layout_edit_page_finish',
      // Items specific to the 'edit' routines that will get moved over:
      'path' => layout_edit_url($page->task_name, $active) . "/%step",
      // wrapper function to add an extra finish button.
      'wrapper' => 'layout_operation_wrapper',
    );

    // If $operation['form'] is simply a string, then it is the function
    // name of the form.
    if (!is_array($operation['form'])) {
      $form_info['order'] = array(
        'form' => $operation['title'],
      );
      $form_info['forms'] = array(
        'form' => array('form id' => $operation['form']),
      );
      if (isset($operation['wrapper'])) {
        $form_info['forms']['form']['wrapper'] = $operation['wrapper'];
      }
    }
    // Otherwise it's the order and forms arrays directly.
    else {
      $form_info['order'] = $operation['form']['order'];
      $form_info['forms'] = $operation['form']['forms'];
    }

    // Allow the operation to override any form info settings:
    if (isset($operation['form info'])) {
      foreach ($operation['form info'] as $key => $setting) {
        $form_info[$key] = $setting;
      }
    }

    if (!empty($page->subtask['operations include'])) {
      // Quickly load any files necessary to display the forms.
      $page->subtask['operations include']['function'] = 'nop';
      ctools_plugin_get_function($page->subtask, 'operations include');
    }

    $step = array_shift($args);
    // If step is unset, go with the basic step.
    if (!isset($step)) {
      $step = current(array_keys($form_info['order']));
    }

    // If it is locked, hide the buttonzzz!
    if ($page->locked && empty($operation['even locked'])) {
      $form_info['no buttons'] = TRUE;
    }

    ctools_include('wizard');
    $form_state = array(
      'page' => $page,
      'type' => 'edit',
      'ajax' => $js && (!isset($operation['ajax']) || !empty($operation['ajax'])),
      'rerender' => TRUE,
      'trail' => $active,
      'task_name' => $page->task_name,
      'task_id' => $page->task_id,
      'task' => $page->task,
      'subtask_id' => $page->subtask_id,
      'subtask' => $page->subtask,
      'operation' => $operation,
    );

    if ($active && $active[0] == 'handlers' && isset($form_state['page']->handlers[$form_state['trail'][1]])) {
      $form_state['handler_id'] = $form_state['trail'][1];
      $form_state['handler'] = &$form_state['page']->handlers[$form_state['handler_id']];
    }

    if ($active && $active[0] == 'actions' && $active[1] == 'configure' && isset($form_state['page']->new_handler)) {
      $form_state['handler_id'] = $form_state['page']->new_handler->name;
      $form_state['handler'] = &$form_state['page']->new_handler;
    }

    $built_form = ctools_wizard_multistep_form($form_info, $step, $form_state);
    $output = drupal_render($built_form);
    $title = empty($form_state['title']) ? $operation['title'] : $form_state['title'];
    $titles[] = $title;
    $title = implode(' &raquo ', array_filter($titles));

    // If there are messages for the form, render them.
    if ($messages = theme('status_messages')) {
      $output = $messages . $output;
    }

    $description = isset($operation['admin description']) ? $operation['admin description'] : (isset($operation['description']) ? $operation['description'] : '');
    $return = array(
      'title' => $title,
      'content' => $output,
      'description' => $description,
    );

    // Append any extra content, used for the preview which is submitted then
    // rendered.
    if (isset($form_state['content'])) {
      $return['content'] .= $form_state['content'];
    }

    // If the form wanted us to go somewhere else next, pass that along.
    if (isset($form_state['new trail'])) {
      $return['new trail'] = $form_state['new trail'];
    }
  }
  else {
    $return = array(
      'title' => t('Error'),
      'content' => t('This operation trail does not exist.'),
    );
  }

  $return['active'] = $active;
  return $return;
}

function layout_operation_wrapper($form, &$form_state) {
  if (empty($form_state['operation']['no update and save']) && !empty($form['buttons']['return']['#wizard type']) && $form['buttons']['return']['#wizard type']) {
    $form['buttons']['save'] = array(
      '#type' => 'submit',
      '#value' => !empty($form_state['form_info']['save text']) ? $form_state['form_info']['save text'] : t('Update and save'),
      '#wizard type' => 'finish',
      '#attributes' => $form['buttons']['return']['#attributes'],
      '#save' => TRUE,
    );
  }

  return $form;
}

/**
 * Callback generated when the an operation edit finished.
 */
function layout_edit_page_finish(&$form_state) {
  if (empty($form_state['operation']['silent'])) {
    if (empty($form_state['clicked_button']['#save'])) {
      drupal_set_message(t('The page has been updated. Changes will not be permanent until you save.'));
    }
    else {
      drupal_set_message(t('The page has been updated and saved.'));
    }
    $path = array();
    foreach ($form_state['trail'] as $operation) {
      $path[] = $operation;
      $form_state['page']->changes[implode('/', $path)] = TRUE;
    }
  }

  // If a handler was modified, set it to changed so we know to overwrite it.
  if (isset($form_state['handler_id'])) {
    $form_state['page']->handler_info[$form_state['handler_id']]['changed'] |= layout_CHANGED_CACHED;
  }

  // While we make buttons go away on locked pages, it is still possible to
  // have a lock a appear while you were editing, and have your changes
  // disappear. This at least warns the user that this has happened.
  if (!empty($page->locked)) {
    drupal_set_message(t('Unable to update changes due to lock.'));
  }

  // If the 'Update and Save' button was selected, write our cache out.
  if (!empty($form_state['clicked_button']['#save'])) {
    layout_save_page_cache($form_state['page']);
    layout_clear_page_cache($form_state['page']->task_name);
    $form_state['page'] = layout_get_page_cache($form_state['page']->task_name);
  }
  else {
    if (empty($form_state['do not cache'])) {
      layout_set_page_cache($form_state['page']);
    }
  }

  // We basically always want to force a rerender when the forms
  // are finished, so make sure there is a new trail.
  if (empty($form_state['new trail'])) {
    // force a rerender to get rid of old form items that may have changed
    // during save.
    $form_state['new trail'] = $form_state['trail'];
  }

  if (isset($form_state['new trail']) && empty($form_state['ajax'])) {
    $form_state['redirect'] = layout_edit_url($form_state['page']->task_name, $form_state['new trail']);
  }

  $form_state['complete'] = TRUE;
}

/**
 * Callback generated when the 'next' button is clicked.
 *
 * All we do here is store the cache.
 */
function layout_edit_page_next(&$form_state) {
  layout_set_page_cache($form_state['page']);
}

/**
 * Callback generated when the 'cancel' button is clicked.
 *
 * All we do here is clear the cache.
 */
function layout_edit_page_cancel(&$form_state) {
  $page = $form_state['page'];
}

/**
 * Render an operations array.
 *
 * This renders an array of operations into a series of nested UL statements,
 * with ajax automatically on unless specified otherwise. Operations will
 * automatically have the URLs generated nested.
 *
 * Each operation should have a 'type', which tells the renderer how to deal
 * with it:
 * - 'operation': An AJAX link to render. This is the default and is
 *   assumed if a type is not specified. Other fields for the operation:
 * - - 'title': The text to display. Can be an image. Must be pre-sanitized.
 * - - 'description': Text to place in the hover box over the link using the
 *     title attribute.
 * - - 'arguments': Anything optional to put at the end of the URL.
 * - - 'path': If set, overrides the default path.
 * - - 'no operations': If set, the path will not have operations appended.
 * - - 'no task': If set, the path will not have the task id.
 * - - 'no link': If set, this item will just be text, not a link.
 * - - 'ajax': If set to TRUE, ajax will be used. The default is TRUE.
 * - - 'class': An optional class to specify for the link.
 * - - 'form': The form to display for this operation, if using a single form.
 * - - 'forms': An array of forms that must be paired with 'order' of this
 *      operation uses multiple forms. See wizard tool for details.
 * - - 'order': The form order to use for multiple forms. See wizard tool for
 *     details.
 * - - 'form info': Form info overrides for the wizard. See the wizard tool
 *      for available settings
 * - 'group':
 * - - 'title': The title of the link. May be HTML.
 * - - 'title class': A class to apply to the title.
 * - - 'children': An array of more operations that this group represents.
 *     All operations within this group will have this group's ID as part
 *     of the AJAX url to make it easier to find.
 * - - 'class': A class to apply to the UL of the children.
 * - - 'collapsible': If TRUE the collapsible tool will be used.
 */
function layout_render_operations(&$page, $operations, $active_trail, $attributes, $location, $parents = array()) {
  drupal_add_library('system', 'drupal.ajax');

  if (!isset($output[$location])) {
    $output[$location] = '';
  }

  $keys = array_keys($operations);
  $first = array_shift($keys);
  $last = array_pop($keys);

  // Make sure the 'first' and 'last' operations are part of THIS nav tree:
  while ($keys && isset($operations[$first]['location']) && $operations[$first]['location'] != $location) {
    $first = array_shift($keys);
  }
  while ($keys && isset($operations[$last]['location']) && $operations[$last]['location'] != $location) {
    $last = array_pop($keys);
  }

  $active = reset($active_trail);
  foreach ($operations as $id => $operation) {
    $current_path = '';
    if ($parents) {
      $current_path .= implode('/', $parents) . '/';
    }
    $current_path .= $id;

    if (empty($operation['type'])) {
      $operation['type'] = 'operation';
    }

    // We only render an li for things in the same nav tree.
    if (empty($operation['location']) || $operation['location'] == $location) {
      if (!is_array($attributes['class'])) {
        $attributes['class'] = array($attributes['class']);
      }

      $class = empty($attributes['class']) || !is_array($attributes['class']) ? array() : $attributes['class'];

      if ($id == $first) {
        $class[] = 'operation-first';
      }
      else if ($id == $last) {
        $class[] = 'operation-last';
      }

      if (empty($operation['silent']) && !empty($page->changes[$current_path])) {
        $class[] = $operation['type'] == 'group' ? 'changed-group' : 'changed';
      }
      else {
        $class[] = 'not-changed';
      }

      if ($active == $id) {
        $class[] = $operation['type'] == 'group' ? 'active-group' : 'active';
      }
      else {
        $class[] = 'not-active';
      }

      $output[$location] .= '<li class="' . implode(' ', $class) . '">';
    }

    switch ($operation['type']) {
      case 'text':
        $output[$location] .= $operation['title'];
        break;
      case 'operation':
        $path = isset($operation['path']) ? $operation['path'] : 'admin/structure/pages/nojs/operation';
        if (!isset($operation['no task'])) {
          $path .= '/' . $page->task_name;
        }

        if (!isset($operation['no operations'])) {
          $path .= '/' . $current_path;
          if (isset($operation['arguments'])) {
            $path .= '/' . $arguments;
          }
        }

        $class = array('page-manager-operation');
        if (!isset($operation['ajax']) || !empty($operation['ajax'])) {
          $class[] = 'use-ajax';
        }
        if (!empty($operation['class'])) {
          $class[] = $operation['class'];
        }

        $description = isset($operation['description']) ? $operation['description'] : '';
        if (empty($operation['silent']) && !empty($page->changes[$current_path])) {
          $description .= ' ' . t('This setting contains unsaved changes.');
        }

        $output[$location] .= l($operation['title'], $path, array('attributes' => array('id' => 'page-manager-operation-' . $id, 'class' => $class, 'title' => $description), 'html' => TRUE));
        break;
      case 'group':
        if ($active == $id) {
          $trail = $active_trail;
          array_shift($trail);
        }
        else {
          $trail = array();
        }
        $group_location = isset($operation['location']) ? $operation['location'] : $location;
        $temp = layout_render_operations($page, $operation['children'], $trail, $operation, $group_location, array_merge($parents, array($id)));
        if ($temp) {
          foreach ($temp as $id => $text) {
            if (empty($output[$id])) {
              $output[$id] = '';
            }
            $output[$id] .= $text;
          }
        }
        break;
    }

    if (empty($operation['location']) || $operation['location'] == $location) {
      $output[$location] .= '</li>';
    }
  }

  if ($output[$location]) {
    $classes = isset($attributes['class']) && is_array($attributes['class']) ? $attributes['class'] : array();
    $classes[] = 'page-manager-operations';

    $output[$location] = '<ul class="' . implode(' ', $classes) . '">' . $output[$location] . '</ul>';

    if (!empty($attributes['title'])) {
      $class = '';
      if (isset($attributes['title class'])) {
        $class = $attributes['title class'];
      }
      $title = '<div class="page-manager-group-title' . $class . '">' . $attributes['title'] . '</div>';

      if (!empty($attributes['collapsible'])) {
        $output[$location] = theme('ctools_collapsible', array('handle' => $title, 'content' => $output[$location], 'collapsed' => empty($active_trail)));
      }
      else {
        $output[$location] = $title . $output[$location];
      }
    }
    return $output;
  }
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

/**
 * Save the page.
 */
function layout_save_page_form_submit(&$form, &$form_state) {
  layout_save_page_cache($form_state['page']);
}

/**
 * Discard changes to the page.
 */
function layout_save_page_form_cancel($form, &$form_state) {
  drupal_set_message(t('All pending changes have been discarded, and the page is now unlocked.'));
  layout_clear_page_cache($form_state['page']->task_name);

  if (!empty($form_state['page']->new)) {
    $form_state['redirect'] = 'admin/structure/pages';
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
 * Throw away a new handler and return to the add form
 */
function layout_handler_add_cancel(&$form_state) {
  $form_state['new trail'] = array('handlers', 'add');

  // Remove the temporary page.
  unset($page->new_handler);
  unset($page->forms);
}

/**
 * Provide a consistent UI for adding handlers.
 */
function layout_handler_add_form($form, $form_state, $features = array()) {
  $task = $form_state['task'];
  $task_handler_plugins = layout_get_task_handler_plugins($task);
  if (empty($task_handler_plugins)) {
    drupal_set_message(t('There are currently no variants available and a page may not be added. Perhaps you need to install the Panels module to get a variant?'), 'error');
    $form['buttons']['return']['#disabled'] = TRUE;
    return;
  }

  foreach ($task_handler_plugins as $id => $plugin) {
    $options[$id] = $plugin['title'];
    if (isset($plugin['add features'])) {
      $features[$id] = $plugin['add features'];
    }
  }

  if (!isset($form_state['type']) || $form_state['type'] != 'add') {
    $form['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title'),
      '#description' => t('Administrative title of this variant. If you leave blank it will be automatically assigned.'),
    );
  }

  $form['handler'] = array(
    '#title' => t('Variant type'),
    '#type' => 'select',
    '#options' => $options,
  );

  // This set of checkboxes is not dangerous at all.
  $form['features'] = array(
    '#type' => 'item',
    '#title' => t('Optional features'),
    '#description' => t('Check any optional features you need to be presented with forms for configuring them. If you do not check them here you will still be able to utilize these features once the new page is created. If you are not sure, leave these unchecked.'),
    '#tree' => TRUE,
  );

  ctools_include('dependent');
  foreach ($features as $plugin => $feature_list) {
    foreach ($feature_list as $feature_id => $feature) {
      $form['features'][$plugin][$feature_id] = array(
        '#type' => 'checkbox',
        '#title' => $feature,
      );
      if (!empty($form_state['page']->forms) && in_array($feature_id, $form_state['page']->forms)) {
        $form['features'][$plugin][$feature_id]['#default_value'] = TRUE;
      }

      if ($plugin != 'default') {
        $form['features'][$plugin][$feature_id] += array(
          '#dependency' => array('edit-handler' => array($plugin)),
        );
      }
    }
  }

  return $form;
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
 * Basic settings form for a page manager page.
 */
function layout_page_form_basic($form, &$form_state) {
  $page = &$form_state['page']->subtask['subtask'];
  $task = $form_state['task'];

  $form['admin_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Administrative title'),
    '#description' => t('The name of this page. This will appear in the administrative interface to easily identify it.'),
    '#default_value' => $page->admin_title,
  );

  $form['name'] = array(
    '#type' => 'machine_name',
    '#title' => t('Machine name'),
    '#machine_name' => array(
      'exists' => 'layout_page_load',
      'source' => array('admin_title'),
    ),
    '#description' => t('The machine readable name of this page. It must be unique, and it must contain only alphanumeric characters and underscores. Once created, you will not be able to change this value!'),
    '#default_value' => $page->name,
  );

  if (isset($page->pid) || empty($form_state['creating'])) {
    $form['name']['#disabled'] = TRUE;
    $form['name']['#value'] = $page->name;
  }

  $form['admin_description'] = array(
    '#type' => 'textarea',
    '#title' => t('Administrative description'),
    '#description' => t('A description of what this page is, does or is for, for administrative use.'),
    '#default_value' => $page->admin_description,
  );

  // path
  $form['path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path'),
    '#description' => t('The URL path to get to this page. You may create named placeholders for variable parts of the path by using %name for required elements and !name for optional elements. For example: "node/%node/foo", "forum/%forum" or "dashboard/!input". These named placeholders can be turned into contexts on the arguments form.'),
    '#default_value' => $page->path,
    '#field_prefix' => url(NULL, array('absolute' => TRUE)) . (variable_get('clean_url', 0) ? '' : '?q='),
  );

  $frontpage = variable_get('site_frontpage', 'node');

  $path = array();
  if ($page->path) {
    foreach (explode('/', $page->path) as $bit) {
      if ($bit[0] != '!') {
        $path[] = $bit;
      }
    }
  }

  $path = implode('/', $path);

  if (empty($path) || $path != $frontpage) {
    $form['frontpage'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($page->make_frontpage),
      '#title' => t('Make this your site home page.'),
      '#description' => t('To set this panel as your home page you must create a unique path name with no % placeholders in the path. The site home page is currently set to %homepage on the !siteinfo configuration form.', array('!siteinfo' => l(t('Site Information'), 'admin/settings/site-information'), '%homepage' => '/' . $frontpage)),
    );
    $form['admin_paths'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($page->conf['admin_paths']),
      '#title' => t('Use this page in an admin overlay.'),
      '#description' => t('Admin overlays are used in many places in Drupal 7 and administrative custom pages should probably utilize this feature.'),
    );
  }
  else if ($path == $frontpage) {
    $form['frontpage_markup'] = array(
      '#value' => '<b>' . t('This page is currently set to be your site home page. This can be modified on the !siteinfo configuration form.', array('!siteinfo' => l(t('Site Information'), 'admin/settings/site-information'))) . '</b>',
    );

    $form['frontpage'] = array(
      '#type' => 'value',
      '#value' => TRUE,
    );
  }

  if (!isset($page->pid) && !empty($form_state['creating'])) {
    $features['default'] = array(
      'access' => t('Access control'),
      'menu' => t('Visible menu item'),
    );

    module_load_include('inc', 'layout', 'layout.admin');
    $form = layout_handler_add_form($form, $form_state, $features);
  }

  return $form;
}

function layout_page_form_basic_validate_filter($value) {
  return $value === -1;
}

/**
 * Validate the basic form.
 */
function layout_page_form_basic_validate(&$form, &$form_state) {
  // Ensure path is unused by other pages.
  $page = $form_state['page']->subtask['subtask'];
  $name = !empty($form_state['values']['name']) ? $form_state['values']['name'] : $page->name;
  if (empty($name)) {
    form_error($form['name'], t('Name is required.'));
  }

  // If this is new, make sure the name is unique:
  if (empty($page->name)) {
    $test = layout_page_load($name);
    if ($test) {
      form_error($form['name'], t('That name is used by another page: @page', array('@page' => $test->admin_title)));
    }

    // Ensure name fits the rules:
    if (preg_match('/[^a-zA-Z0-9_]/', $form_state['values']['name'])) {
      form_error($form['name'], t('Page name must be alphanumeric or underscores only.'));
    }
  }

  $pages = layout_page_load_all();
  foreach ($pages as $test) {
    if ($test->name != $name && $test->path == $form_state['values']['path'] && empty($test->disabled)) {
      form_error($form['path'], t('That path is used by another page: @page', array('@page' => $test->admin_title)));
    }
  }

  // Ensure path is unused by things NOT pages. We do the double check because
  // we're checking against our page callback.
  $path = array();
  if (empty($form_state['values']['path'])) {
    form_error($form['path'], t('Path is required.'));
    // stop processing here if there is no path.
    return;
  }

  $found = FALSE;
  $error = FALSE;
  foreach (explode('/', $form_state['values']['path']) as $position => $bit) {
    if (!isset($bit) || $bit === '') {
      continue;
    }

    if ($bit == '%' || $bit == '!') {
      form_error($form['path'], t('You cannot have an unnamed placeholder (% or ! by itself). Please name your placeholder by adding a short piece of descriptive text to the % or !, such as %user or %node.'));
    }

    if ($bit[0] == '%') {
      if ($found) {
        form_error($form['path'], t('You cannot have a dynamic path element after an optional path element.'));
      }

      if ($position == 0) {
        form_error($form['path'], t('The first element in a path may not be dynamic.'));
      }

      $path[] = '%';
    }
    else if ($bit[0] == '!') {
      $found = TRUE;
    }
    else {
      if ($found) {
        form_error($form['path'], t('You cannot have a static path element after an optional path element.'));
      }
      $path[] = $bit;
    }
  }

  // Check to see if something that isn't a page manager page is using the path.
  $path = implode('/', $path);
  $result = db_query('SELECT * FROM {menu_router} WHERE path = :path', array(':path' => $path));
  foreach ($result as $router) {
    if ($router->page_callback != 'layout_page_execute') {
      form_error($form['path'], t('That path is already in use. This system cannot override existing paths.'));
    }
  }

  // Ensure the path is not already an alias to something else.
  if (strpos($path, '%') === FALSE) {
    $alias = db_query('SELECT alias, source FROM {url_alias} WHERE alias = :path', array(':path' => $path))->fetchObject();
    if ($alias) {
      form_error($form['path'], t('That path is currently assigned to be an alias for @alias. This system cannot override existing aliases.', array('@alias' => $alias->src)));
    }
  }
  else {
    if (!empty($form_state['values']['frontpage'])) {
      form_error($form['path'], t('You cannot make this page your site home page if it uses % placeholders.'));
    }
  }

  // Ensure path is properly formed.
  $args = layout_page_get_named_arguments($form_state['values']['path']);
  if ($invalid_args = array_filter($args, 'layout_page_form_basic_validate_filter')) {
    foreach ($invalid_args as $arg => $position) {
      form_error($form['path'], t('Duplicated argument %arg', array('%arg' => $arg)));
    }
  }

  if (isset($args['%'])) {
    form_error($form['path'], t('Invalid arg <em>%</em>. All arguments must be named with keywords.'));
  }

  $form_state['arguments'] = $args;
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
 * Form to handle assigning argument handlers to named arguments.
 */
function layout_page_form_argument($form, &$form_state) {
  $page = &$form_state['page']->subtask['subtask'];
  $path = $page->path;

  $arguments = layout_page_get_named_arguments($path);

  $form['table'] = array(
    '#theme' => 'layout_page_form_argument_table',
    '#page-manager-path' => $path,
    'argument' => array(),
  );

  $task_name = $form_state['page']->task_name;
  foreach ($arguments as $keyword => $position) {
    $conf = array();

    if (isset($page->temporary_arguments[$keyword]) && !empty($form_state['allow temp'])) {
      $conf = $page->temporary_arguments[$keyword];
    }
    else if (isset($page->arguments[$keyword])) {
      $conf = $page->arguments[$keyword];
    }

    $context = t('No context assigned');

    $plugin = array();
    if ($conf && isset($conf['name'])) {
      ctools_include('context');
      $plugin = ctools_get_argument($conf['name']);

      if (isset($plugin['title'])) {
        $context = $plugin['title'];
      }
    }

    $form['table']['argument'][$keyword]['#keyword'] = $keyword;
    $form['table']['argument'][$keyword]['#position'] = $position;
    $form['table']['argument'][$keyword]['#context'] = $context;

    // The URL for this ajax button
    $form['table']['argument'][$keyword]['change-url'] = array(
      '#attributes' => array('class' => array("page-manager-context-$keyword-change-url")),
      '#type' => 'hidden',
      '#value' => url("admin/structure/pages/argument/change/$task_name/$keyword", array('absolute' => TRUE)),
    );
    $form['table']['argument'][$keyword]['change'] = array(
      '#type' => 'submit',
      '#value' => t('Change'),
      '#attributes' => array('class' => array('ctools-use-modal')),
      '#id' => "page-manager-context-$keyword-change",
    );

    $form['table']['argument'][$keyword]['settings'] = array();

    // Only show the button if this has a settings form available:
    if (!empty($plugin)) {
      // The URL for this ajax button
      $form['table']['argument'][$keyword]['settings-url'] = array(
        '#attributes' => array('class' => array("page-manager-context-$keyword-settings-url")),
        '#type' => 'hidden',
        '#value' => url("admin/structure/pages/argument/settings/$task_name/$keyword", array('absolute' => TRUE)),
      );
      $form['table']['argument'][$keyword]['settings'] = array(
        '#type' => 'submit',
        '#value' => t('Settings'),
        '#attributes' => array('class' => array('ctools-use-modal')),
        '#id' => "page-manager-context-$keyword-settings",
      );
    }
  }

  return $form;
}

/**
 * Theme the table for this form.
 */
function theme_layout_page_form_argument_table($vars) {
  $form = $vars['form'];
  $header = array(
    array('data' => t('Argument'), 'class' => array('page-manager-argument')),
    array('data' => t('Position in path'), 'class' => array('page-manager-position')),
    array('data' => t('Context assigned'), 'class' => array('page-manager-context')),
    array('data' => t('Operations'), 'class' => array('page-manager-operations')),
  );

  $rows = array();

  ctools_include('modal');
  ctools_modal_add_js();
  foreach (element_children($form['argument']) as $key) {
    $row = array();
    $row[] = '%' . check_plain($form['argument'][$key]['#keyword']);
    $row[] = check_plain($form['argument'][$key]['#position']);
    $row[] = $form['argument'][$key]['#context'] . ' &nbsp; ' . drupal_render($form['argument'][$key]['change']);;
    $row[] = drupal_render($form['argument'][$key]['settings']) . drupal_render($form['argument'][$key]);

    $rows[] = array('data' => $row);
  }

  if (!$rows) {
    $rows[] = array(array('data' => t('The path %path has no arguments to configure.', array('%path' => $form['#page-manager-path'])), 'colspan' => 4));
  }

  $attributes = array(
    'id' => 'page-manager-argument-table',
  );

  $output = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => $attributes));
  return $output;
}

/**
 * Ajax entry point to edit an item
 */
function layout_page_subtask_argument_ajax($step = NULL, $task_name = NULL, $keyword = NULL) {
  ctools_include('ajax');
  ctools_include('modal');
  ctools_include('context');
  ctools_include('wizard');

  if (!$step) {
    return ctools_ajax_render_error();
  }

  if (!$cache = layout_get_page_cache($task_name)) {
    return ctools_ajax_render_error(t('Invalid object name.'));
  }

  $page = &$cache->subtask['subtask'];
  $path = $page->path;
  $arguments = layout_page_get_named_arguments($path);

  // Load stored object from cache.
  if (!isset($arguments[$keyword])) {
    return ctools_ajax_render_error(t('Invalid keyword.'));
  }

  // Set up wizard info
  $form_info = array(
    'id' => 'layout_page_argument',
    'path' => "admin/structure/pages/argument/%step/$task_name/$keyword",
    'show cancel' => TRUE,
    'next callback' => 'layout_page_argument_next',
    'finish callback' => 'layout_page_argument_finish',
    'cancel callback' => 'layout_page_argument_cancel',
    'order' => array(
      'change' => t('Change context type'),
      'settings' => t('Argument settings'),
    ),
    'forms' => array(
      'change' => array(
        'title' => t('Change argument'),
        'form id' => 'layout_page_argument_form_change',
      ),
      'settings' => array(
        'title' => t('Argument settings'),
        'form id' => 'layout_page_argument_form_settings',
      ),
    ),
  );

  $form_state = array(
    'page' => $cache,
    'keyword' => $keyword,
    'ajax' => TRUE,
    'modal' => TRUE,
    'modal return' => TRUE,
    'commands' => array(),
  );

  $output = ctools_wizard_multistep_form($form_info, $step, $form_state);
  if (!empty($form_state['cancel'])) {
    $commands = array(ctools_modal_command_dismiss());
  }
  else if (!empty($form_state['complete'])) {
    if (isset($page->temporary_arguments[$keyword])) {
      $page->arguments[$keyword] = $page->temporary_arguments[$keyword];
    }

    if (isset($page->temporary_arguments)) {
      unset($page->temporary_arguments);
    }

    // Update the cache with changes.
    layout_set_page_cache($cache);

    // Rerender the table so we can ajax it back in.
    // Go directly to the form and retrieve it using a blank form and
    // a clone of our current form state. This is an abbreviated
    // drupal_get_form that is halted prior to render and is never
    // fully processed, but is guaranteed to produce the same form we
    // started with so we don't have to do crazy stuff to rerender
    // just part of it.

    // @todo should there be a tool to do this?

    $clone_state = $form_state;
    $clone_state['allow temp'] = TRUE;
    $form = drupal_build_form('layout_page_form_argument', $form_state);

    // Render just the table portion.
    $output = drupal_render($form['table']);
    $commands = array(
      ajax_command_replace('#page-manager-argument-table', $output),
      ctools_modal_command_dismiss(),
    );
  }
  else {
    $commands = ctools_modal_form_render($form_state, $output);
  }
  print ajax_render($commands);
  ajax_footer();
  exit;
}

/**
 * Callback generated when the add page process is finished.
 */
function layout_page_argument_finish(&$form_state) {
}

/**
 * Callback generated when the 'next' button is clicked.
 *
 * All we do here is store the cache.
 */
function layout_page_argument_next(&$form_state) {
  // Update the cache with changes.
  layout_set_page_cache($form_state['page']);
}

/**
 * Callback generated when the 'cancel' button is clicked.
 *
 * We might have some temporary data lying around. We must remove it.
 */
function layout_page_argument_cancel(&$form_state) {
  $page = &$form_state['page']->subtask['subtask'];
  if (isset($page->temporary_arguments)) {
    unset($page->temporary_arguments);
    // Update the cache with changes.
    layout_set_page_cache($page);
  }
}

/**
 * Basic settings form for a page manager page.
 */
function layout_page_argument_form_change($form, &$form_state) {
  $page = &$form_state['page']->subtask['subtask'];
  $keyword = &$form_state['keyword'];

  ctools_include('context');
  $plugins = ctools_get_arguments();

  $options = array();
  foreach ($plugins as $id => $plugin) {
    if (empty($plugin['no ui'])) {
      $options[$id] = $plugin['title'];
    }
  }

  asort($options);

  $options = array('' => t('No context selected')) + $options;

  $argument = '';
  if (isset($page->arguments[$keyword]) && isset($page->arguments[$keyword]['name'])) {
    $argument = $page->arguments[$keyword]['name'];
  }

  $form['argument'] = array(
    '#type' => 'radios',
    '#options' => $options,
    '#default_value' => $argument,
  );

  return $form;
}

/**
 * Submit handler to change an argument.
 */
function layout_page_argument_form_change_submit(&$form, &$form_state) {
  $page     = &$form_state['page']->subtask['subtask'];
  $keyword  = &$form_state['keyword'];
  $argument = $form_state['values']['argument'];

  // If the argument is not changing, we do not need to do anything.
  if (isset($page->arguments[$keyword]['name']) && $page->arguments[$keyword]['name'] == $argument) {
    // Set the task to cancel since no change means do nothing:
    $form_state['clicked_button']['#wizard type'] = 'cancel';
    return;
  }

  ctools_include('context');

  // If switching to the no context, just wipe out the old data.
  if (empty($argument)) {
    $form_state['clicked_button']['#wizard type'] = 'finish';
    $page->temporary_arguments[$keyword] = array(
      'settings' => array(),
      'identifier' => t('No context'),
    );
    return;
  }

  $plugin = ctools_get_argument($argument);

  // Acquire defaults.
  $settings = array();

  if (isset($plugin['default'])) {
    if (is_array($plugin['default'])) {
      $settings = $plugin['default'];
    }
    else if (function_exists($plugin['default'])) {
      $settings = $plugin['default']();
    }
  }

  $id = ctools_context_next_id($page->arguments, $argument);
  $title = isset($plugin['title']) ? $plugin['title'] : t('No context');

  // Set the new argument in a temporary location.
  $page->temporary_arguments[$keyword] = array(
    'id' => $id,
    'identifier' => $title . ($id > 1 ? ' ' . $id : ''),
    'name' => $argument,
    'settings' => $settings,
  );
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
 * Import a task handler from cut & paste
 */
function layout_page_import_subtask($form, &$form_state, $task_name) {
  $form_state['task'] = layout_get_task($task_name);

  drupal_set_title(t('Import page'));
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Page name'),
    '#description' => t('Enter the name to use for this page if it is different from the source page. Leave blank to use the original name of the page.'),
  );

  $form['path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path'),
    '#description' => t('Enter the path to use for this page if it is different from the source page. Leave blank to use the original path of the page.'),
  );

  $form['overwrite'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow overwrite of an existing page'),
    '#description' => t('If the name you selected already exists in the database, this page will be allowed to overwrite the existing page.'),
  );

  $form['object'] = array(
    '#type' => 'textarea',
    '#title' => t('Paste page code here'),
    '#rows' => 15,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );
  return $form;
}

/**
 * Ensure we got a valid page.
 */
function layout_page_import_subtask_validate(&$form, &$form_state) {
  ob_start();
  eval($form_state['values']['object']);
  ob_end_clean();

  if (!isset($page) || !is_object($page)) {
    $errors = ob_get_contents();
    if (empty($errors)) {
      $errors = t('No handler found.');
    }
    form_error($form['object'], t('Unable to get a page from the import. Errors reported: @errors', array('@errors' => $errors)));
  }

  if (empty($form_state['values']['name'])) {
    $form_state['values']['name'] = $page->name;
  }

  $task_name = layout_make_task_name('page', $form_state['values']['name']);
  $form_state['cache'] = layout_get_page_cache($task_name);

  if ($form_state['cache'] && $form_state['cache']->locked) {
    form_error($form['name'], t('That page name is in use and locked by another user. You must <a href="!break">break the lock</a> on that page before proceeding, or choose a different name.', array('!break' => url(layout_edit_url($task_name, array('actions', 'break-lock'))))));
    return;
  }

  if (empty($form_state['values']['path'])) {
    $form_state['values']['path'] = $page->path;
  }

  if (empty($form_state['values']['overwrite'])) {
    $page->name = NULL;
  }

  $form_state['page'] = new stdClass();
  $form_state['page']->subtask['subtask'] = $page;
  layout_page_form_basic_validate($form, $form_state);
}

/**
 * Submit the import page to create the new page and redirect.
 */
function layout_page_import_subtask_submit($form, &$form_state) {
  $page = &$form_state['page']->subtask['subtask'];
  $page->name = $form_state['values']['name'];
  $page->path = $form_state['values']['path'];

  $task_name = layout_make_task_name('page', $page->name);
  $cache = layout_get_page_cache($task_name);
  if (!$cache) {
    $cache = new stdClass();
  }

  layout_page_new_page_cache($page, $cache);
  layout_set_page_cache($cache);

  $form_state['redirect'] = layout_edit_url($task_name);
}

/**
 * Entry point to export a page.
 */
function layout_page_form_export($form, &$form_state) {
  $page = $form_state['page']->subtask['subtask'];

  $export = layout_page_export($page, $form_state['page']->handlers);

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

<?php
/**
 * @file
 * Contains hook implementations Layout module provides for itself.
 */

/**
 * Implements hook_layout_info().
 */
function layout_layout_info() {
  $layouts['onecol'] = array(
    'title' => t('Single column'),
    'file path' => 'layouts/onecol',
    'icon' => 'onecol.png',
    'theme' => 'layout_onecol',
    'css' => 'onecol.css',
    'regions' => array('middle' => t('Middle column')),
  );
  $layouts['twocol'] = array(
    'title' => t('Two column'),
    'file path' => 'layouts/twocol',
    'icon' => 'twocol.png',
    'theme' => 'layout_twocol',
    'css' => 'twocol.css',
    'regions' => array(
      'left' => t('Left side'),
      'right' => t('Right side')
    ),
  );
  return $layouts;
}

/**
 * Implements hook_layout_context_info().
 */
function layout_layout_context_info() {
  $info['node'] = array(
    'title' => t('Node'),
    'description' => t('A node object.'),
    // Define the class which is used to handle this context.
    'class' => 'NodeLayoutContext',
    // Define menu paths where the node ID is a "known" context.
    'menu paths' => array(
      'node/%node',
      'node/%node/view',
    ),
    // Given the menu paths defined above, identify the parts of the path that
    // are needed to generate this context. These values will be passed in as
    // $args to the context class.
    'menu loaders' => array(
      '%node' => 'nid',
    ),
  );
  $info['node_add'] = array(
    'title' => t('Node add form'),
    'description' => t('Form for adding a new node.'),
    'class' => 'NodeAddLayoutContext',
    'menu parts' => array(
      '%type' => 'type',
    ),
    'menu paths' => array(
      'node/add/%type',
    ),
  );
  $info['node_edit'] = array(
    'title' => t('Node edit form'),
    'description' => t('Form for editing an existing node.'),
    'class' => 'NodeEditLayoutContext',
    'menu loaders' => array(
      '%node' => 'nid',
    ),
    'menu paths' => array(
      'node/%node/edit',
    ),
  );
  $info['string'] = array(
    'title' => t('String pass-through'),
    'description' => t('A pass-through context which allows a string to be passed from the URL to a block.'),
    'class' => 'LayoutStringContext',
    'menu loaders' => array(),
    'menu paths' => array(),
  );
  return $info;
}

/**
 * Implements hook_layout_access_info().
 */
function layout_layout_access_info() {
  $info = array();
  return $info;
}

/**
 * Implements hook_layout_renderer_info().
 */
function layout_layout_renderer_info() {
  $info = array();
  $info['editor'] = array(
    'class' => 'LayoutRendererEditor',
  );
  $info['simple'] = array(
    'class' => 'LayoutRendererSimple',
  );
  $info['standard'] = array(
    'class' => 'LayoutRendererStandard',
  );
  return $info;
}

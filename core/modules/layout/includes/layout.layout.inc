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
    'path' => 'layouts/onecol',
    'icon' => 'onecol.png',
    'css' => 'onecol.css',
    'theme' => 'layout_onecol',
    'regions' => array(
      'middle' => t('Middle column'),
    ),
  );
  $layouts['twocol'] = array(
    'title' => t('Two column'),
    'path' => 'layouts/twocol',
    'icon' => 'twocol.png',
    'css' => 'twocol.css',
    'theme' => 'layout_twocol',
    'regions' => array(
      'left' => t('Left side'),
      'right' => t('Right side')
    ),
  );
  $layouts['three_three_four'] = array(
    'title' => t('Three/three/four stacked'),
    'path' => 'layouts/three_three_four',
    'icon' => 'three-three-four.png',
    'css' => 'three-three-four.css',
    'theme' => 'layout_three_three_four',
    'regions' => array(
      'header' => t('Header'),
      'featured' => t('Featured'),
      'highlighted' => t('Highlighted'),
      'content' => t('Content'),
      'sidebar_first' => t('Sidebar first'),
      'sidebar_second' => t('Sidebar second'),
      'triptych_first' => t('Tryptych first'),
      'triptych_middle' => t('Tryptych second'),
      'triptych_last' => t('Tryptych last'),
      'footer_firstcolumn' => t('Footer first'),
      'footer_secondcolumn' => t('Footer second'),
      'footer_thirdcolumn' => t('Footer third'),
      'footer_fourthcolumn' => t('Footer fourth'),
      'footer' => t('Footer bottom'),
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
    // In situations where this context is being used at an "unknown" (user-
    // specified) path, provide a string that be used as the loader in
    // hook_menu().
    'path placeholder' => '%node',
    // Given an argument, the callback that will be responsible for loading the
    // main context data.
    'load callback' => 'node_load',
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
  $info['user'] = array(
    'title' => t('User'),
    'description' => t('A user object.'),
    'class' => 'UserLayoutContext',
    'menu paths' => array(
      'user/%user',
      'user/%user/view',
    ),
    'menu loaders' => array(
      '%user' => 'uid',
    ),
    'path placeholder' => '%user',
    'load callback' => 'user_load',
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
  $entities = entity_get_info();
  // Remove vocabularity related entities.
  unset($entities['taxonomy_vocabulary']);

  // Provide a bundle (type) plugin for each entity type.
  foreach ($entities as $entity_type => $entity) {
    $bundle_label = array_key_exists('bundle label', $entity) ? $entity['bundle label'] : 'Type';
    $info['entity:' . $entity_type] = array(
      'title' => t('@entity: @bundle', array('@entity' => $entity['label'], '@bundle' => $bundle_label)),
      'description' => t('Control access by @entity entity bundle.', array('@entity' => $entity_type)),
      'keyword' => $entity_type,
      'class' => 'EntityBundleLayoutAccess',
      'required contexts' => array(
        $entity_type => $entity_type,
      ),
    );
  }

  $info['front'] = array(
    'title' => t('Front page'),
    'description' => t('Is this the front page.'),
    'class' => 'FrontLayoutAccess',
  );
  $info['front'] = array(
    'title' => t('Front page'),
    'description' => t('Is this the front page.'),
    'class' => 'FrontLayoutAccess',
  );
  $info['language'] = array(
    'title' => t('Site language'),
    'description' => t('Control access by the currently active interface language.'),
    'class' => 'LanguageLayoutAccess',
  );
  $info['user_permission'] = array(
    'title' => t('User: Permission'),
    'description' => t('Control access by permission string.'),
    'class' => 'UserPermissionLayoutAccess',
    'required contexts' => array(
      'user' => 'user',
    ),
  );
  $info['user_role'] = array(
    'title' => t('User: Role'),
    'description' => t('Control access by role.'),
    'class' => 'UserRoleLayoutAccess',
    // Contexts are specified as context key => context type. The key will be
    // used in the $contexts array passed to the access class methods. The type
    // references a context provided by hook_layout_context_info().
    'required contexts' => array(
      'user' => 'user',
    ),
    // Optional if needing to clarify between contexts of the same type.
    'required contexts labels' => array(
      'user' => t('User'),
    ),
  );
  $info['path'] = array(
    'title' => t('URL path'),
    'description' => t('Control access by the current path.'),
    'class' => 'PathLayoutAccess',
  );
  return $info;
}

/**
 * Implements hook_layout_renderer_info().
 */
function layout_layout_renderer_info() {
  $info['broken'] = array(
    'class' => 'LayoutRendererBroken',
  );
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

/**
 * Implements hook_layout_style_info().
 */
function layout_layout_style_info() {
  $info['default'] = array(
    'title' => t('Default'),
    'description' => t('The default block rendering style with predictable tags provided by the site template.'),
    'region theme' => NULL,
    'block theme' => 'layout_block',
    'class' => 'LayoutStyle',
    'template' => 'templates/layout-block',
    'file' => 'includes/layout.theme.inc',
  );
  $info['dynamic'] = array(
    'title' => t('Dynamic'),
    'description' => t('Customizable rendering style in which every HTML tag can be configured.'),
    'region theme' => NULL,
    'block theme' => 'layout_block_dynamic',
    'class' => 'LayoutStyleDynamic',
    'template' => 'templates/layout-block',
    'file' => 'includes/layout.theme.inc',
  );
  return $info;
}

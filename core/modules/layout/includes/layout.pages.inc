<?php
/**
 * @file
 * Page callbacks for non-administrative pages of Layout module.
 */

/**
 * Menu callback; Generic page callback for all Layout-provided pages.
 */
function layout_page_callback() {
  $args = func_get_args();
  $menu_item_name = array_shift($args);
  $menu_item = layout_menu_item_load($menu_item_name);

  // Set contexts on the menu item contexts.
  $contexts = $menu_item->getContexts();
  foreach ($contexts as $position => $context) {
    $context->setData($args[$position]);
  }

  // Determine the correct layout to use at this path.
  $layouts = layout_get_path_layout_names($menu_item->path);
  $match_found = FALSE;
  foreach ($layouts as $layout_name) {
    $layout = layout_load($layout_name);
    // Note that because all layouts share the same menu_item, and that has
    // already been loaded and had its contexts set, the layout also has those
    // contexts available when checking access.
    if ($layout->checkAccess()) {
      $match_found = TRUE;
      break;
    }
  }

  // No layouts match based on conditions.
  if (!$match_found) {
    return MENU_NOT_FOUND;
  }

  // Use the selected layout and render it.
  $renderer = layout_create_renderer('standard', $layout);
  $renderer->ensurePageContentBlock(FALSE);
  return $renderer->render();
}

/**
 * Access callback; Check access for Layout-provided pages.
 *
 * This private function is called from layout_page_access() in layout.module.
 */
function _layout_page_access() {
  $args = func_get_args();
  $layout_name = array_shift($args);
  $layout = layout_load($layout_name);
  $contexts = $layout->getContexts();
//  foreach ($contexts as $position => $context) {
//    $context->setData($args[$position]);
//  }

  return TRUE;
}

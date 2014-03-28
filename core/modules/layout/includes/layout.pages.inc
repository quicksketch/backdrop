<?php
/**
 * @file
 * Page callbacks for non-administrative pages of Layout module.
 */

/**
 * Menu callback; Generic page callback for all Layout-provided pages.
 */
function layout_page_callback() {
  return t('This page provides a custom layout.');
}

/**
 * Access callback; Check access for Layout-provided pages.
 *
 * This private function is called from layout_page_access() in layout.module.
 */
function _layout_page_access() {
  // @todo: Port Page Manager argument checking.
  $args = func_get_args();
  return TRUE;
}

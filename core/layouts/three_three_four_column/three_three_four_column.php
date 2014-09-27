<?php
/**
 * @file
 * Preprocess functions for the 3/3/4 Column layout.
 */

/**
 * Preprocess the 3/3/4 Column layout variables.
 */

/**
 * Process variables for the three-three-four layout.
 */
function template_preprocess_layout__three_three_four_column(&$variables) {
  if ($variables['content']['sidebar_first'] && $variables['content']['sidebar_second']) {
    $variables['classes_array'][] = 'layout-two-sidebars';
  }
  elseif ($variables['content']['sidebar_first'] || $variables['content']['sidebar_second']) {
    $variables['classes_array'][] = 'layout-one-sidebar';
    if ($variables['content']['sidebar_first']) {
      $variables['classes_array'][] = 'layout-sidebar-first';
    }
    else {
      $variables['classes_array'][] = 'layout-sidebar-second';
    }
  }
  else {
    $variables['classes_array'][] = 'layout-no-sidebars';
  }
}

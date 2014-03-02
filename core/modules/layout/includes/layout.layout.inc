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
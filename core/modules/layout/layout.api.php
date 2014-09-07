<?php

/**
 * @file
 * Describe hooks provided by the Page Manager module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Provides a list of layouts that can be used within the Layout module.
 *
 * This hook returns an array keyed by a unique identifier for a layout name.
 * The contents of each item in the returned value may contain the following
 * keys:
 *   - title: The human-readable name of the layout.
 *   - path: A local path within the providing module to files needed by this
 *     layout, such as associated CSS, the icon image, and template file.
 *   - icon: An image representing the appearance of this layout.
 *   - css: A CSS file used whenever this layout is presented.
 *   - regions: A list of regions this layout provides, keyed by a machine name
 *     with a human label value.
 *   - theme: A key to be used in hook_theme() for this layout. A template name
 *     will automatically be created in which all underscores of this key are
 *     converted to hyphens. For example a theme value of
 *     "mymodule_super_layout" would assume a template of
 *     "mymodule-super-layout.tpl.php".
 */
function hook_layout_info() {
  $layouts['onecol'] = array(
    'title' => t('Single column'),
    'path' => 'layouts/onecol',
    'icon' => 'onecol.png',
    'css' => 'onecol.css',
    'theme' => 'layout_onecol',
    'regions' => array(
      'header' => t('Header'),
      'content' => t('Content'),
      'footer' => t('Footer'),
    ),
  );
  $layouts['twocol'] = array(
    'title' => t('Two column'),
    'path' => 'layouts/twocol',
    'icon' => 'twocol.png',
    'css' => 'twocol.css',
    'theme' => 'layout_twocol',
    'regions' => array(
      'header' => t('Header'),
      'content' => t('Content'),
      'sidebar' => t('Sidebar'),
      'footer' => t('Footer'),
    ),
  );
  return $layouts;
}

/**
 * Provides information on rendering styles that can be used by layouts.
 *
 * This hook provides a list of styles that can be used both both regions and
 * individual blocks. A style can modify the output of a layout if needed for
 * specialized presentation.
 *
 * @return array
 *   An info array keyed by a unique machine name for that style. Possible keys
 *   include:
 *   - title: The translated title of the style.
 *   - description: The translated description of the style.
 *   - region theme: Optional. If this style needs to modify the display of an
 *     entire region, specify the theme function/template key that would be
 *     passed into the theme() function.
 *   - block theme: Optional. If this style modifies the display of blocks,
 *     specify the theme function/template key that would be passed into the
 *     theme function.
 *   - class: Optional. The name of a class which contains any advanced methods
 *     to configure and save settings for this display style. If not specified,
 *     the default class of LayoutStyle will be used.
 *   - file: Optional. If using theme or preprocess functions, specify the path
 *     to the containing file. This file path is NOT the path to the class.
 *     Class paths and loading is done through hook_autoload_info().
 *   - path: The relative path from this module to the directory containing any
 *     templates or include files needed by this style.
 *   - hook theme: Optional. If specified, additional information to be merged
 *     into hook_theme() on behalf of this style. This may be necessary if the
 *     values provided for "region theme" and "block theme" have not already
 *     been registered.
 *
 * @see LayoutStyle
 * @see hook_autoload_info()
 */
function hook_layout_style_info() {
  $info['custom_style'] = array(
    'title' => t('A new style'),
    'description' => t('An advanced style with settings.'),
    // The theme key indicating what theme function/template will be used.
    'region theme' => 'mymodule_layout_region',
    // The theme key for rendering an individual block.
    'block theme' => 'mymodule_layout_block',
    // Provide a class name if this style has settings. The class should extend
    // the LayoutStyle class.
    'class' => 'MyModuleLayoutStyle',
    // If nesting templates or a theme include file in a directory, the relative
    // path can be specified.
    'path' => 'templates',
  );
  return $info;
}

/**
 * @} End of "addtogroup hooks".
 */

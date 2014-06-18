<?php

/**
 * @file
 * Class for loading, modifying, and executing a layout.
 */
class Layout {
  /**
   * The name of the layout.
   *
   * @var string
   */
  var $name = '';

  /**
   * The human readable name of the layout.
   *
   * @var string
   */
  var $title = '';

  /**
   * The description of the view, which is used only in the interface.
   *
   * @var string
   */
  var $description;

  /**
   * The module that originally provided this layout (if any).
   *
   * @var string
   */
  var $module;

  /**
   * The internal path of a layout.
   *
   * The layout path must match the provided menu item path.
   *
   * @var string
   */
  var $path;

  /**
   * The weight of this layout compared with other layouts at the same path.
   *
   * @var int
   */
  var $weight = 0;

  /**
   * The storage state of the layout.
   *
   * This is the machine-version of the $type variable, which represents whether
   * this view is has a default, user-created, or overridden configuration.
   * Possible values for this variable include the constants
   * LAYOUT_STORAGE_NORMAL, LAYOUT_STORAGE_OVERRIDE, or LAYOUT_STORAGE_DEFAULT.
   *
   * @var int
   */
  var $storage;

  /**
   * The name of the layout template used by this configuration.
   *
   * @var string
   */
  var $layout;

  /**
   * Whether this layout is disabled.
   *
   * @var boolean
   */
  var $disabled = FALSE;

  /**
   * An array of all contexts used by this layout.
   *
   * @var array
   */
  var $context = array();

  /**
   * An array of all access rules used by this layout.
   *
   * @var array
   */
  var $access = array();

  /**
   * An array of all relationships used by this layout.
   *
   * @var array
   */
  var $relationship = array();

  /**
   * A LayoutMenuItem containing menu properties for this layout.
   *
   * Layouts that are at user-defined paths (not overriding a module) must have
   * a menu item specified. Multiple layouts may share the same menu item.
   *
   * @var LayoutMenuItem
   */
  var $menu_item = NULL;

  /**
   * Constructor for a Layout class.
   *
   * @param array $config
   *   An array of configuration data.
   */
  function __construct(array $config = array()) {
    foreach ($config as $property => $property_value) {
      $this->{$property} = $property_value;
    }

    // Set the storage type.
    if (isset($config['module'])) {
      if (empty($config['storage']) || $config['storage'] == LAYOUT_STORAGE_DEFAULT) {
        $this->storage = LAYOUT_STORAGE_DEFAULT;
      }
      else {
        $this->storage = LAYOUT_STORAGE_OVERRIDE;
      }
    }
    else {
      $this->storage = LAYOUT_STORAGE_NORMAL;
    }

    // Convert all stored contexts, access, and relationships to their
    // handler equivalents.
    foreach (array('context', 'access', 'relationship') as $plugin_type) {
      foreach ($this->{$plugin_type} as $plugin_type_key => $plugin_data) {
        $this->{$plugin_type}[$plugin_type_key] = layout_create_handler($plugin_type, $plugin_data['plugin'], $plugin_data['data']);
      }
    }
  }

  /**
   * Save a layout to config.
   */
  function save() {
    // Convert all properties to an array.
    $data = (array) $this;

    // Remove unnecessary properties.
    $remove_properties = array(
      'is_new',
      'changed',
      'menu_item',
    );
    foreach ($remove_properties as $property) {
      if (array_key_exists($property, $data)) {
        unset($data[$property]);
      }
    }

    // Convert all stored contexts, access, and relationships to arrays.
    foreach (array('context', 'access', 'relationship') as $config_type) {
      foreach ($data[$config_type] as $config_type_key => $config_type_data) {
        unset($config_type_data->is_new);
        $data[$config_type][$config_type_key] = array(
          'plugin' => $config_type_data->plugin,
          'data' => $config_type_data->toArray(),
        );
      }
    }

    if (isset($this->original_name) && $this->original_name != $this->name) {
      config('layout.layout.' . $this->original_name)->delete();
    }
    config('layout.layout.' . $this->name)
      ->setData($data)
      ->save();

    // If this layout overrides an existing module path, reassign or delete
    // the layout menu item.
    if (menu_get_item($this->path) && !layout_provides_path($this->path)) {
      if ($this->menu_item) {
        $this->menu_item->reassign();
      }
    }
    // If not overriding an existing module path, ensure that the layout always
    // has a layout menu item at the same path.
    else {
      if ($this->menu_item) {
        // If an existing menu item exists but this layout's path has changed,
        // reassign/delete the existing menu item, then create a new one.
        if ($this->menu_item->path !== $this->path) {
          $new_menu_item = clone($this->menu_item);
          $new_menu_item->path = $this->path;
          $new_menu_item->name = $this->name;
          $new_menu_item->is_new = TRUE;
          $this->menu_item->reassign();
          $this->menu_item = $new_menu_item;
          $this->menu_item->save();
        }
      }
      else {
        // If this layout doesn't have a menu item, make one.
        $menu_item_settings = array(
          'path' => $this->path,
          'name' => $this->name,
        );
        $this->menu_item = new LayoutMenuItem($menu_item_settings);
        $this->menu_item->save();
      }
    }
    layout_reset_caches();
  }

  /**
   * Delete this layout.
   */
  function delete() {
    if ($this->storage === LAYOUT_STORAGE_NORMAL) {
      config('layout.layout.' . $this->name)->delete();
      // Reassign the menu item to a still-enabled layout (if any).
      if ($this->menu_item && $this->menu_item->name === $this->name) {
        $this->menu_item->reassign();
      }
      layout_reset_caches();
    }
    else {
      $this->disable();
    }
  }

  /**
   * Revert a layout to a module-provided default.
   */
  function revert() {
    if (!empty($this->module)) {
      // Restore the module-provided layout config.
      config_install_default_config($this->module, 'layout.layout.' . $this->name);

      // Restore the module-provided menu settings if this layout provides the
      // active menu item.
      if ($this->menu_item && $this->menu_item->name === $this->name) {
        $this->menu_item->revert();
      }
    }
  }

  /**
   * Disable a layout.
   */
  function disable() {
    $this->disabled = TRUE;
    $this->save();

    // Reassign the menu item to a still-enabled layout (if any).
    if ($this->menu_item && $this->menu_item->name === $this->name) {
      $this->menu_item->reassign();
    }
  }

  /**
   * Enable a layout.
   */
  function enable() {
    $this->disabled = FALSE;
    $this->save();

    // Check if the menu item needs to be assigned to this layout.
    if ($this->menu_item->name === $this->name) {
      $this->menu_item->reassign();
    }
  }
}


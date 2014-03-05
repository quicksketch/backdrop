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
   * The module that originally provided this view (if any).
   *
   * @var string
   */
  var $module;

  /**
   * The internal path of a layout.
   *
   * @var string
   */
  var $path;

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
    foreach (array('context', 'access', 'relationship') as $config_type) {
      foreach ($this->{$config_type} as $config_type_key => $config_type_data) {
        $this->{$config_type}[$config_type_key] = layout_create_handler($config_type_data['handler'], $config_type_data['data']);
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
    if (isset($data['is_new'])) {
      unset($data['is_new']);
    }
    if (isset($data['changed'])) {
      unset($data['changed']);
    }

    // Convert all stored contexts, access, and relationships to arrays.
    foreach (array('context', 'access', 'relationship') as $config_type) {
      foreach ($data[$config_type] as $config_type_key => $config_type_data) {
        unset($config_type_data->is_new);
        $data[$config_type][$config_type_key] = array(
          'handler' => get_class($config_type_data),
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
  }

  /**
   * Delete this layout.
   */
  function delete() {
    config('layout.layout.' . $this->name)->delete();
    menu_rebuild();
  }

  /**
   * Check if this layout overrides a path provided by another module.
   */
  function isOverridingPath() {
    $result = db_query('SELECT * FROM {menu_router} WHERE path = :path', array(':path' => $this->path));
    foreach ($result as $router) {
      if ($router->page_callback != 'layout_page') {
        return TRUE;
      }
    }
    return FALSE;
  }
}
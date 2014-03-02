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

}
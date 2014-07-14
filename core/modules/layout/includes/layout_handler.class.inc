<?php
/**
 * @file
 * Base classes used by all Layout handlers.
 *
 * All layout handlers have some things in common, including the name of the
 * plugin itself, and any settings affiliated with that handler.
 */
abstract class LayoutHandler {
  /**
   * Indicator if this object has not yet been saved.
   *
   * @var Boolean
   */
  var $is_new = FALSE;

  /**
   * The name of this plugin.
   *
   * Plugin names are text strings that map to particular classes. This mapping
   * is provided by the appropriate hook_layout_*_info() hook, such as
   * hook_layout_access_info().
   */
  var $plugin;

  /**
   * An array of all settings pertaining to this handler.
   *
   * @var array
   */
  var $settings = array();

  function __construct($plugin_name, array $data = array()) {
    // Provide a settings array default for all plugins.
    $data += array(
      'settings' => array(),
    );

    $this->plugin = $plugin_name;
    $this->settings = $data['settings'];
  }

  /**
   * Assemble a human-readable label of this object.
   */
  function label() {
    $label = get_class($this);
    if ($this->settings['title']) {
      $label .= '(' . check_plain($this->settings['title']) . ')';
    }
    return $label;
  }

  /**
   * Convert this object's properties to an array.
   *
   * This is used to convert this handler's configuration to a storage-safe
   * array, such as to be stored by the config management system as JSON.
   */
  function toArray() {
    return array(
      'settings' => $this->settings,
    );
  }

  /**
   * Provides a child plugin's info based on the plugin name.
   *
   * For example the "Entity field value" class provides not a single plugin but
   * one for every field configured on the site. In this situation the "parent"
   * name would be the name of the providing plugin, e.g. "entity_field_value",
   * and the child name would be the entity type, bundle, and field name, e.g.
   * "node:article:field_image".
   *
   * @param array $plugin_info
   *   This plugin's info array as provided by hook_layout_access_info().
   * @param string $parent_name
   *   The name of the parent plugin.
   * @param string $child_name
   *   The identifier for the child plugin.
   * @return array
   *   An array of plugin info, as would be provided by
   *   hook_layout_access_info(), or NULL if the child is not found.
   */
  static function getChild(array $plugin_info, $parent_name, $child_name) {
    return NULL;
  }

  /**
   * Provides an array of all child plugins provided by this class.
   *
   * This can be used to make a single class provide multiple plugins. This is
   * particularly useful when an abstracted system can handle a lot of different
   * things in the same manner, e.g. the Field system provides a single class
   * that creates plugins for all fields in the system.
   *
   * @param array $plugin_info
   *   This plugin's info array as provided by hook_layout_access_info().
   * @param string $parent_name
   *   The name of the parent plugin.
   * @return array
   *   An array of plugin info, as would be provided by
   *   hook_layout_access_info().
   */
  static function getChilden(array $plugin_info, $parent) {
    return NULL;
  }
}

/**
 * Handler to hold configuration of handlers that seemingly have been removed.
 */
class LayoutHandlerBroken extends LayoutHandler {}
<?php
/**
 * @file
 * Class for loading, modifying, and saving a layout access rule (condition).
 */
class LayoutAccess extends LayoutHandler {
  /**
   * An array of all settings pertaining to this access rule.
   *
   * @var array
   */
  var $settings = array();

  /**
   * Constructor for a Layout class.
   *
   * @param array $settings
   *   An array of configuration data.
   */
  function __construct($plugin_name, array $settings = array()) {
    $this->plugin = $plugin_name;
    $this->settings = $settings;
  }

  /**
   * Provides a human-readable summary of this access check's behavior.
   *
   * @return string
   *   The summary string for this access check.
   */
  function summary() {
    return '';
  }

  /**
   * Checks for access based on the available context.
   *
   * @param $context
   *   An object containing the context.
   * @return boolean
   *   The result of the access check.
   */
  function checkAccess($context) {

  }

  /**
   * Settings form for configuring this access item.
   */
  function form(&$form, &$form_state) {
  }


  /**
   * Validate handler for the settings form for this access item.
   */
  function formValidate($form, &$form_state) {

  }

  /**
   * Submit handler for the settings form for this access item.
   */
  function formSubmit($form, &$form_state) {

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
 * A common base class used for access handlers that may be negated.
 */
class LayoutAccessNegatable extends LayoutAccess {
  function __construct($plugin_name, array $settings = array()) {
    $settings += array(
      'negate' => FALSE,
    );
    parent::__construct($plugin_name, $settings);
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    $form['not'] = array(
      '#type' => 'checkbox',
      '#title' => t('Reverse (NOT)'),
      '#weight' => 100,
    );
  }
}

/**
 * A class to be used for contexts whose handler cannot be found.
 */
class LayoutAccessBroken extends LayoutAccess {
  function summary() {
    return t('Broken or missing access plugin');
  }
  function checkAccess($context) {
    return TRUE;
  }
}

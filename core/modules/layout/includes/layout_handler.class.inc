<?php
/**
 * @file
 * Base classes used by all Layout handlers.
 */
abstract class LayoutHandler {
  /**
   * The human-readable title of this object.
   *
   * This is usually optional, in which case a generated title will be used.
   *
   * @var string
   *
   * @see LayoutHandler::label()
   */
  var $title = '';

  /**
   * Indicator if this object has not yet been saved.
   *
   * @var Boolean
   */
  var $is_new = FALSE;

  /**
   * The name of this plugin.
   */
  var $plugin;

  /**
   * Assemble a human-readable label of this object.
   */
  function label() {
    $label = get_class($this);
    if ($this->title) {
      $label .= '(' . check_plain($this->title) . ')';
    }
    return $label;
  }

  /**
   * Convert this object's properties to an array.
   */
  function toArray() {
    $array = (array) $this;
    unset($array['plugin']);
    return $array;
  }
}

/**
 * Handler to hold configuration of handlers that seemingly have been removed.
 */
class LayoutHandlerBroken extends LayoutHandler {
  function __construct($plugin_name, array $config = array()) {
    $this->plugin = $plugin_name;
    foreach ($config as $key => $value) {
      $this->$key = $value;
    }
  }
}
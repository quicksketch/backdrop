<?php

/**
 * @file
 * Class that holds information relating to a layouts context.
 *
 * This class does not extend the LayoutHandler class because contexts are not
 * stored directly within configuration. Instead contexts are determined based
 * on the Layout's path
 */
abstract class LayoutContext {
  /**
   * The name of the plugin that provides this context.
   *
   * @var string
   */
  var $plugin;

  /**
   * Indicator as to whether this context is required and cannot be removed.
   *
   * @var bool
   */
  var $required = FALSE;

  /**
   * Indicator as to whether this context may not be changed to a different one.
   *
   * @var bool
   */
  var $locked = FALSE;

  /**
   * The order of this context in its containing layout.
   *
   * If this context is bound to an argument in a path, this will match that
   * position in the path.
   */
  var $position;

  /**
   * The data within this context.
   *
   * @var mixed
   */
  var $data;

  /**
   * The argument value from the URL or passed in directly.
   *
   * @var string
   */
  var $argument;

  /**
   * A list of values that must match in order for this context to be valid.
   *
   * This list is compared against the $data object to ensure that this context
   * applies to the current situation. The key of each entry maps to a property
   * on the $data object. And the values of each entry must match the $data
   * property value in order to be considered valid.
   *
   * @var array
   */
  var $restrictions = array();

  /**
   * Return the indicator for this context, i.e. "entity", "string".
   */
  abstract function type();

  /**
   * Assemble a human-readable label of this object.
   */
  function label() {
    $info = layout_get_context_info($this->plugin);
    return $info['title'];
  }

  /**
   * Set the data for this context.
   */
  function setData($data) {
    $this->data = $data;
  }
}

/**
 * A class to be used for contexts whose handler cannot be found.
 */
class LayoutContextBroken extends LayoutContext {
  function type() {
    return 'broken';
  }
  function label() {
    return t('Broken or missing context (@plugin)', array('@plugin' => $this->plugin));
  }
}

/**
 * Pass-through context used when an unknown argument is encountered.
 */
class LayoutOverridesPathContext extends LayoutContext {
  function type() {
    return 'overrides_path';
  }
  function label() {
    return t('Layout overrides path');
  }
}

/**
 * Pass-through context used when an unknown argument is encountered.
 */
class LayoutStringContext extends LayoutContext {
  function type() {
    return 'string';
  }
  function label() {
    return t('String pass-through');
  }
}

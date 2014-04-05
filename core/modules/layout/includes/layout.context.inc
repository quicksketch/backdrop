<?php

/**
 * @file
 * Class that holds information relating to a layouts context.
 *
 * The contents of this object are usually stored in a Layout object, within the
 * $layout->context array.
 */
abstract class LayoutContext extends LayoutHandler {
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
   * If this context is bound to an argument in a path, this  will match that
   * position in the path.
   */
  var $position;

  /**
   * The data within this context.
   *
   * @var mixed
   */
  protected $data;

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
   * Return this object as an array for storage to config.
   *
   * @return array
   */
  function toArray() {
    // Remove protected/private variables before serializing.
    $data = $this->data;
    unset($this->data);
    $array = parent::toArray();
    $this->data = $data;
    return $array;
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
    return t('Broken or missing context');
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

/**
 * Given a path with wildcard placeholders (%), determine the required contexts.
 *
 * @param string $path
 *   The path for which required contexts should be retrieved.
 * @return array
 *   An array of required contexts that match the given path.
 */
function layout_context_required_by_path($path) {
  $required_context_info = array();

  // Populate each required context on the available known handlers.
  $all_info = layout_get_context_info();
  foreach ($all_info as $context_info) {
    foreach ($context_info['menu paths'] as $context_path) {
      // Remove named placeholders to just match on % symbols.
      $cleaned_path = preg_replace('/%[a-z0-9_]+/', '%', $context_path);
      // If the path matches based on % wildcards, add this context as a
      // required context info in the correct order.
      if (strpos($path, $cleaned_path) === 0) {
        // Get the first named argument.
        $loader_part = key($context_info['menu loaders']);
        // Determine the argument placement within the path
        $parts = explode('/', $context_path);
        foreach ($parts as $part_index => $part) {
          if ($part === $loader_part) {
            $required_context_info[$part_index] = $context_info;
          }
        }
      }
    }
  }

  // If any wildcards are not filled, populate with a placeholder context
  // that can be modified.
  $parts = explode('/', $path);
  foreach ($parts as $part_index => $part) {
    if ($part === '%' && !isset($required_context_info[$part_index])) {
      $required_context_info[$part_index] = $all_info['string'];
    }
  }

  // Put the required contexts in order based on the path.
  ksort($required_context_info);

  // Convert required context info into objects.
  $required_contexts = array();
  foreach ($required_context_info as $context_position => $context_info) {
    $context = layout_create_handler($context_info['context class']);
    $context->required = TRUE;
    // String placeholders are not locked, allowing a new value to be set.
    $context->locked = $context_info['name'] !== 'string';
    $context->position = $context_position;
    $required_contexts[$context_position] = $context;
  }

  return $required_contexts;
}

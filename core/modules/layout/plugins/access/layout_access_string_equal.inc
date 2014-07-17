<?php
/**
 * @file
 * Plugin to provide access control/visibility based on string matching.
 */
class StringEqualLayoutAccess extends LayoutAccess {
  /**
   * Constructor for a Layout access rule.
   */
  function __construct($plugin_name, array $data = array()) {
    parent::__construct($plugin_name, $data);
    $this->settings += array(
      'visibility_setting' => 1,
      'paths' => '',
    );
  }

  /**
   * {@inheritdoc}
   */
  function summary() {
    $paths = array();
    foreach (explode("\n", $this->settings['paths']) as $path) {
      $paths[] = check_plain($path);
    }
    if ($this->settings['visibility_setting']) {
      return format_plural(count($paths), 'Current path is "@paths"', 'Current path type is one of "@paths"', array('@paths' => implode(', ', $paths)));
    }
    else {
      return format_plural(count($paths), 'Current path is not "@paths"', 'Current path type is not one of "@paths"', array('@paths' => implode(', ', $paths)));
    }
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    if (empty($context) || empty($context->data)) {
      $string = '';
    }
    else {
      $string = $context->data;
    }

    $value = $this->settings['value'];
    if (empty($this->settings['case'])) {
      $string = backdrop_strtolower($string);
      $value = backdrop_strtolower($value);
    }

    switch ($this->settings['operator']) {
      case '=':
        return $string === $value;
      case '!=':
        return $string !== $value;
      case 'regex':
        return preg_match($value, $string);
      case '!regex':
        return !preg_match($value, $string);
    }
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    $form['operator'] = array(
      '#type' => 'radios',
      '#title' => t('Operator'),
      '#options' => array(
        '=' => t('Equal'),
        '!=' => t('Not equal'),
        'regex' => t('Regular expression'),
        '!regex' => t('Not equal to regular expression'),
      ),
      '#default_value' => $this->settings['operator'],
      '#description' => t('If using a regular expression, you should enclose the pattern in slashes like so: <em>/foo/</em>. If you need to compare against slashes you can use another character to enclose the pattern, such as @. See <a href="http://www.php.net/manual/en/reference.pcre.pattern.syntax.php">PHP regex documentation</a> for more.'),
    );

    $form['value'] = array(
      '#type' => 'textfield',
      '#title' => t('String'),
      '#default_value' => $this->settings['value'],
    );

    $form['case'] = array(
      '#type' => 'checkbox',
      '#title' => t('Case sensitive'),
      '#default_value' => $this->settings['case'],
    );
  }
}

/**
 * Provide a summary description based upon the specified context
 */
function ctools_string_equal_ctools_access_summary($conf, $context) {
  $values = array('@identifier' => $context->identifier, '@value' => $conf['value']);
  switch ($conf['operator']) {
    case '=':
      return t('@identifier is "@value"', $values);
    case '!=':
      return t('@identifier is not "@value"', $values);
    case 'regex':
      return t('@identifier matches "@value"', $values);
    case '!regex':
      return t('@identifier does not match "@value"', $values);
  }
}


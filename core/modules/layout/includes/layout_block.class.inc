<?php
/**
 * @file
 * A class that wraps around a block to store settings information.
 */
class LayoutBlock extends LayoutHandler {

  /**
   * The module that provides this block.
   *
   * @var string
   */
  var $module;

  /**
   * The key for this block within the module.
   *
   * @var string
   */
  var $delta;

  /**
   * The identifier for this instance of this block.
   *
   * @var string
   */
  var $uuid;

  /**
   * The current style for this block.
   *
   * @var LayoutStyle
   */
  var $style;

  /**
   * An array of all access rules (of type LayoutAccess), keyed by ID.
   *
   * @var array
   */
  var $conditions = array();

  /**
   * Create a new LayoutBlock object.
   *
   * @param string $module
   *   The module that provides this block.
   * @param string $delta
   *   The key for this block within the module.
   * @param array $settings
   *   Settings that provide current configuration of the block, such as
   *   access rules, style settings, block settings, etc.
   */
  function __construct($plugin_name, array $settings = array()) {
    $this->plugin = $plugin_name;

    // Provide defaults.
    $settings += array(
      'settings' => array(),
      'style' => array(
        'plugin' => 'default',
        'settings' => array(),
      ),
      'conditions' => array(),
    );

    // Initialize basics, module, delta, and UUID.
    list($this->module, $this->delta) = explode(':', $plugin_name);
    $this->uuid = isset($settings['uuid']) ? $settings['uuid'] : NULL;
    $this->is_new = !$this->uuid;

    // Initialize the style.
    $this->style = layout_create_handler('layout_style', $settings['style']['plugin'], $settings['style']['settings']);

    // Initialize conditions.
    foreach ($settings['conditions'] as $condition) {
      $this->conditions[] = layout_create_handler('layout_condition', $condition['plugin'], $condition['settings']);
    }

    // All other settings.
    $settings['settings'] += array(
      'title_display' => LAYOUT_TITLE_DEFAULT,
      'title' => '',
      'block_settings' => array(),
    );
    $this->settings = $settings['settings'];
  }

  /**
   * Return the title of a block as provided by hook_block_view().
   *
   * @return mixed
   */
  function getTitle() {
    if ($this->settings['title_display'] === LAYOUT_TITLE_CUSTOM && $this->settings['title']) {
      return $this->settings['title'];
    }
  }

  /**
   * Return an administrative title that will always have a value.
   */
  function getAdminTitle() {
    $title = $this->getTitle();
    if (empty($title)) {
      // Last effort, use the title from hook_block_info().
      $info = module_invoke($this->module, 'block_info');
      $title = $info[$this->delta]['info'];
    }
    return $title;
  }

  /**
   * Return the content of a block as provided by hook_block_view().
   *
   * @return mixed
   */
  function getContent() {
    $block_content = $this->buildBlock();
    return $block_content['content'];
  }

  /**
   * Return a preview for this block.
   */
  function getAdminPreview() {
    $info = module_invoke($this->module, 'block_info');
    if (isset($info[$this->delta]['description'])) {
      return $info[$this->delta]['description'];
    }
  }

  /**
   * Build the settings form for editing this block.
   */
  function form(&$form, &$form_state) {
    $form['title_display']['title_display'] = array(
      '#type' => 'select',
      '#title' => t('Title type'),
      '#options' => array(
        LAYOUT_TITLE_DEFAULT => t('Default title'),
        LAYOUT_TITLE_CUSTOM => t('Custom title'),
        LAYOUT_TITLE_NONE => t('No title'),
      ),
      '#default_value' => $this->settings['title_display'],
    );
    $form['title_display']['title'] = array(
      '#type' => 'textfield',
      '#default_value' => $this->settings['title'],
      '#title' => t('Title'),
      '#description' => t('The title of this layout. If left blank, a default title may be used.'),
      '#states' => array(
        'visible' => array(
          'form.layout-block-configure-form :input[name="title_display[title_display]"]' => array('value' => LAYOUT_TITLE_CUSTOM),
        ),
      ),
      '#maxlength' => 255,
    );
  }

  /**
   * Validate handler to validate the form settings.
   */
  function formValidate($form, &$form_state) {
    // No additional validation needed yet.
  }

  /**
   * Submit handler to save the form settings.
   */
  function formSubmit($form, &$form_state) {
    $this->settings['title_display'] = $form_state['values']['title_display']['title_display'];
    $this->settings['title'] = $form_state['values']['title_display']['title'];
    $this->settings['style'] = $form_state['values']['style'];
  }

  /**
   * Convert the configuration of this block to an array for storage.
   */
  function toArray() {
    $array = array(
      'module' => $this->module,
      'delta' => $this->delta,
      'settings' => $this->settings,
      'uuid' => $this->uuid,
      'style' => $this->style->toArray(),
    );
    foreach ($this->conditions as $condition) {
      $array['conditions'][] = $condition->toArray();
    }
    return $array;
  }
}

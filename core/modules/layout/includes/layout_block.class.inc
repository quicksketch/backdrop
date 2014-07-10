<?php
/**
 * @file
 * A class that wraps around a block to store settings information.
 */
class LayoutBlock {

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
   * Settings for this block.
   */
  var $settings = array();

  /**
   * The content for this block as provided by hook_block_view().
   *
   * @var mixed
   */
  private $content;

  /**
   * The identifier for this instance of this block.
   *
   * @var string
   */
  var $uuid;

  /**
   * If this block is newly created and does not yet have a UUID.
   *
   * @var boolean
   */
  var $is_new = FALSE;

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
  function __construct($module, $delta, $uuid = NULL, $settings = array()) {
    $settings += array(
      'title_display' => LAYOUT_TITLE_DEFAULT,
      'title' => '',
      'block_settings' => array(),
      'style' => array(
        'plugin' => 'default',
        'settings' => array(),
      ),
      'conditions' => array(),
    );

    $this->module = $module;
    $this->delta = $delta;
    $this->uuid = $uuid;

    // Initialize the style.
    $this->style = layout_create_handler('layout_style', $settings['style']['plugin'], $settings['style']['settings']);
    unset($settings['style']);

    // Initialize conditions.
    foreach ($settings['conditions'] as $condition) {
      $this->conditions[] = layout_create_handler('layout_condition', $condition['plugin'], $condition['settings']);
    }
    unset($settings['conditions']);

    $this->settings = $settings;
    if (empty($uuid)) {
      $this->is_new = TRUE;
    }
  }

  protected function buildBlock() {
    if (!isset($this->content)) {
      $this->content = module_invoke($this->module, 'block_view', $this->delta, $this->settings['block_settings']);
    }
    return $this->content;
  }

  /**
   * Return the title of a block as provided by hook_block_view().
   *
   * @return mixed
   */
  function getTitle() {
    if ($this->settings['title_display'] === LAYOUT_TITLE_DEFAULT) {
      $block_content = $this->buildBlock();
      return $block_content['subject'];
    }
    elseif ($this->settings['title']) {
      return $this->settings['title'];
    }
    else {
      // Last effort, use the title from hook_block_info().
      $info = module_invoke($this->module, 'block_info');
      return $info[$this->delta]['info'];
    }
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

    // Load the legacy block form into block_settings.
    $block_form = module_invoke($this->module, 'block_configure', $this->delta, $this->settings['block_settings']);
    if ($block_form) {
      $form['block_settings'] = array(
        '#type' => 'container',
        '#id' => 'layout-block-settings',
      );
      $form['block_settings'] = array_merge($form['block_settings'], $block_form);
    }
  }

  /**
   * Submit handler to save the block settings.
   */
  function formSubmit($form, $form_state) {
    $this->settings['title_display'] = $form_state['values']['title_display']['title_display'];
    $this->settings['title'] = $form_state['values']['title_display']['title'];
    $this->settings['style'] = $form_state['values']['style'];

    // Allow blocks to save their settings into the layout. The last parameter
    // can be taken in by reference and modified.
    module_invoke($this->module, 'block_save', $this->delta, $form_state['values']['block_settings']);
    $this->settings['block_settings'] = $form_state['values']['block_settings'];
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

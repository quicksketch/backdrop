<?php
/**
 * @file
 * Class that wraps around legacy hook_block_view/config/save() hooks.
 */
class LayoutBlockLegacy extends LayoutBlock {
  /**
   * The content for this block as provided by hook_block_view().
   *
   * @var mixed
   */
  private $content;

  /**
   * Build a block using the legacy hook_block_view().
   */
  protected function buildBlock() {
    if (!isset($this->content)) {
      // The old block API does not expect fully built block objects, stub out a
      // pseudo block for the time being.
      $legacy_block = (object) array(
        'module' => $this->module,
        'delta' => $this->delta,
      );
      $this->content = module_invoke($this->module, 'block_view', $this->delta, $this->settings['block_settings']);
      backdrop_alter('block_view', $this->content, $legacy_block);
    }
    return $this->content;
  }

  /**
   * {@inheritdoc}
   */
  function getTitle() {
    $title = parent::getTitle();

    // Use the default title if expected.
    if (empty($title) && $this->settings['title_display'] === LAYOUT_TITLE_DEFAULT) {
      $block_content = $this->buildBlock();
      if (isset($block_content['subject'])) {
        $title = $block_content['subject'];
      }
    }

    return $title;
  }

  /**
   * {@inheritdoc}
   */
  function getContent() {
    $block_content = $this->buildBlock();
    return isset($block_content['content']) ? $block_content['content'] : '';
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    $info = module_invoke($this->module, 'block_info');
    return $info[$this->delta]['info'];
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

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
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    // Allow blocks to save their settings into the layout. The last parameter
    // can be taken in by reference and modified.
    if (isset($form_state['values']['block_settings'])) {
      module_invoke($this->module, 'block_save', $this->delta, $form_state['values']['block_settings']);
      $this->settings['block_settings'] = $form_state['values']['block_settings'];
    }
  }
}

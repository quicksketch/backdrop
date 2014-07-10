<?php
/**
 * @file
 * Class providing forms and settings for the default Layout block style.
 */
class LayoutStyleDynamic extends LayoutStyle {
  /**
   * Constructor for a Layout style dynamic class.
   */
  function __construct($plugin_name, array $settings = array()) {
    $settings += array(
      'wrapper_tag' => 'div',
      'heading_tag' => 'h2',
      'heading_classes' => 'layout-block',
      'content_tag' => 'div',
      'content_classes' => '',
    );
    parent::__construct($plugin_name, $settings);
  }

  /**
   * Specifies the settings form for configuring the style.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    if (!$this->is_region) {
      $tag_list = array(
        'div' => 'DIV',
        'aside' => 'ASIDE',
        'section' => 'SECTION',
        'p' => 'P',
        '' => t('No wrapper'),
      );
      $form['wrapper_tag'] = array(
        '#title' => t('Block tag'),
        '#type' => 'select',
        '#options' => $tag_list,
        '#weight' => -1,
      );
      $form['classes']['#states'] = array(
        'invisible' => array('name="select[wrapper_tag]"' => array('value' => '')),
      );
      $form['heading'] = array(
        '#title' => t('Heading level'),
        '#type' => 'select',
        '#options' => array(
          'h1' => 'H1',
          'h2' => 'H2',
          'h3' => 'H3',
          'h4' => 'H4',
          'h5' => 'H5',
          'h6' => 'H6',
          'div' => 'DIV',
          'p' => 'P',
        ),
        '#default_value' => $this->settings['heading'],
      );
      $form['heading_classes'] = array(
        '#title' => t('Content classes'),
        '#type' => 'textfield',
        '#default_value' => $this->settings['classes'],
        '#description' => t('Additional classes to be added to the block heading.'),
      );
      $form['content_tag'] = array(
        '#title' => t('Content tag'),
        '#type' => 'select',
        '#options' => $tag_list,
      );
      $form['content_classes'] = array(
        '#title' => t('Content classes'),
        '#type' => 'textfield',
        '#default_value' => $this->settings['classes'],
        '#description' => t('Additional classes to be added to the content within this block.'),
        '#states' => array(
          'invisible' => array('name="select[content_tag]"' => array('value' => '')),
        ),
      );
    }
  }
}

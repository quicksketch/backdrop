<?php
/**
 * @file
 * Plugin to provide access control based upon entity bundle.
 */
class EntityBundleLayoutAccess extends LayoutAccess {
  /**
   * Constructor for a Layout access rule.
   */
  function __construct($plugin_name, array $data = array()) {
    parent::__construct($plugin_name, $data);
    $this->settings += array(
      'type' => array()
    );

    // Set entity type.
    $entity_type = explode(':', $this->plugin);
    $this->entity_type = $entity_type[1];
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $entity_info = entity_get_info($this->entity_type);
    foreach ($entity_info['bundles'] as $type => $info) {
      $options[$type] = check_plain($info['label']);
    }

    $form['type'] = array(
      '#title' => t('@entity @bundle', array('@entity' => $entity_info['label'], '@bundle' => $entity_info['bundle label'])),
      '#type' => 'checkboxes',
      '#options' => $options,
      '#default_value' => $this->settings['type'],
    );
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);
    $this->settings['type'] = array_keys(array_filter($form_state['values']['type']));
  }

  /**
   * {@inheritdoc}
   */
  function summary() {
    $entity_info = entity_get_info($this->entity_type);

    $names = array();
    foreach (array_filter($this->settings['type']) as $bundle) {
      $names[] = check_plain($entity_info['bundles'][$bundle]['label']);
    }

    if (empty($names)) {
      return t('Entity is a specific type.');
    }

    return format_plural(count($names), '@entity is "@bundles"', '@entity is one of "@bundles"', array('@entity' => $entity_info['label'], '@bundles' => implode(', ', $names)));
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    if (!$this->settings['type']) {
      return FALSE;
    };

    $entity = $this->contexts[$this->entity_type]->data;
    $bundle = $entity->bundle();

    // There should always be a context at this point, but this is safer.
    if (empty($entity) || !$bundle) {
      return FALSE;
    }

    if (!in_array($bundle, $this->settings['type'])) {
      return FALSE;
    }

    return TRUE;
  }
}

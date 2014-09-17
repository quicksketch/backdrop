<?php
/**
 * @file
 * Layout context handler for entities.
 */
class EntityLayoutContext extends LayoutContext {
  /**
   * The entity type for this context.
   *
   * @var string
   */
  protected $entity_type;

  function __construct($plugin_name, $config = array()) {
    parent::__construct($plugin_name, $config = array());

    $this->entity_type = $plugin_name;
  }

  function type() {
    return 'entity';
  }

  function label() {
    $info = entity_get_info($this->entity_type);
    if (isset($this->settings['id'])) {
      if ($entity = entity_load($this->entity_type, $this->settings['id'])) {
        return t('@type: @nid (@title)', array('@type' => $info['label'], '@id' => $entity->id(), '@title' => $entity->label()));
      }
      else {
        return t('Unknown @type: @id', array('@type' => $info['label'], '@id' =>$this->settings['id']));
      }
    }
    elseif ($this->locked) {
      return t('@type from path', array('@type' => $info['label']));
    }
    else {
      if ($this->entity_type === 'user') {
        return t('Current user');
      }
      else {
        return check_plain($info['label']);
      }
    }
  }
}

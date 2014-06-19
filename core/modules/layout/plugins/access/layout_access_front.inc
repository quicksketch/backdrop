<?php
/**
 * @file
 * Class for loading, modifying, and saving a layout access rule (condition).
 */
class LayoutAccessFront extends LayoutAccessNegatable {
  /**
   * {@inheritdoc}
   */
  function checkAccess($context) {
    return backdrop_is_front_page();
  }

  /**
   * {@inheritdoc}
   */
  function summary() {
    return $this->settings['negate'] ? t('Is not the front page') : t('Is the front page');
  }
}

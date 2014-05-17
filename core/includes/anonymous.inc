<?php
/**
 * Provides a class for managing anonymous users.
 */
class AnonymousUser {
  public $uid = 0;
  public $name = NULL;
  public $hostname = NULL;
  public $roles = array(DRUPAL_ANONYMOUS_RID => 'anonymous user');
  public $cache = 0;

  function __contruct($values = array()) {
    foreach ($values as $key => $value) {
      $this->$key = $value;
    }
  }
}

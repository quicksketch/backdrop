<?php
/**
 * @file
 * Layout context handler for user accounts.
 */
class UserLayoutContext extends LayoutContext {
  function type() {
    return 'user';
  }

  function label() {
    if (isset($this->settings['uid'])) {
      if ($account = user_load($this->settings['uid'])) {
        return t('User account: @uid (@name)', array('@uid' => $account->uid, '@name' => $account->label()));
      }
      else {
        return t('Unknown user: @uid', array('@uid' => $this->settings['uid']));
      }
    }
    elseif ($this->locked) {
      return t('User account from path');
    }
    else {
      return t('Current user');
    }
  }
}

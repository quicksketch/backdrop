<?php

/**
 * @file
 * Class for loading, modifying, and executing a layout.
 */
class Layout {
  /**
   * The name of the layout.
   *
   * @var string
   */
  var $name = '';

  /**
   * The human readable name of the layout.
   *
   * @var string
   */
  var $title = '';

  /**
   * The description of the view, which is used only in the interface.
   *
   * @var string
   */
  var $description;

  /**
   * The module that originally provided this layout (if any).
   *
   * @var string
   */
  var $module;

  /**
   * The internal path of a layout.
   *
   * The layout path must match the provided menu item path. Set and get this
   * variable's value with Layout::getPath() and Layout::setPath().
   *
   * @var string
   */
  private $path;

  /**
   * The weight of this layout compared with other layouts at the same path.
   *
   * @var int
   */
  var $weight = 0;

  /**
   * The storage state of the layout.
   *
   * This is the machine-version of the $type variable, which represents whether
   * this view is has a default, user-created, or overridden configuration.
   * Possible values for this variable include the constants
   * LAYOUT_STORAGE_NORMAL, LAYOUT_STORAGE_OVERRIDE, or LAYOUT_STORAGE_DEFAULT.
   *
   * @var int
   */
  var $storage;

  /**
   * The name of the layout template used by this configuration.
   *
   * @var string
   */
  var $layout;

  /**
   * Whether this layout is disabled.
   *
   * @var boolean
   */
  var $disabled = FALSE;

  /**
   * Whether this layout is locked (being currently edited).
   *
   * @var boolean
   */
  var $locked = FALSE;

  /**
   * An array of all contexts used by this layout.
   *
   * @var array
   */
  var $context = array();

  /**
   * An array of all conditional rules used by this layout.
   *
   * @var array
   */
  var $conditions = array();

  /**
   * An array of all relationships used by this layout.
   *
   * @var array
   */
  //var $relationship = array();

  /**
   * A LayoutMenuItem containing menu properties for this layout.
   *
   * Layouts that are at user-defined paths (not overriding a module) must have
   * a menu item specified. Multiple layouts may share the same menu item.
   *
   * @var LayoutMenuItem
   */
  var $menu_item = NULL;

  /**
   * All user-specified settings for this layout.
   *
   * @var array
   */
  var $settings = array();

  /**
   * The main array of configured blocks, keyed by a generated ID per instance.
   *
   * @var array
   */
  var $content = array();

  /**
   * Nested array that stores the arrangement of the configured blocks.
   *
   * This is keyed first by the region name and the position within that region,
   * and has the value of the block key within the $content array.
   *
   * @var array
   */
  var $positions = array();

  /**
   * An array of LayoutContext instances used by this menu item.
   *
   * @var array
   */
  private $contexts;

  /**
   * Constructor for a Layout class.
   *
   * @param array $config
   *   An array of configuration data.
   */
  function __construct(array $config = array()) {
    foreach ($config as $property => $property_value) {
      $this->{$property} = $property_value;
    }

    // Specify default settings.
    $this->settings += array(
      'title' => '',
      'title_display' => LAYOUT_TITLE_DEFAULT,
      'title_block' => NULL,
    );

    // Set the storage type.
    if (isset($config['module'])) {
      if (empty($config['storage']) || $config['storage'] == LAYOUT_STORAGE_DEFAULT) {
        $this->storage = LAYOUT_STORAGE_DEFAULT;
      }
      else {
        $this->storage = LAYOUT_STORAGE_OVERRIDE;
      }
    }
    else {
      $this->storage = LAYOUT_STORAGE_NORMAL;
    }

    // Convert all stored contexts, conditions, and relationships to their
    // handler equivalents.
    $handlers = array(
      // Items are key => handler_type.
      'content' => 'block',
      'conditions' => 'layout_access',
      // 'contexts' => 'layout_context',
    );
    foreach ($handlers as $property_key => $plugin_type) {
      foreach ($this->{$property_key} as $plugin_type_key => $plugin_data) {
        $this->{$property_key}[$plugin_type_key] = layout_create_handler($plugin_type, $plugin_data['plugin'], $plugin_data['data']);
      }
    }
  }

  /**
   * Save a layout to config.
   */
  function save() {
    $data = array(
      'path' => $this->path,
      'name' => $this->name,
      'title' => $this->title,
      'description' => $this->description,
      'module' => $this->module,
      'weight' => $this->weight,
      'storage' => $this->storage,
      'layout' => $this->layout,
      'disabled' => $this->disabled,
      'settings' => $this->settings,
      'positions' => $this->positions,
    );

    // Convert all stored conditions to arrays.
    // @todo: Save per-layout contexts and relationships.
    $sub_parts = array(
      'content',
      'conditions',
      // 'contexts',
      // 'relationships',
    );
    foreach ($sub_parts as $config_type) {
      foreach ($this->$config_type as $config_type_key => $config_type_data) {
        unset($config_type_data->is_new);
        $data[$config_type][$config_type_key] = array(
          'plugin' => $config_type_data->plugin,
          'data' => $config_type_data->toArray(),
        );
      }
    }

    if (isset($this->original_name) && $this->original_name != $this->name) {
      config('layout.layout.' . $this->original_name)->delete();
    }
    config('layout.layout.' . $this->name)
      ->setData($data)
      ->save();

    // If this layout overrides an existing module path, reassign or delete
    // the layout menu item.
    if (menu_get_item($this->path) && layout_provides_path($this->path) === FALSE) {
      if ($this->menu_item) {
        $this->menu_item->delete();
      }
    }
    // If not overriding an existing module path, ensure that the layout always
    // has a layout menu item at the same path.
    else {
      if ($this->menu_item) {
        // If an existing menu item exists but this layout's path has changed,
        // reassign/delete the existing menu item, then create a new one.
        if ($this->menu_item->path !== $this->path) {
          $new_menu_item = clone($this->menu_item);
          $new_menu_item->path = $this->path;
          $new_menu_item->name = $this->name;
          $new_menu_item->is_new = TRUE;
          $this->menu_item->reassign();
          $this->menu_item = $new_menu_item;
          $this->menu_item->save();
        }
        else {
          if (isset($this->original_name) && $this->original_name !== $this->name && $this->menu_item->name === $this->original_name) {
            config('layout.menu_item.' . $this->original_name)->delete();
          }
          $this->menu_item->save();
        }
      }
    }
    layout_reset_caches();
  }

  /**
   * Delete this layout.
   */
  function delete() {
    if ($this->storage === LAYOUT_STORAGE_NORMAL) {
      config('layout.layout.' . $this->name)->delete();
      // Reassign the menu item to a still-enabled layout (if any).
      if ($this->menu_item && $this->menu_item->name === $this->name) {
        $this->menu_item->reassign();
      }
      layout_reset_caches();
    }
    else {
      $this->disable();
    }
  }

  /**
   * Revert a layout to a module-provided default.
   */
  function revert() {
    if (!empty($this->module)) {
      // Restore the module-provided layout config.
      config_install_default_config($this->module, 'layout.layout.' . $this->name);

      // Restore the module-provided menu settings if this layout provides the
      // active menu item.
      if ($this->menu_item && $this->menu_item->name === $this->name) {
        $this->menu_item->revert();
      }
    }
  }

  /**
   * Disable a layout.
   */
  function disable() {
    $this->disabled = TRUE;
    $this->save();

    // Reassign the menu item to a still-enabled layout (if any).
    if ($this->menu_item && $this->menu_item->name === $this->name) {
      $this->menu_item->reassign();
    }
  }

  /**
   * Enable a layout.
   */
  function enable() {
    $this->disabled = FALSE;
    $this->save();

    // Check if the menu item needs to be assigned to this layout.
    if ($this->menu_item->name === $this->name) {
      $this->menu_item->reassign();
    }
  }

  /**
   * Return a form for configuring this layout's settings.
   */
  function form(&$form, &$form_state) {
    // Most layouts do not have a form.
  }

  /**
   * Validate the settings form.
   */
  function formValidate($form, &$form_state) {
    // Most layouts do not have a form.
  }

  /**
   * Save the settings added in the form method.
   */
  function formSubmit($form, &$form_state) {
    // Most layouts do not have a form.
  }

  /**
   * Set a layout path.
   */
  function setPath($path) {
    if (empty($this->menu_item) || $this->menu_item->path !== $path) {
      // Use an existing menu item if existing.
      if ($existing_item = layout_menu_item_load_by_path($path)) {
        $this->menu_item = $existing_item;
      }
      // If this layout doesn't have a menu item, make one. If the path is
      // provided by another module, no menu item is necessary.
      elseif (layout_provides_path($path) === NULL) {
        $menu_item_settings = array(
          'path' => $path,
          'name' => $this->name,
        );
        $this->menu_item = new LayoutMenuItem($menu_item_settings);
      }
      else {
        if ($this->menu_item) {
          $this->menu_item->delete();
        }
        $this->menu_item = NULL;
      }
    }
    $this->path = $path;
  }

  /**
   * Return a layout's path based on its assigned menu item.
   *
   * @return string
   */
  function getPath() {
    if ($this->menu_item) {
      return $this->menu_item->path;
    }
    else {
      return $this->path;
    }
  }

  /**
   * Return all contexts (from both the layout and menu item) for this Layout.
   */
  function getContexts() {
    if (is_null($this->contexts)) {
      if ($this->menu_item) {
        $this->contexts = $this->menu_item->getContexts();
      }
      elseif ($this->path) {
        $this->contexts = layout_context_required_by_path($this->path);
      }
      else {
        $this->contexts = array();
      }
    }

    // Add on the current user context, which is always available.
    if (!isset($this->contexts['current_user'])) {
      $this->contexts['current_user'] = layout_create_context('user');
      $this->contexts['current_user']->setData($GLOBALS['user']);
      $this->contexts['current_user']->plugin = 'user';
    }

    // Add on the overrides path context, which is always available.
    if (empty($this->menu_item) && !isset($this->contexts['overrides_path'])) {
      $this->contexts['overrides_path'] = layout_create_context('overrides_path');
      $this->contexts['overrides_path']->plugin = 'overrides_path';
    }

    return $this->contexts;
  }

  /**
   * Reset the internally stored contexts.
   *
   * This is used before storing a layout, or when fresh contexts are important,
   * such as when actively changing the layout's path.
   */
  function resetContexts() {
    $this->contexts = NULL;
  }

  /**
   * Check if the layout has a context of a particular name.
   *
   * @param array $required_contexts
   *   An unindexed array of context plugin names.
   * @return boolean
   *   TRUE if this layout has all the required contexts, FALSE otherwise.
   */
  function hasContexts($required_contexts) {
    $all_contexts = $this->getContexts();
    foreach ($required_contexts as $required_context_name) {
      $context_missing = TRUE;
      foreach ($all_contexts as $context) {
        if ($context->plugin === $required_context_name) {
          // Matching context available, continue to the next one.
          $context_missing = FALSE;
          break;
        }
      }
      if ($context_missing) {
        return FALSE;
      }
    }

    return TRUE;
  }

  /**
   * Check if this layout is a fallback default layout.
   */
  function isDefault() {
    return in_array($this->name, array('layout_default', 'layout_admin_default'));
  }

  /**
   * Check access to this layout based on the current contexts.
   *
   * This method is generally called on all layouts that share the same path.
   * The first layout that grants access will be used to render the page, and if
   * no layouts grant access, then Layout module will fall through to using the
   * default site-wide layout.
   */
  function checkAccess() {
    $contexts = $this->getContexts();
    foreach ($this->conditions as $condition) {
      $condition->setContexts($contexts);
      if (!$condition->checkAccess()) {
        return FALSE;
      }
    }
    return TRUE;
  }
}


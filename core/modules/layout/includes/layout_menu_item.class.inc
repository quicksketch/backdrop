<?php
/**
 * @file
 * Class for storing information about menu item entries.
 *
 * Menu item entries may be shared between multiple layouts. Several
 * different layouts may exist at the same path with different selection
 * criteria, i.e. a menu local task (tab) at node/%/example.
 */
class LayoutMenuItem {

  /**
   * The machine name of the layout menu item.
   *
   * This name usually matches the layout name that created the menu item. If
   * multiple layouts are at the same path, this name will match the layout with
   * the highest weight, which is typically the "fallback" layout at that path.
   *
   * @var string
   */
  var $name = '';

  /**
   * The title of this menu item.
   *
   * @var string
   */
  var $title = '';

  /**
   * The menu path of this item.
   *
   * @var string
   */
  var $path = '';

  /**
   * The settings for this menu item's menu link (if any).
   *
   * This contains at least the following properties:
   *  - title: The title of this menu item link.
   *  - weight: The position in the menu.
   *  - type: The menu item type.
   *  - parent: The parent menu item information, including name and weight if
   *    this item is in a menu.
   *
   * @var array
   */
  var $menu = array(
    'title' => NULL,
    'weight' => 0,
    'type' => 'none',
    'parent' => NULL,
  );

  /**
   * The module that originally provided this layout (if any).
   *
   * @var string
   */
  var $module = NULL;

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
  var $storage = LAYOUT_STORAGE_NORMAL;

  /**
   * Constructor for LayoutMenuItems.
   */
  function __construct($data) {
    foreach ($data as $key => $value) {
      $this->$key = $value;
    }
  }

  /**
   * Save a menu item configuration.
   */
  function save() {
    // Convert all properties to an array.
    $data = (array) $this;

    config('layout.menu_item.' . $this->name)
      ->setData($data)
      ->save();
    layout_reset_caches();
    menu_rebuild();
  }

  /**
   * Delete a menu item configuration.
   */
  function delete() {
    config('layout.menu_item.' . $this->name)
      ->delete();
    layout_reset_caches();
    menu_rebuild();
  }

  /**
   * Revert a menu item back to its module-provided default.
   */
  function revert() {
    if ($this->module) {
      config_install_default_config($this->module, 'layout.menu_item.' . $this->name);
      layout_reset_caches();
      menu_rebuild();
    }
  }

  /**
   * Rename this menu item to match the most appropriate layout at the same path.
   *
   * If this menu item's parent layouts have been edited, reordered, or deleted
   * the menu item may need to be reassigned to a different layout. This checks
   * all of the layouts that match this menu item's path, and renames this menu
   * item to match the highest weighted layout (which is usually the fallback
   * layout at a particular path). If no layouts are associated with this
   * particular path any more, this menu item will be deleted.
   */
  function reassign() {
    // Before reassigning, clear caches so that our layout list is up-to-date.
    layout_reset_caches();
    $layout_names = layout_get_path_layout_names($this->path);

    // If affiliated with layouts, take the last one and match its name.
    if (count($layout_names)) {
      $layout_name = array_pop($layout_names);
      if ($this->name != $layout_name) {
        $this->delete();
        $this->name = $layout_name;
        $this->save();
      }
    }
    // If no longer affiliated with any layouts, delete this menu item.
    else {
      $this->delete();
    }

    layout_reset_caches();
    menu_rebuild();
  }
}
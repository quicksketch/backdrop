<?php

/**
 * @file
 * Class file to control the main Layout editor.
 */

class LayoutRendererEditor extends LayoutRendererStandard {

  /**
   * An array of AJAX commands to return. If populated it will automatically
   * be used by the AJAX router.
   */
  var $commands = array();
  var $admin = TRUE;

  /**
   * Set to true if edit links (for blocks and regions) should not be displayed.
   * This can be used for special edit modes such as layout change and layout
   * builder that do not actually have real content.
   */
  var $no_edit_links = FALSE;

  // -------------------------------------------------------------------------
  // Display edit rendering.

  function edit() {
    $form_state = array(
      'layout' => &$this->layout,
      'renderer' => &$this,
      'content_types' => $this->cache->content_types,
      'no_redirect' => TRUE,
      'layout_title' => !empty($this->cache->layout_title),
    );

    $output = backdrop_build_form('layout_edit_layout_form', $form_state);
    if (empty($form_state['executed']) || !empty($form_state['clicked_button']['preview'])) {
      return $output;
    }

    if (!empty($form_state['clicked_button']['#save-layout'])) {
      backdrop_set_message(t('Panel content has been updated.'));
      layout_save_layout($this->layout);
    }
    else {
      backdrop_set_message(t('Your changes have been discarded.'));
    }

    layout_cache_clear('layout', $this->layout->did);
    return $this->layout;
  }

  function add_meta() {
    parent::add_meta();
    if ($this->admin) {
      backdrop_add_library('system', 'ui.sortable');
      backdrop_add_js(backdrop_get_path('module', 'layout') . '/js/layout.admin.js');
      backdrop_add_css(backdrop_get_path('module', 'layout') . '/css/layout.admin.css');
    }
  }

  function render() {
    // Pass through to normal rendering if not in admin mode.
    if (!$this->admin) {
      return parent::render();
    }

    $this->add_meta();

    $output = '<div id="layout-edit-main">';
    $output .= $this->render_layout();
    $output .= '</div>';

    return $output;
  }

  function render_region($region_id, $blocks) {
    // Pass through to normal rendering if not in admin mode.
    if (!$this->admin) {
      return parent::render_region($region_id, $blocks);
    }

    $content = implode('', $blocks);

    $region_buttons = $this->get_region_links($region_id);

    $output = '<div class="layout-editor-region" id="layout-editor-region-' . $region_id . '" data-region-name="' . $region_id . '">';
    $output .= '<div class="layout-editor-region-title clearfix">';
    $output .= '<h2 class="label">' . check_plain($this->layout_info['regions'][$region_id]) . '</h2>';
    $output .= $region_buttons;
    $output .= '</div>';
    $output .= '<div class="layout-editor-region-content">' . $content . '</div>';
    $output .= '</div>';

    return $output;
  }

  function render_block(&$block) {
    // Pass through to normal rendering if not in admin mode.
    if (!$this->admin) {
      return parent::render_block($block);
    }

    $buttons = $this->get_block_links($block);
    $content = $block->getAdminPreview();

    $class = 'layout-editor-block';
    if (isset($this->layout->title_block) && $this->layout->title_block == $block->uuid) {
      $class .= ' layout-editor-block-is-title';
    }
    $class .= ' clearfix';

    $output = '<div class="' . $class . '" id="layout-editor-block-' . $block->uuid . '" data-block-id="' . $block->uuid . '">';
    $output .= '<div class="layout-editor-block-title clearfix">';
    $output .= '<span class="text">' . $block->getAdminTitle() . '</span>';
    if ($buttons) {
      $output .= '<span class="buttons">' . $buttons . '</span>';
    }
    $output .= '</div>'; // layout-block-title
    $output .= '<div class="layout-editor-block-content">' . render($content) . '</div>';
    $output .= '</div>'; // layout-block

    return $output;
  }

  /**
   * Render the links to display when editing a region.
   */
  function get_region_links($region_id) {
    if (!empty($this->no_edit_links)) {
      return '';
    }
    $links = array();
    $links['add'] = array(
      'title' => t('Add content'),
      'href' => $this->get_url('add-block', $region_id),
      'attributes' => array(
        'class' => array('use-ajax'),
        'data-dialog' => TRUE,
        'data-dialog-options' => json_encode(array('dialogClass' => 'layout-dialog')),
      ),
    );

    $dropbutton = array(
      '#type' => 'dropbutton',
      '#links' => $links,
    );

    return backdrop_render($dropbutton);
  }

  /**
   * Render the links to display when editing a block.
   */
  function get_block_links($block) {
    if (!empty($this->no_edit_links)) {
      return '';
    }

    $links = array();
    $links['configure'] = array(
      'title' => t('Configure'),
      'href' => $this->get_url('configure-block', $block->uuid),
      'attributes' => array(
        'class' => array('use-ajax'),
        'data-dialog' => TRUE,
        'data-dialog-options' => json_encode(array('dialogClass' => 'layout-dialog')),
      ),
    );

    $links['remove'] = array(
      'title' => t('Remove'),
      'href' => $this->get_url('remove-block', $block->uuid),
      'query' => array('token' => backdrop_get_token($this->layout->name)),
      'attributes' => array(
        'class' => array('remove-block', 'use-ajax'),
      ),
    );

    $dropbutton = array(
      '#type' => 'dropbutton',
      '#links' => $links,
    );

    return backdrop_render($dropbutton);
  }

  // -----------------------------------------------------------------------
  // Display edit AJAX callbacks and helpers.

  /**
   * Generate a URL path for the AJAX editor.
   */
  function get_url() {
    $args = func_get_args();
    $command = array_shift($args);
    $url = 'admin/structure/layouts/manage/' . $this->layout->name . '/' . $command . '/' . $this->plugin['name'];
    if ($args) {
      $url .= '/' . implode('/', $args);
    }

    return $url;
  }

  /**
   * Get the appropriate style from the layout in the cache.
   *
   * Since we have styles for regions, blocks and the layout itself, and
   * they are stored differently, we use this method to simplify getting
   * style information into a way that's easy to cope with.
   */
  function get_style($type, $id = '') {
    if (isset($this->cache->style)) {
      $style = layout_get_style_info($this->cache->style);
      $defaults = isset($style['defaults']) ? $style['defaults'] : array();
      // Get the &$conf variable based upon whose style we're editing.
      switch ($type) {
        case 'layout':
          $this->settings['style'] = $this->cache->style;
          $this->settings['style_settings']['default'] = $defaults;
          break;

        case 'region':
          $this->settings[$id]['style'] = $this->cache->style;
          $this->settings['style_settings'][$id] = $defaults;
          break;

        case 'block':
          $block = &$this->content[$id];
          $block->style['style'] = $this->cache->style;
          $block->style['settings'] = $defaults;
          $conf = &$block->style['settings'];
          break;
      }
    }
    else {
      switch ($type) {
        case 'layout':
          $style = layout_get_style_info((!empty($this->settings['style'])) ? $this->settings['style'] : 'default');
          break;

        case 'region':
          $style = layout_get_style_info((!empty($this->settings[$id]['style'])) ? $this->settings[$id]['style'] : '-1');
          break;

        case 'block':
          $block = &$this->content[$id];
          $style = layout_get_style_info(!empty($block->style['style']) ? $block->style['style'] : 'default');
          break;
      }
    }

    // Set up our $conf reference.
    switch ($type) {
      case 'layout':
        $conf = &$this->settings['style_settings']['default'];
        break;

      case 'region':
        $conf = &$this->settings['style_settings'][$id];
        break;

      case 'block':
        $block = &$this->content[$id];
        $conf = &$block->style['settings'];
        break;
    }

    return array($style, &$conf);
  }
}

<?php

/**
 * The standard render pipeline for a Panels display object.
 *
 * Given a fully-loaded panels_display object, this class will turn its
 * combination of layout, panes, and styles into HTML, invoking caching
 * appropriately along the way. Interacting with the renderer externally is
 * very simple - just pass it the display object and call the render() method:
 *
 * @code
 *   // given that $display is a fully loaded Panels display object
 *   $renderer = panels_get_renderer_handler('standard', $display)
 *   $html_output = $renderer->render();
 * @endcode
 *
 * Internally, the render pipeline is divided into two phases, prepare and
 * render:
 *   - The prepare phase transforms the skeletal data on the provided
 *     display object into a structure that is expected by the render phase.
 *     It is divided into a series of discrete sub-methods and operates
 *     primarily by passing parameters, all with the intention of making
 *     subclassing easier.
 *   - The render phase relies primarily on data stored in the renderer object's
 *     properties, presumably set in the prepare phase. It iterates through the
 *     rendering of each pane, pane styling, placement in panel regions, region
 *     styling, and finally the arrangement of rendered regions in the layout.
 *     Caching, if in use, is triggered per pane, or on the entire display.
 *
 * In short: prepare builds conf, render renders conf. Subclasses should respect
 * this separation of responsibilities by adhering to these loose guidelines,
 * given a loaded display object:
 *   - If your renderer needs to modify the datastructure representing what is
 *     to be rendered (panes and their conf, styles, caching, etc.), it should
 *     use the prepare phase.
 *   - If your renderer needs to modify the manner in which that renderable
 *     datastructure data is rendered, it should use the render phase.
 *
 * In the vast majority of use cases, this standard renderer will be sufficient
 * and need not be switched out/subclassed; style and/or layout plugins can
 * accommodate nearly every use case. If you think you might need a custom
 * renderer, consider the following criteria/examples:
 *   - Some additional markup needs to be added to EVERY SINGLE panel.
 *   - Given a full display object, just render one pane.
 *   - Show a Panels admin interface.
 *
 * The system is almost functionally identical to the old procedural approach,
 * with some exceptions (@see panels_renderer_legacy for details). The approach
 * here differs primarily in its friendliness to tweaking in subclasses.
 */
class LayoutRendererStandard {
  /**
   * The fully-loaded Layout object that is to be rendered.
   *
   * @var Layout
   */
  var $layout;

  /**
   * The information from hook_layout_info() for the layout being rendered.
   *
   * This array is stored here for easy access, as it contains most importantly
   * the list of regions that can be configured.
   *
   * @var array
   */
  var $layout_info;

  /**
   * A multilevel array of rendered data. The first level of the array
   * indicates the type of rendered data, typically with up to three keys:
   * 'layout', 'regions', and 'panes'. The relevant rendered data is stored as
   * the value for each of these keys as it is generated:
   *  - 'panes' are an associative array of rendered output, keyed on pane id.
   *  - 'regions' are an associative array of rendered output, keyed on region
   *    name.
   *  - 'layout' is the whole of the rendered output.
   *
   * @var array
   */
  var $rendered = array();

  /**
   * A multilevel array of data prepared for rendering. The first level of the
   * array indicates the type of prepared data. The standard renderer populates
   * and uses two top-level keys, 'panes' and 'regions':
   *  - 'panes' are an associative array of pane objects to be rendered, keyed
   *    on pane id and sorted into proper rendering order.
   *  - 'regions' are an associative array of regions, keyed on region name,
   *    each of which is itself an indexed array of pane ids in the order in
   *    which those panes appear in that region.
   *
   * @var array
   */
  var $prepared = array();

  /**
   * Boolean state variable, indicating whether or not the prepare() method has
   * been run.
   *
   * This state is checked in panels_renderer_standard::render_layout() to
   * determine whether the prepare method should be automatically triggered.
   *
   * @var bool
   */
  var $prep_run = FALSE;

  /**
   * The plugin definition that defines this renderer handler.
   *
   * @var array
   */
  var $plugin;

  /**
   * TRUE if this renderer is rendering in administrative mode
   * which will allow layouts to have extra functionality.
   *
   * @var bool
   */
  var $admin = FALSE;

  /**
   * Where to add standard meta information. There are three possibilities:
   * - standard: Put the meta information in the normal location. Default.
   * - inline: Put the meta information directly inline. This will
   *   not work for javascript.
   *
   * @var string
   */
  var $meta_location = 'standard';

  /**
   * Include rendered HTML prior to the layout.
   *
   * @var string
   */
  var $prefix = '';

  /**
   * Include rendered HTML after the layout.
   *
   * @var string
   */
  var $suffix = '';

  /**
   * Construct the object that will be used to render the layout.
   *
   * @param Layout $layout
   *   The layout object to be rendered.
   * @param array $plugin
   *   The definition of the renderer plugin.
   */
  function __construct(Layout $layout, array $renderer_plugin) {
    $this->plugin = $renderer_plugin;
    $layout_info = layout_get_layout_info($layout->layout);
    $this->layout = &$layout;

    if (empty($layout_info)) {
      throw new LayoutMissingException(t('The layout plugin "@plugin" could not been found to display the "@title" layout.', array('@plugin' => $layout->layout, '@title' => $layout->title)));
    }
    else {
      $this->layout_info = $layout_info;
    }
  }

  /**
   * Prepare the attached display for rendering.
   *
   * This is the outermost prepare method. It calls several sub-methods as part
   * of the overall preparation process. This compartmentalization is intended
   * to ease the task of modifying renderer behavior in child classes.
   *
   * If you override this method, it is important that you either call this
   * method via parent::prepare(), or manually set $this->prep_run = TRUE.
   *
   * @param mixed $external_settings
   *  An optional parameter allowing external code to pass in additional
   *  settings for use in the preparation process. Not used in the default
   *  renderer, but included for interface consistency.
   */
  function prepare($external_settings = NULL) {
    $this->prepare_blocks($this->layout->content);
    $this->prepare_regions($this->layout->positions, $this->layout->settings);
    $this->prep_run = TRUE;
  }

  /**
   * Prepare the list of panes to be rendered, accounting for visibility/access
   * settings and rendering order.
   *
   * This method represents the standard approach for determining the list of
   * panes to be rendered that is compatible with all parts of the Panels
   * architecture. It first applies visibility & access checks, then sorts panes
   * into their proper rendering order, and returns the result as an array.
   *
   * Inheriting classes should override this method if that renderer needs to
   * regularly make additions to the set of panes that will be rendered.
   *
   * @param array $blocks
   *  An associative array of block data (stdClass objects), keyed on block ID.
   * @return array
   *  An associative array of panes to be rendered, keyed on pane id and sorted
   *  into proper rendering order.
   */
  function prepare_blocks($blocks) {
    // Use local variables as writing to them is very slightly faster
    $first = $normal = $last = array();

    // Prepare the list of panes to be rendered
    foreach ($blocks as $uuid => $block) {
      if (empty($this->admin)) {
        // If this pane is not visible to the user, skip out and do the next one
        if (!$block->shown || !layout_block_access($block, $this->layout)) {
          continue;
        }
      }

      $block_info = layout_get_block_info($block->module, $block->delta);

      // If this pane wants to render last, add it to the $last array. We allow
      // this because some panes need to be rendered after other panes,
      // primarily so they can do things like the leftovers of forms.
      if (!empty($block_info['render last'])) {
        $last[$uuid] = $block;
      }
      // If it wants to render first, add it to the $first array. This is used
      // by panes that need to do some processing before other panes are
      // rendered.
      else if (!empty($block_info['render first'])) {
        $first[$uuid] = $block;
      }
      // Otherwise, render it in the normal order.
      else {
        $normal[$uuid] = $block;
      }
    }

    $this->prepared['blocks'] = $first + $normal + $last;
    return $this->prepared['blocks'];
  }

  /**
   * Prepare the list of regions to be rendered.
   *
   * This method is primarily about properly initializing the style plugin that
   * will be used to render the region. This is crucial as regions cannot be
   * rendered without a style plugin (in keeping with Layout's philosophy of
   * hardcoding none of its output), but for most regions no style has been
   * explicitly set. The logic here is what accommodates that situation:
   *  - If a region has had its style explicitly set, then we fetch that plugin
   *    and continue.
   *  - If the region has no explicit style, but a style was set at the layout
   *    level, then inherit the style from the layout.
   *  - If neither the region nor the layout have explicitly set styles, then
   *    fall back to the hardcoded 'default' style, a very minimal style.
   *
   * The other important task accomplished by this method is ensuring that even
   * regions without any blocks are still properly prepared for the rendering
   * process. This is essential because the way layouts are loaded, results only
   * in a list of regions that contain blocks - not necessarily all the regions
   * defined by the layout plugin, which can only be determined by asking the
   * plugin at runtime. This method consults that retrieved list of regions and
   * prepares all of those, ensuring none are inadvertently skipped.
   *
   * @param array $region_block_list
   *   An associative array of block IDs, keyed on the region to which those IDs
   *   are assigned.
   * @param array $settings
   *   All known region style settings, including both the top-level layout's
   *   settings (if any) and all region-specific settings (if any).
   * @return array
   *   An array of regions prepared for rendering.
   */
  function prepare_regions($region_block_list, $settings) {
    // Initialize defaults to be used for regions without their own explicit
    // settings. Use display settings if they exist, else hardcoded defaults.
    $default = array(
      'style' => !empty($settings['style']) ? $settings['style'] : 'default',
      'style settings' => isset($settings['style_settings']['default']) ? $settings['style_settings']['default'] : array(),
    );

    $regions = array();
    if (empty($settings)) {
      // No region settings exist, init all with the defaults.
      foreach ($this->layout_info['regions'] as $region_id => $title) {
        // Ensure this region has at least an empty panes array.
        $blocks = !empty($region_block_list[$region_id]) ? $region_block_list[$region_id] : array();

        $regions[$region_id] = $default;
        $regions[$region_id]['uuids'] = $blocks;
      }
    }
    else {
      // Some settings exist; iterate through each region and set individually.
      foreach ($this->layout_info['regions'] as $region_id => $title) {
        // Ensure this region has at least an empty blocks array.
        $blocks = !empty($region_block_list[$region_id]) ? $region_block_list[$region_id] : array();
        if (empty($settings[$region_id]['style']) || $settings[$region_id]['style'] == -1) {
          $regions[$region_id] = $default;
        }
        else {
          $regions[$region_id]['style'] = $settings[$region_id]['style'];
          $regions[$region_id]['style settings'] = isset($settings['style_settings'][$region_id]) ? $settings['style_settings'][$region_id] : array();
        }
        $regions[$region_id]['uuids'] = $blocks;
      }
    }

    $this->prepared['regions'] = $regions;
    return $this->prepared['regions'];
  }

  /**
   * Build inner content, then hand off to layout-specified theme function for
   * final render step.
   *
   * This is the outermost method in the Panels render pipeline. It calls the
   * inner methods, which return a content array, which is in turn passed to the
   * theme function specified in the layout plugin.
   *
   * @return string
   *  Themed & rendered HTML output.
   */
  function render() {
    // Attach out-of-band data first.
    $this->add_meta();

    if (empty($this->layout->cache['method']) || !empty($this->layout->skip_cache)) {
      return $this->render_layout();
    }
    else {
      $cache = panels_get_cached_content($this->layout, $this->layout->args, $this->layout->context);
      if ($cache === FALSE) {
        $cache = new panels_cache_object();
        $cache->set_content($this->render_layout());
        panels_set_cached_content($cache, $this->layout, $this->layout->args, $this->layout->context);
      }
      return $cache->content;
    }
  }

  /**
   * Perform display/layout-level render operations.
   *
   * This method triggers all the inner pane/region rendering processes, passes
   * that to the layout plugin's theme callback, and returns the rendered HTML.
   *
   * If display-level caching is enabled and that cache is warm, this method
   * will not be called.
   *
   * @return string
   *   The HTML string representing the entire rendered, themed panel.
   */
  function render_layout() {
    if (empty($this->prep_run)) {
      $this->prepare();
    }
    $this->render_blocks();
    $this->render_regions();

    if ($this->admin && !empty($this->layout_info['admin theme'])) {
      $theme = $this->layout_info['admin theme'];
    }
    else {
      $theme = $this->layout_info['theme'];
    }

    $this->rendered['layout'] = theme($theme, array('content' => $this->rendered['regions'], 'settings' => $this->layout->settings, 'layout' => $this->layout, 'layout_info' => $this->layout_info, 'renderer' => $this));
    return $this->prefix . $this->rendered['layout'] . $this->suffix;
  }

  /**
   * Attach out-of-band page metadata (e.g., CSS and JS).
   *
   * This must be done before render, because layouts-within-layouts must have
   * their CSS added in the right order: inner content before outer content.
   */
  function add_meta() {
    if (!empty($this->layout_info['css'])) {
      if (file_exists(path_to_theme() . '/' . $this->layout_info['css'])) {
        $this->add_css(path_to_theme() . '/' . $this->layout_info['css']);
      }
      else {
        $this->add_css($this->layout_info['path'] . '/' . $this->layout_info['css']);
      }
    }

    if ($this->admin && isset($this->layout_info['admin css'])) {
      $this->add_css($this->layout_info['path'] . '/' . $this->layout_info['admin css']);
    }
  }

  /**
   * Add CSS information to the renderer.
   *
   * To facilitate previews over Views, CSS can now be added in a manner
   * that does not necessarily mean just using backdrop_add_css. Therefore,
   * during the panel rendering process, this method can be used to add
   * css and make certain that ti gets to the proper location.
   *
   * The arguments should exactly match backdrop_add_css().
   *
   * @see backdrop_add_css
   */
  function add_css($filename) {
    switch ($this->meta_location) {
      case 'standard':
        backdrop_add_css($filename);
        break;
      case 'inline':
        $url = base_path() . $filename;
        $this->prefix .= '<link type="text/css" rel="stylesheet" href="' . $url . '" />'."\n";
        break;
    }
  }

  /**
   * Render all prepared panes, first by dispatching to their plugin's render
   * callback, then handing that output off to the pane's style plugin.
   *
   * @return array
   *   The array of rendered panes, keyed on pane pid.
   */
  function render_blocks() {
    // First, render all the panes into little boxes.
    $this->rendered['blocks'] = array();
    foreach ($this->prepared['blocks'] as $uuid => $block) {
      $content = $this->render_block($block);
      if ($content) {
        $this->rendered['blocks'][$uuid] = $content;
      }
    }
    return $this->rendered['blocks'];
  }

  /**
   * Render a pane using its designated style.
   *
   * This method also manages 'title pane' functionality, where the title from
   * an individual pane can be bubbled up to take over the title for the entire
   * display.
   *
   * @param stdClass $pane
   *  A Panels pane object, as loaded from the database.
   */
  function render_block(&$block) {
    $content = $this->render_block_content($block);
    if ($this->layout->settings['title_display'] == LAYOUT_TITLE_BLOCK && !empty($this->layout->settings['title_pane']) && $this->layout->settings['title_pane'] == $block->uuid) {

      // If the user selected to override the title with nothing, and selected
      // this as the title pane, assume the user actually wanted the original
      // title to bubble up to the top but not actually be used on the pane.
      if (empty($content->title) && !empty($content->original_title)) {
        $this->layout->stored_pane_title = $content->original_title;
      }
      else {
        $this->layout->stored_pane_title = !empty($content->title) ? $content->title : '';
      }
    }

    if (!empty($content->content)) {
      $classes = array();
      if (isset($block->style['settings']['classes'])) {
        $classes = explode(' ', $block->style['settings']['classes']);
      }
      if (!empty($block->style['style'])) {
        $style = layout_get_style_info($block->style['style']);

        if (isset($style) && isset($style['block theme'])) {
          $output = theme($style['block theme'], array('content' => $content, 'block' => $block, 'style' => $style, 'settings' => $block->style['settings'], 'classes' => $classes));
          // This could be null if no theme function existed.
          if (isset($output)) {
            return $output;
          }
        }
      }

      // Fallback.
      return theme('layout_block', array('content' => $content, 'block' => $block, 'display' => $this->layout));
    }
  }

  /**
   * Render the interior contents of a single pane.
   *
   * This method retrieves pane content and produces a ready-to-render content
   * object. It also manages pane-specific caching.
   *
   * @param stdClass $pane
   *   A Panels pane object, as loaded from the database.
   * @return stdClass $content
   *   A renderable object, containing a subject, content, etc. Based on the
   *   renderable objects used by the block system.
   */
  function render_block_content(&$pane) {
    ctools_include('context');
    // TODO finally safe to remove this check?
    if (!is_array($this->layout->context)) {
      watchdog('panels', 'renderer::render_block_content() hit with a non-array for the context', $this->layout, WATCHDOG_DEBUG);
      $this->layout->context = array();
    }

    $content = FALSE;
    $caching = !empty($block->cache['method']) && empty($this->layout->skip_cache);
    if ($caching && ($cache = panels_get_cached_content($this->layout, $this->layout->args, $this->layout->context, $pane))) {
      $content = $cache->content;
    }
    else {
      if ($caching) {
        // This is created before rendering so that calls to backdrop_add_js
        // and backdrop_add_css will be captured.
        $cache = new panels_cache_object();
      }

      $content = ctools_content_render($block->type, $block->subtype, $block->settings, array(), $this->layout->args, $this->layout->context);

      if (empty($content)) {
        return;
      }

      foreach (module_implements('panels_pane_content_alter') as $module) {
        $function = $module . '_panels_pane_content_alter';
        $function($content, $pane, $this->layout->args, $this->layout->context, $this, $this->layout);
      }
      if ($caching && isset($cache)) {
        $cache->set_content($content);
        panels_set_cached_content($cache, $this->layout, $this->layout->args, $this->layout->context, $pane);
        $content = $cache->content;
      }
    }

    // Pass long the css_id that is usually available.
    if (!empty($block->css['css_id'])) {
      $content->css_id = check_plain($block->css['css_id']);
    }

    // Pass long the css_class that is usually available.
    if (!empty($block->css['css_class'])) {
      $content->css_class = check_plain($block->css['css_class']);
    }

    return $content;
  }

  /**
   * Render all prepared regions, placing already-rendered panes into their
   * appropriate positions therein.
   *
   * @return array
   *   An array of rendered panel regions, keyed on the region name.
   */
  function render_regions() {
    $this->rendered['regions'] = array();

    // Loop through all panel regions, put all panes that belong to the current
    // region in an array, then render the region. Primarily this ensures that
    // the blocks are arranged in the proper order.
    foreach ($this->prepared['regions'] as $region_id => $conf) {
      $region_blocks = array();
      foreach ($conf['uuids'] as $uuid) {
        // Only include blocks for region rendering if they had some output.
        if (!empty($this->rendered['blocks'][$uuid])) {
          $region_blocks[$uuid] = $this->rendered['blocks'][$uuid];
        }
      }
      $this->rendered['regions'][$region_id] = $this->render_region($region_id, $region_blocks);
    }

    return $this->rendered['regions'];
  }

  /**
   * Render a single panel region.
   *
   * Primarily just a passthrough to the panel region rendering callback
   * specified by the style plugin that is attached to the current panel region.
   *
   * @param $region_id
   *   The ID of the panel region being rendered
   * @param $blocks
   *   An array of blocks that are assigned to the region that's being rendered.
   *
   * @return string
   *   The rendered, HTML string output of the passed-in region.
   */
  function render_region($region_id, $blocks) {
    $style = $this->prepared['regions'][$region_id]['style'];
    $style_settings = $this->prepared['regions'][$region_id]['style settings'];

    if ($style['region theme']) {
      $output = theme($style['region theme'], array('layout' => $this->layout, 'blocks' => $blocks, 'settings' => $style_settings, 'style' => $style));
    }
    else {
      $output = implode('', $blocks);
    }

    return $output;
  }
}

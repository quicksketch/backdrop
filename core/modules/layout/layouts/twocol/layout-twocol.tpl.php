<?php
/**
 * @file
 * Template for a 2 column layout.
 *
 * This template provides a two column layout, with each column roughly equal in
 * width.
 *
 * Variables:
 * - $attributes: A string of attributes to be added to the layout wrapper.
 * - $content: An array of content, each item in the array is keyed to one
 *   region of the layout. This layout supports the following sections:
 *   - $content['left']: Content in the left column.
 *   - $content['right']: Content in the right column.
 */
?>
<div class="layout-twocol <?php print $classes; ?>"<?php print $attributes; ?>>
  <div class="layout-region layout-col layout-col-first">
    <div class="inside"><?php print $content['left']; ?></div>
  </div>
  <div class="layout-region layout-col layout-col-last">
    <div class="inside"><?php print $content['right']; ?></div>
  </div>
</div>

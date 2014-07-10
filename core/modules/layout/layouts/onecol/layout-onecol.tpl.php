<?php
/**
 * @file
 * Template for a single column panel layout.
 *
 * This template provides a very simple "one column" panel display layout.
 *
 * Variables:
 * - $attributes: A string of attributes to be added to the layout wrapper.
 * - $content: An array of content, each item in the array is keyed to one
 *   panel of the layout. This layout supports the following sections:
 *   $content['middle']: The only panel in the layout.
 */
?>
<div class="layout-onecol <?php print $classes; ?>"<?php print $attributes; ?>>
  <?php print $content['middle']; ?>
</div>

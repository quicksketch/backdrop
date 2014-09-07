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
  <div id="skip-link">
    <a href="#main-content" class="element-invisible element-focusable"><?php print t('Skip to main content'); ?></a>
  </div>

  <?php if ($content['header']): ?>
    <header id="header" role="banner" aria-label="<?php print t('Site header'); ?>"><div class="section clearfix">
      <?php print $content['header']; ?>
    </div></header>
  <?php endif; ?>

  <main id="content" class="column" role="main"><div class="section">
    <a id="main-content"></a>
    <?php print render($title_prefix); ?>
    <?php if ($title): ?>
      <h1 class="title" id="page-title">
        <?php print $title; ?>
      </h1>
    <?php endif; ?>
    <?php print render($title_suffix); ?>
    <?php if ($tabs): ?>
      <div class="tabs">
        <?php print $tabs; ?>
      </div>
    <?php endif; ?>

    <div id="page">
      <?php if ($messages): ?>
        <div id="messages"><div class="section clearfix">
          <?php print $messages; ?>
        </div></div> <!-- /.section, /#messages -->
      <?php endif; ?>

      <?php print $action_links; ?>
      <?php print $content['content']; ?>
    </div>
  </div></main> <!-- /.section, /#content -->

  <?php if ($content['footer']): ?>
    <div id="footer" class="clearfix"><div class="section">
      <?php print $content['footer']; ?>
    </div></div><!-- /.section, /#footer -->
  <?php endif; ?>
</div>

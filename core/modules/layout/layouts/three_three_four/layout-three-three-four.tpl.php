<?php
/**
 * @file
 * Template for a complex 3-3-4 column layout.
 *
 * This template mimicks the display of the legacy "Bartik" layout, which
 * includes responsive and collapsible columns.
 *
 * Variables:
 * - $attributes: A string of attributes to be added to the layout wrapper.
 * - $content: An array of content, each item in the array is keyed to one
 *   region of the layout. This layout supports the following sections:
 *   - $content['left']: Content in the left column.
 *   - $content['right']: Content in the right column.
 */
?>
<div class="layout-three-three-four <?php print $classes; ?>"<?php print $attributes; ?>>
  <?php if ($content['header']): ?>
  <header id="header" role="banner" aria-label="<?php print t('Site header'); ?>"><div class="section clearfix">
    <?php print $content['header']; ?>
  </div></header>
  <?php endif; ?>

  <?php if ($messages): ?>
  <div id="messages"><div class="section clearfix">
    <?php print $messages; ?>
  </div></div> <!-- /.section, /#messages -->
  <?php endif; ?>

  <?php if ($content['featured']): ?>
  <div id="featured"><div class="section clearfix">
    <?php print $content['featured']; ?>
  </div></div> <!-- /.section, /#featured -->
  <?php endif; ?>

  <div id="main-wrapper" class="clearfix"><div id="main" class="clearfix">
    <main id="content" class="column" role="main"><div class="section">

      <?php print $breadcrumb; ?>

      <?php if ($content['highlighted']): ?><div id="highlighted"><?php print $content['highlighted']; ?></div><?php endif; ?>

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
      <?php print $content['content']; ?>
    </div></main> <!-- /.section, /#content -->

    <?php if ($content['sidebar_first']): ?>
    <div id="sidebar-first" class="column sidebar"><div class="section">
      <?php print $content['sidebar_first']; ?>
    </div></div> <!-- /.section, /#sidebar-first -->
    <?php endif; ?>

    <?php if ($content['sidebar_second']): ?>
    <div id="sidebar-second" class="column sidebar"><div class="section">
      <?php print $content['sidebar_second']; ?>
    </div></div> <!-- /.section, /#sidebar-second -->
    <?php endif; ?>

  </div></div><!-- /#main, /#main-wrapper -->

  <?php if ($content['triptych_first'] || $content['triptych_middle'] || $content['triptych_last']): ?>
    <div id="triptych-wrapper"><div id="triptych" class="clearfix">
      <?php print $content['triptych_first']; ?>
      <?php print $content['triptych_middle']; ?>
      <?php print $content['triptych_last']; ?>
    </div></div> <!-- /#triptych, /#triptych-wrapper -->
  <?php endif; ?>

  <div id="footer-wrapper"><div class="section">
    <?php if ($content['footer_firstcolumn'] || $content['footer_secondcolumn'] || $content['footer_thirdcolumn'] || $content['footer_fourthcolumn']): ?>
      <div id="footer-columns" class="clearfix">
        <?php print $content['footer_firstcolumn']; ?>
        <?php print $content['footer_secondcolumn']; ?>
        <?php print $content['footer_thirdcolumn']; ?>
        <?php print $content['footer_fourthcolumn']; ?>
      </div> <!-- /#footer-columns -->
    <?php endif; ?>

    <?php if ($content['footer']): ?>
      <div id="footer" class="clearfix">
        <?php print $content['footer']; ?>
      </div> <!-- /#footer -->
    <?php endif; ?>
  </div></div> <!-- /.section, /#footer-wrapper -->
</div>

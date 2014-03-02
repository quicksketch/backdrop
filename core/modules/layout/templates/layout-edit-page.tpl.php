<?php
/**
 * @file
 * Template for the layout editor.
 *
 * Variables available:
 * -
 *
 * For javascript purposes the id must not change.
 */
?>
<div id="layout-edit">
  <?php print $locked; ?>
  <div class="layout-wrapper">
    <?php if (isset($operations['primary'])): ?>
      <div class="primary-actions clearfix actions">
        <?php print $operations['primary']; ?>
      </div>
    <?php endif; ?>
    <div class="layout-tabs clearfix">
      <div class="layout-edit-operations">
        <div class="inside">
          <?php print $operations['nav']; ?>
        </div>
      </div>
      <div class="layout-ajax-pad">
        <div class="inside">
          <div class="content-header">
            <div class="content-title">
              <?php print $changed; ?>
              <?php print $content['title']; ?>
            </div>
            <?php if (isset($operations['secondary'])): ?>
              <div class="secondary-actions clearfix actions">
                <?php print $operations['secondary']; ?>
              </div>
            <?php endif; ?>
          </div>

          <div class="content-content">
            <?php if (!empty($content['description'])): ?>
              <div class="description">
                <?php print $content['description']; ?>
              </div>
              <?php endif; ?>
            <?php print $content['content']; ?>
          </div>
        </div>
      </div>
    </div>
  </div>
  <?php print $save; ?>
</div>
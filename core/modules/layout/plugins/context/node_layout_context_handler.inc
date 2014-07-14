<?php
/**
 * @file
 * Layout context handler for node content.
 */
class NodeLayoutContext extends LayoutContext {
  function type() {
    return 'node';
  }

  function label() {
    if (isset($this->settings['nid'])) {
      if ($node = node_load($this->settings['nid'])) {
        return t('Node: @nid (@title)', array('@nid' => $node->nid, '@title' => $node->title));
      }
      else {
        return t('Unknown node: @nid', array('@nid' =>$this->settings['nid']));
      }
    }
    elseif ($this->locked) {
      return t('Node from path');
    }
    else {
      return t('Node');
    }
  }
}

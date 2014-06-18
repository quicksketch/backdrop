<?php
/**
 * @file
 * Layout context handler for node content.
 */
class NodeLayoutContext extends LayoutContext {
  function type() {
    return 'node';
  }

  function humanLabel() {
    if (isset($this->options['nid'])) {
      if ($node = node_load($this->options['nid'])) {
        return t('Node: @nid (@title)', array('@nid' => $node->nid, '@title' => $node->title));
      }
      else {
        return t('Unknown node: @nid', array('@nid' =>$this->options['nid']));
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

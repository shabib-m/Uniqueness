<?php

/**
 * @file
 * Settings page for the uniqueness module.
 */

/**
 * Form builder for uniqueness settings page; system_settings_form().
 */
function uniqueness_settings($form, &$form_state) {
  uniqueness_module_status_check();

  $form = array();
  $form['#config'] = 'uniqueness.settings';
  $config = config('uniqueness.settings');

  // Search options.
  $form['search'] = array(
    '#type' => 'fieldset',
    '#title' => t('Search'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['search']['uniqueness_search_mode'] = array(
    '#type' => 'radios',
    '#title' => t('Search mode'),
    '#description' => t('Select the mode which should be used for generating the list of related nodes.'),
    '#options' => _uniqueness_search_mode_options(),
    '#default_value' => $config->get('uniqueness_search_mode'),
    '#required' => TRUE,
  );
  $form['search']['uniqueness_scope'] = array(
    '#type' => 'radios',
    '#title' => t('Search scope'),
    '#options' => array(
      UNIQUENESS_SCOPE_ALL => t('Search in all nodes'),
      UNIQUENESS_SCOPE_CONTENT_TYPE => t('Search only within the content type of the node being added or edited.'),
    ),
    '#default_value' => $config->get('uniqueness_scope'),
    '#description' => t('Search all nodes or just nodes of the same content type.'),
    '#required' => TRUE,
  );
  $form['search']['uniqueness_results_max'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum number of results'),
    '#default_value' => $config->get('uniqueness_results_max'),
    '#size' => 5,
    '#maxlength' => 4,
    '#element_validate' => array('uniqueness_form_validate_results_max'),
    '#description' => t('Limit the number of search results. (For "Core search", must be 10 or fewer.)'),
    '#required' => TRUE,
  );
  $form['search']['uniqueness_query_min'] = array(
    '#type' => 'textfield',
    '#title' => t('Minimum length of search string'),
    '#default_value' => $config->get('uniqueness_query_min'),
    '#size' => 5,
    '#maxlength' => 4,
    '#element_validate' => array('uniqueness_form_validate_query_min'),
    '#description' => t('Enter the minimum number of characters required in the node title for triggering a search. (For "Core search", must be @min or more.)', array('@min' => 3)),
    '#required' => TRUE,
  );

  // Appearance.
  $form['appearance'] = array(
    '#type' => 'fieldset',
    '#title' => t('Appearance'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['appearance']['uniqueness_widgets'] = array(
    '#type' => 'checkboxes',
    '#options' => array(
      UNIQUENESS_WIDGET_INLINE => t('Display related content embedded on the create or edit content form.'),
      UNIQUENESS_WIDGET_BLOCK => t('Provide a block for displaying related content.'),
    ),
    '#default_value' => $config->get('uniqueness_widgets'),
  );
  $form['appearance']['uniqueness_default_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Default title'),
    '#default_value' => $config->get('uniqueness_default_title'),
    '#description' => t('Note: When displayed in a block, this title may be overridden by configuring the Uniqueness block in the appropriate layout on the <a href="@layouts-url">Layouts administration</a> page.',
      array('@layouts-url' => url('admin/structure/layouts'))),
    '#required' => TRUE,
  );
  $form['appearance']['uniqueness_default_description'] = array(
    '#type' => 'textarea',
    '#title' => t('Default description'),
    '#default_value' => $config->get('uniqueness_default_description'),
    '#rows' => 2,
  );
  $form['appearance']['uniqueness_searching_string'] = array(
    '#type' => 'textfield',
    '#title' => t('Search notifier'),
    '#default_value' => $config->get('uniqueness_searching_string'),
    '#description' => t('The text to display while the uniqueness search is in progress.'),
  );
  $form['appearance']['uniqueness_no_result_string'] = array(
    '#type' => 'textfield',
    '#title' => t('No related content message'),
    '#default_value' => $config->get('uniqueness_no_result_string'),
    '#description' => t('The text to display if the uniqueness search no longer finds any related content.'),
  );
  $form['appearance']['uniqueness_results_prepend'] = array(
    '#type' => 'radios',
    '#title' => t('Results display'),
    '#options' => array(
      0 => t('Replace old results with new ones'),
      1 => t('Leave old results and prepend new ones'),
    ),
    '#default_value' => $config->get('uniqueness_results_prepend'),
    '#description' => t('Choose if new results replace or are added to existing results. Browser cache may keep this setting from taking affect right away.'),
    '#required' => TRUE,
  );
  return system_settings_form($form);
}

function _uniqueness_search_mode_options() {
  $options = array(
    UNIQUENESS_SEARCH_MODE_NODETITLE => t('Simple node title search'),
  );
  if (module_exists('search')) {
    $options += array(
      UNIQUENESS_SEARCH_MODE_CORE => t('Core search'),
    );
  }
  if (module_exists('apachesolr_search')) {
    $options += array(
      UNIQUENESS_SEARCH_MODE_SOLR => t('Apache Solr search'),
    );
  }
  return $options;
}

/**
 * Implements hook_form_validate().
 */
function uniqueness_form_validate_results_max($element, &$form_state) {
  // Validate that the number of results to show is numeric and within range
  $upper_limit = ($form_state['values']['uniqueness_search_mode'] == UNIQUENESS_SEARCH_MODE_CORE) ? 10 : 100;
  $value = $element['#value'];
  if (!is_numeric($value) || $value <= 0 || $value > $upper_limit) {
    form_error($element, t('The number of results must be between 1 and @upper.', array('@upper' => $upper_limit)));
  }
}

/**
 * Implements hook_form_validate().
 */
function uniqueness_form_validate_query_min($element, &$form_state) {
  // Validate that the minimum number of search characters is numeric and within range
  $lower_limit = ($form_state['values']['uniqueness_search_mode'] == UNIQUENESS_SEARCH_MODE_CORE) ? 3 : 1;
  $value = $element['#value'];
  if (!is_numeric($value) || $value < $lower_limit || $value > 30) {
    form_error($element, t('The number of search characters must be between @lower and 30.', array('@lower' => $lower_limit)));
  }
}

/**
 * Helper function which shows a message if uniqueness search is not activated
 * on any content type.
 */
function uniqueness_module_status_check() {
  foreach (array_keys(node_type_get_types()) as $type) {
    $v = uniqueness_get_type_settings($type);
    if (!empty($v)) {
     return;
    }
  }
  backdrop_set_message(t('Uniqueness search has not been enabled for any content types. To enable it, visit the configuration page for each desired <a href="@content-types-page">content type</a>.', array('@content-types-page' => url('admin/structure/types'))));
}

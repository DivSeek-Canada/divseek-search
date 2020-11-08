<?php

/**
 * @file
 * Includes definition of custom form elements.
 */

/**
 * Implements hook_element_info().
 */
function divseek_search_element_info() {
	$elements = [];

	// Phenotypic value filter.
	$elements['phenotypic_value'] = [
		'#input' => TRUE,
		'#process' => ['phenotypic_value_process'],
		'#element_validate' => ['phenotypic_value_validate'],
		'#theme_wrappers' => ['divseek_search_inline_form_element'],
	];

	return $elements;
}

/**
 * 'phenotypic_value' Process Callback.
 */
function phenotypic_value_process($element, &$form_state, $complete_form) {

	// @debug dpm($element, 'element');
	$element['#tree'] = TRUE;
	$element['#prefix'] = '<div id="phenotypic-value-field-wrapper"';
	$element['#suffix'] = '</div>';

	// Defaults.
	if (!isset($element['#default_value'])) {
		$element['#default_value'] = [
			'trait' => NULL,
			'operator' => '>=',
			'value' => NULL,
			'unit' => NULL,
		];
	}
	else {
		if (!isset($element['#default_value']['trait'])) {
			$element['#default_value']['trait'] = NULL;
		}
		if (!isset($element['#default_value']['operator'])) {
			$element['#default_value']['operator'] = '>=';
		}
		if (!isset($element['#default_value']['value'])) {
			$element['#default_value']['value'] = NULL;
		}
		if (!isset($element['#default_value']['unit'])) {
			$element['#default_value']['unit'] = NULL;
		}
	}

	// Determine what should be disabled.
	if (!isset($element['#disabled'])) {
		$element['#disabled'] = FALSE;
	}
	if (!isset($element['#genus']) OR $element['#genus'] == NULL) {
		$element['#disabled'] = TRUE;
	}
	$unit_disabled = TRUE;
	if (!empty($element['#default_value']['trait'])) {
		$unit_disabled = FALSE;
	}

	//  - Controlled Vocabulary (cv). Used in the cv_name in cvterm property array below.
	$traits = [];
	if ($element['#genus']) {
		$sysvar = ap_get_variablenames(
			array('variablename' => $element['#genus']),
			array('set' => 'cvdbon', 'suffix' => 'allsuffix')
		);
	  $sysvar_genus_cv = variable_get($sysvar['cv']);
	  $cvprop = ap_get_cv(array(
	    'cv_id' => $sysvar_genus_cv
	  ));
	  $trait_cv_name = $cvprop['name'];
		$traits = chado_query('SELECT cvterm_id, name FROM {cvterm}
			WHERE cv_id IN (SELECT cv_id FROM {cv} WHERE name = :cv)',
			[':cv' => $trait_cv_name])->fetchAllKeyed();
	}
	$element['trait'] = [
		'#type' => 'select',
		'#options' => $traits,
		'#empty_option' => '- Trait -',
		'#disabled' => $element['#disabled'],
		'#default_value' => $element['#default_value']['trait'],
		'#ajax' => [
			'wrapper' => 'phenotypic-value-field-wrapper',
			'callback' => 'ajax_phenotype_value_field_refresh',
			'element' => $element['#name'],
		],
	];

	$element['is'] = [
		'#markup' => '<div class="form-item form-item-phenotypic-value-is">is</div>',
	];

	$element['operator'] = [
		'#type' => 'select',
		'#options' => [
			'>' =>'>',
			'>=' => '≥',
			'<=' => '≤',
			'<' => '<',
			'=' => '=',
			'<>' => '≠'
		],
		'#default_value' => $element['#default_value']['operator'],
	];

	$element['value'] = [
		'#type' => 'textfield',
		'#default_value' => $element['#default_value']['value'],
	];

	$units = [];
	if (!$unit_disabled) {
		$methods = ap_get_trait_methods(
			$element['#genus'],
			NULL,
			[],
			$element['#default_value']['trait']
		);
		foreach ($methods as $method_id => $method) {
			$units4thismethod = ap_get_method_units(
				$element['#genus'],
				$method,
				[],
				$method_id
			);
			$units = $units + $units4thismethod;
		}
	}
	$element['unit'] = [
		'#type' => 'select',
		'#options' => $units,
		'#empty_option' => '- Unit -',
		'#disabled' => $unit_disabled,
		'#default_value' => $element['#default_value']['unit'],
	];

	return $element;
}

/**
 * 'phenotypic_value' Validation Callback.
 */
function phenotypic_value_validate($element, &$form_state) {

	// All elements must be provided on submit.
	if ($form_state['triggering_element']['#type'] == 'submit') {

		$name = $element['#name'];
		if (!empty($form_state['values'][$name]['trait'])) {
			if (empty($form_state['values'][$name]['value'])) {
				form_set_error("$name][value",
					'The "value" is required to filter by phenotypic value.');
			}
			if (empty($form_state['values'][$name]['unit'])) {
				form_set_error("$name][unit",
					'The "unit" is required to filter by phenotypic value.');
			}
		}
	}
}

/**
 * Inline-form element wrappers.
 */
function theme_divseek_search_inline_form_element($variables) {
   $element = $variables['element'];

   // Add element #id for #type 'item'.
   if (isset($element['#markup']) && !empty($element['#id'])) {
     $attributes['id'] = $element['#id'];
   }

   // Add element's #type and #name as class to aid with JS/CSS selectors.
   $attributes['class'] = array(
     'form-item',
   );
   if (!empty($element['#type'])) {
     $attributes['class'][] = 'form-type-' . strtr($element['#type'], '_', '-');
   }
   if (!empty($element['#name'])) {
     $attributes['class'][] = 'form-item-' . strtr($element['#name'], array(
       ' ' => '-',
       '_' => '-',
       '[' => '-',
       ']' => '',
     ));
   }

   // Add a class for disabled elements to facilitate cross-browser styling.
   if (!empty($element['#attributes']['disabled'])) {
     $attributes['class'][] = 'form-disabled';
   }
   $output = '<div' . drupal_attributes($attributes) . '>' . "\n";

   // If #title is not set, we don't display any label or required marker.
   if (!isset($element['#title'])) {
     $element['#title_display'] = 'none';
   }
   $prefix = isset($element['#field_prefix']) ? '<span class="field-prefix">' . $element['#field_prefix'] . '</span> ' : '';
   $suffix = isset($element['#field_suffix']) ? ' <span class="field-suffix">' . $element['#field_suffix'] . '</span>' : '';
   switch ($element['#title_display']) {
     case 'before':
       $output .= ' ' . theme('form_element_label', $variables);
       $output .= ' ' . '<div class="container-inline">' . $prefix . $element['#children'] . $suffix . "</div>\n";
       break;
     case 'invisible':
     case 'after':
       $output .= ' ' . $prefix . $element['#children'] . $suffix;
       $output .= ' ' . theme('form_element_label', $variables) . "\n";
       break;
     case 'none':
     case 'attribute':

       // Output no label and no required marker, only the children.
       $output .= ' ' . $prefix . $element['#children'] . $suffix . "\n";
       break;
   }
   if (!empty($element['#description'])) {
     $output .= ' <div class="description">' . $element['#description'] . "</div>\n";
   }
   $output .= "</div>\n";
   return $output;
}

/**
 * AJAX: Refresh the phenotypic value form field.
 */
function ajax_phenotype_value_field_refresh($form, $form_state) {
	$name = $form_state['triggering_element']['#ajax']['element'];
	return $form[$name];
}

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

  // Phenotypic value filter.
  $elements['multi_phenotypic_value'] = [
    '#input' => TRUE,
    '#process' => ['multi_phenotypic_value_process'],
    '#element_validate' => ['multi_phenotypic_value_validate'],
    '#theme_wrappers' => ['divseek_search_inline_form_element'],
  ];

	return $elements;
}

/////////////////// Single Phenotypic Value ////////////////////////////////////

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
      'scope' => 'siteyear',
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
		if (!isset($element['#default_value']['scope'])) {
			$element['#default_value']['scope'] = 'siteyear';
		}
	}
  $ajax_element = $element['#name'];
  $ajax_wrapper = 'phenotypic-value-field-wrapper';
  if (isset($element['#ajax_element'])) {
    $ajax_element = $element['#ajax_element'];
  }
  if (isset($element['#ajax_wrapper'])) {
    $ajax_wrapper = $element['#ajax_wrapper'];
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
			'wrapper' => $ajax_wrapper,
			'callback' => 'ajax_phenotype_value_field_refresh',
			'element' => $ajax_element
		],
    '#prefix' => '<div class="phenotypic-statement">',
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

	$element['scope'] = [
		'#type' => 'select',
		'#options' => [
			'measurement' => 'Any Measurement',
			'siteyear' => 'Mean per Site Year',
			'experiment' => 'Mean per Experiment',
			'trait' => 'Mean per Trait',
		],
		'#default_value' => $element['#default_value']['scope'],
    '#suffix' => '</div>'
	];

  $trait_id = ($element['#default_value']['trait']) ? (int) $element['#default_value']['trait'] : NULL;
  $units = chado_query("SELECT unit_id, unit_name FROM {mview_phenotype}
    WHERE trait_id=:trait_id GROUP BY unit_id, unit_name",
    [':trait_id' => $trait_id])->fetchAllKeyed(0,1);
  drupal_add_js("https://cdnjs.cloudflare.com/ajax/libs/d3/4.13.0/d3.min.js",
    [
      'type' => 'external',
      'scope' => 'header',
      'weight' => -20,
    ]);
  drupal_add_js("https://d3js.org/d3-scale-chromatic.v1.min.js",
    [
      'type' => 'external',
      'scope' => 'header',
      'weight' => -19,
    ]);
  drupal_add_js(['divseeksearch' => [
      'dataurl' => url('data/divseeksearch/trait/heatmap'),
      'data' => [
        [
          'trait_id' => $trait_id,
          'units' => (object) $units,
          'unit_ids' => array_keys($units),
        ],
      ],
    ]], 'setting');
  drupal_add_js(drupal_get_path("module","divseek_search"). '/js/phenotypicvalues-heatmap.js',
    [
      'type' => 'file',
      'scope' => 'header',
      'weight' => -18,
    ]);
  $element['chart'] = [
    '#type' => 'markup',
    '#prefix' => '<div id="phenotypicvalues-heatmap">',
    '#suffix' => '</div>',
  ];
  foreach ($units as $unit_id => $unit_name) {
    $id = 'phenotypicvalues-heatmap-' . $trait_id . '-' . $unit_id;
    $tooltip = 'Unit: ' . $unit_name . "; Indicates the magnitude of germplasm with a given mean value per trait/unit where larger numbers of germplasm are shown with a darker colour.";
    $element['chart'][$id] = [
      '#type' => 'markup',
      '#markup' => '<div id="'.$id.'" class="phenotypicvalues-heatmap-item"></div>',
    ];
  }

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

/////////////////// Multi-Phenotypic Value ////////////////////////////////////

/**
 * 'multi_phenotypic_value' Process Callback.
 */
function multi_phenotypic_value_process($element, &$form_state, $complete_form) {


  // We will have many fields with the same name, so we need to be able to
  // access the form hierarchically.
  $element['#tree'] = TRUE;
  $element['#prefix'] = '<div id="multi-phenotypicvalue-field-wrapper">';
  $element['#suffix'] = '</div>';

  // Defaults.
  if (empty($form_state['num_traits'])) {
    if (isset($element['#num_traits'])) {
      $form_state['num_traits'] = $element['#num_traits'];
    }
    else {
      $form_state['num_traits'] = 1;
    }
  }
  if (empty($element['#genus'])) {
    $element['#genus'] = NULL;
  }
  if (empty($element['#default_value'])) {
    $element['#default_value'] = [];
  }
  for ($i = 1; $i <= $form_state['num_traits']; $i++) {
    if (!isset($element['#default_value'][$i])) {
      $element['#default_value'][$i] = [
        'trait' => NULL,
  			'operator' => '>=',
  			'value' => NULL,
  			'unit' => NULL,
        'scope' => 'siteyear',
      ];
    }
  }


  // Build the number of name fieldsets indicated by $form_state['num_names']
  for ($i = 1; $i <= $form_state['num_traits']; $i++) {
    $element[$i] = [
      '#type' => 'phenotypic_value',
      '#genus' => $element['#genus'],
      '#default_value' => $element['#default_value'][$i],
      '#ajax_element' => $element['#name'],
      '#ajax_wrapper' => 'multi-phenotypicvalue-field-wrapper',
      '#prefix' => '<div id="multi-phenotypicvalue-'.$i.'">',
      '#suffix' => '</div>',
    ];
  }

  if ($element['#genus'] !== NULL) {
    // Adds "Add another name" button.
    $element['add_trait'] = array(
      '#type' => 'submit',
      '#value' => t('Add Another Trait'),
      '#submit' => array('multi_phenotypic_value_process_add_trait'),
    );
  }

  return $element;
}

/**
 * Submit handler: "Add Another Trait" in multi_phenotypic_value_process().
 *
 * $form_state['num_traits'] tells the form builder function how many trait
 * fieldsets to build, so here we increment it.
 *
 * All elements of $form_state are persisted, so there's no need to use a
 * particular key, like the old $form_state['storage']. We can just use
 * $form_state['num_traits'].
 */
function multi_phenotypic_value_process_add_trait($form, &$form_state) {

  $form_state['num_traits']++;

  // Setting $form_state['rebuild'] = TRUE causes the form to be rebuilt again.
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler: "Remove Last Trait" in multi_phenotypic_value_process().
 */
function multi_phenotypic_value_process_remove_trait($form, &$form_state) {
  if ($form_state['num_traits'] > 1) {
    $form_state['num_traits']--;
  }

  // Setting $form_state['rebuild'] = TRUE causes the form to be rebuilt again.
  $form_state['rebuild'] = TRUE;
}


/**
 * 'multi_phenotypic_value' Validate Callback.
 */
function multi_phenotypic_value_validate($element, &$form_state) {

}

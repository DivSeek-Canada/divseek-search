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
  drupal_add_js("https://d3js.org/d3.v4.js", "external");
  drupal_add_js("https://d3js.org/d3-scale-chromatic.v1.min.js", "external");
  drupal_add_js(['divseeksearch' => [
      'dataurl' => url('data/divseeksearch/trait/heatmap'),
      'trait_id' => $trait_id,
      'units' => (object) $units,
      'unit_ids' => array_keys($units),
    ]], 'setting');
  drupal_add_js(drupal_get_path("module","divseek_search"). '/js/phenotypicvalues-heatmap.js');
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
      '#markup' => '<div id="'.$id.'"></div>',
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

/**
 * Data-driven heatmap.
 */
function divseek_search_get_heatmap_data($trait_id, $unit_id) {
  $data = [];

  // First check if quantitative or qualitative.
  $quantitative = chado_query("SELECT true FROM {mview_phenotype_average}
    WHERE trait_id=:trait_id AND unit_id=:unit_id AND qual_values IS NULL",
    [':trait_id' => $trait_id, 'unit_id' => $unit_id])->fetchField();

  // If Quantitative, then determine the range.
  if ($quantitative) {
    $range = chado_query("
      SELECT min(mean_value) as min, max(mean_value) as max
      FROM {mview_phenotype_average}
      WHERE trait_id=:trait_id AND unit_id=:unit_id",
      [':trait_id' => $trait_id, ':unit_id' => $unit_id])->fetchObject();

    $num_bins = 10;
    $diff = $range->max - $range->min;
    $decimals = 0;
    if ($diff <= $num_bins) {
      $decimals = 2;
    }
    $start = round($range->min, $decimals, PHP_ROUND_HALF_DOWN);
    $max = round($range->max, $decimals, PHP_ROUND_HALF_UP);
    $step = round(($max - $start) / $num_bins, $decimals, PHP_ROUND_HALF_UP);
    $end = $start + ($step * $num_bins);

    $sql = "SELECT count(*) FROM {mview_phenotype_average}
      WHERE trait_id=:trait_id AND unit_id=:unit_id AND scope = 'trait'
      AND mean_value BETWEEN :start AND :end";
    for($i=0; $i<$num_bins; $i++) {
      $i_start = ($start + ($i*$step));
      $i_end = $i_start + $step;
      $num_germ = chado_query($sql, [
          ':trait_id' => $trait_id,
          ':unit_id' => $unit_id,
          ':start' => $i_start,
          ':end' => $i_end,
        ])->fetchField();

      $data[] = [
        'group' => $i_start . '-' . $i_end,
        'variable' => $unit_id,
        'value' => $num_germ,
      ];
    }
  }
  // Otherwise it's qualitative.
  else {
    $sql = "SELECT qual_values, count(*) as num FROM {mview_phenotype_average}
      WHERE trait_id=:trait_id AND scope='trait'
      GROUP BY qual_values ORDER BY qual_values";
    $resource = chado_query($sql, [':trait_id' => $trait_id]);
    while ($r = $resource->fetchObject()) {
      $data[] = [
        'group' => $r->qual_values,
        'variable' => $unit_id,
        'value' => $r->num,
      ];
    }
  }

  return drupal_json_output($data);
}

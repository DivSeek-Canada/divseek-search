<?php

/**
 * Provides a search for breeders to discover markers associated with a
 * trait of interest through existing QTL.
 *
 */
class TraitQtlMarkerSearch extends ChadoCustomSearch {

  /**
   * The human readable title of this search. This will be used in listings
   * and shown on the search page as the title.
   */
  public static $title = 'Trait QTL Marker Discovery Tool';

  /**
   * The machine name of the module providing this search.
   */
  public static $module = 'divseek_search';

  /**
   * A description of this search. This is shown at the top of the search page
   * and used for the menu item.
   */
  public static $description = '';

  /**
   * The machine names of the permissions with access to this search. This is
   * used to map your search to existing permissions. It must be an array and
   * is used in hook_menu(). Some examples include 'access content'
   * and 'administer tripal'.
   */
  public static $permissions = ['access content'];

  /**
   * If TRUE, this search will require the submit button to be clicked before
   * executing the query; whereas, if FALSE it will be executed on the
   * first page load without parameters.
   *
   * NOTE: to control the results without parameters check $this->submitted
   * in getQuery() and if FALSE return your pre-submit query.
   */
  public static $require_submit = TRUE;

  /**
   * Add a pager to search results
   * with $num_items_per_page results on a single page.
   * NOTE: Your query has to handle paging.
   */
  public static $pager = TRUE;
  public static $num_items_per_page = 20;

  /**
   * This defined the hook_menu definition for this search. At a minimum the
   * path is required.
   */
  public static $menu = [
    'path' => 'search/trait-qtl-markers',
    // @todo support menu items.
  ];

  /**
   * Add CSS/JS to the form/results page.
   * NOTE: paths supplied should be relative to $module.
   */
  public static $attached = [
    'css' => [
      'css/all_searches.css',
      'css/trait-qtl-marker.css',
    ],
    'js' => [],
  ];

  /**
   * Information regarding the fields and filters for this search.
   */
  public static $info = [
    // Lists the columns in your results table.
    'fields' => [
      'marker' => [
        'title' => 'Marker Name',
        //'entity_link' => [
        //  'chado_table' => 'feature',
        //  'id_column' => 'feature_id'
        //],
      ],
      'lg' => [
        'title' => 'Linkage Group',
      ],
      'qtl' => [
        'title' => 'QTL'
      ],
    ],
    // The filter criteria available to the user.
    // This is used to generate a search form which can be altered.
    'filters' => [
      'genus' => [
        'title' => 'Genus',
        'help' => 'The crop species of the population the genetic map was created for (e.g. Lens).',
      ],
      'trait' => [
        'title' => 'Trait',
        'help' => 'The phenotypic trait you are interested in finding genetic markers for.',
      ],
      'genetic_maps' => [
        'title' => 'Genetic Maps',
        'help' => 'The name of the genetic maps you are interested in exploring.',
      ],
      'linkage_group' => [
        'title' => 'Linkage Group',
        'help' => 'The name of the linkage group to look for QTL and markers on.',
      ],
      'qtl' => [
        'title' => 'QTL',
        'help' => 'The QTL to look for markers within.',
      ],
      /*
      'germplasm' => [
        'title' => 'Germplasm',
        'help' => 'The germplasm you are interested in.',
      ],
      */
    ],
  ];

  /**
   * Text that should appear on the button at the bottom of the importer
   * form.
   */
  public static $button_text = 'Search';

  /**
   * Generate the filter form.
   *
   * The base class will generate textfields for each filter defined in $info,
   * set defaults, labels and descriptions, as well as, create the search
   * button.
   *
   * Extend this method to alter the filter form.
   */
  public function form($form, $form_state) {
    $form = parent::form($form, $form_state);

    // Retrieve the cvterm_ids we need for our query.
    $term_ids = $this->getTypeIDS();
    // @debug dpm($term_ids, 'Term IDS');

    // Retrieve the default values for the form
    // based on form_state and build_args.
    $values = $this->getDefaultValues($form_state);

    // Sort into fielsets.
    // -- Trait.
    $form['s1'] = [
      '#type' => 'fieldset',
      '#title' => 'Step 1: Trait',
      '#prefix' => '<div id="tqtlm-s1trait">',
      '#suffix' => '</div>',
    ];
    $form['s1']['genus'] = $form['genus'];
    $form['s1']['genus']['#weight'] = -10;
    $form['s1']['genus']['#prefix'] = '<div id="tqtlm-s1trait-flex">';
    $form['s1']['trait'] = $form['trait'];
    $form['s1']['trait']['#weight'] = -9;
    $form['s1']['trait']['#suffix'] = '</div>';
    $form['s1']['trait']['#disabled'] = TRUE;
    unset($form['genus'], $form['trait']);

    // -- QTL.
    $form['s2'] = [
      '#type' => 'fieldset',
      '#title' => 'Step 2: Quantitative Trait Loci',
      '#prefix' => '<div id="tqtlm-s2qtl">',
      '#suffix' => '</div>',
    ];
    $form['s2']['genetic_maps'] = $form['genetic_maps'];
    $form['s2']['genetic_maps']['#weight'] = -10;
    $form['s2']['genetic_maps']['#disabled'] = TRUE;
    $form['s2']['genetic_maps']['#prefix'] = '<div id="tqtlm-s2qtl-flex">';
    $form['s2']['linkage_group'] = $form['linkage_group'];
    $form['s2']['linkage_group']['#weight'] = -9;
    $form['s2']['linkage_group']['#disabled'] = TRUE;
    $form['s2']['qtl'] = $form['qtl'];
    $form['s2']['qtl']['#weight'] = -8;
    $form['s2']['qtl']['#suffix'] = '</div>';
    $form['s2']['qtl']['#disabled'] = TRUE;
    unset($form['genetic_maps'], $form['linkage_group'], $form['qtl']);

    // -- Germplasm.
    /*
    $form['s3'] = [
      '#type' => 'fieldset',
      '#title' => 'Step 3: Germplasm (Optional)',
    ];
    $form['s3']['germplasm'] = $form['germplasm'];
    unset($form['germplasm']);
    */

    // AJAX.
    // -- Select genus should refresh trait fieldset.
    $form['s1']['genus']['#ajax'] = [
      'callback' => 'ajax_tqtlm_refreshtrait_callback',
      'wrapper' => 'tqtlm-s1trait',
      'method' => 'replace',
      'effect' => 'fade',
    ];

    // -- Select trait should refresh genetic map & QTL fieldset.
    $form['s1']['trait']['#ajax'] = [
      'callback' => 'ajax_tqtlm_refreshqtl_callback',
      'wrapper' => 'tqtlm-s2qtl',
      'method' => 'replace',
      'effect' => 'fade',
    ];

    // -- Select genetic map/lg should refresh genetic map & QTL fieldset.
    $form['s2']['genetic_maps']['#ajax'] = [
      'callback' => 'ajax_tqtlm_refreshqtl_callback',
      'wrapper' => 'tqtlm-s2qtl',
      'method' => 'replace',
      'effect' => 'fade',
    ];
    $form['s2']['linkage_group']['#ajax'] = [
      'callback' => 'ajax_tqtlm_refreshqtl_callback',
      'wrapper' => 'tqtlm-s2qtl',
      'method' => 'replace',
      'effect' => 'fade',
    ];


    // Make into Drop-downs.
    // @debug dpm($values, 'values for form');
    // -- Genus.
    $options = chado_query(
      "SELECT organism_genus FROM {mview_phenotype_average}
       ORDER BY organism_genus ASC")->fetchAllKeyed(0,0);
    $form['s1']['genus']['#type'] = 'select';
    $form['s1']['genus']['#options'] = $options;
    $form['s1']['genus']['#empty_option'] = '- Select -';
    // -- Trait.
    $options = [];
    if (!empty($values['genus'])) {
      $form['s1']['trait']['#disabled'] = FALSE;
      $options = chado_query(
        "SELECT cvterm_id, name FROM {cvterm}
         WHERE cvterm_id IN (
           SELECT trait_id
           FROM {mview_phenotype_average}
           WHERE organism_genus=:genus)
         AND cvterm_id IN (SELECT cvterm_id FROM {feature_cvterm)
         ORDER BY name ASC",
         [':genus' => $values['genus']])->fetchAllKeyed(0,1);
    }
    $form['s1']['trait']['#type'] = 'select';
    $form['s1']['trait']['#options'] = $options;
    $form['s1']['trait']['#empty_option'] = '- Select -';
    // -- Genetic Map.
    $options = [];
    if (!empty($values['trait'])) {
      $form['s2']['genetic_maps']['#disabled'] = FALSE;
      $options = chado_query(
        "SELECT map_id, map_name FROM {tripal_map_qtl_and_mtl_mview}
         WHERE genus = :genus
         AND cast(feature_id as int) IN (
           SELECT feature_id FROM {feature_cvterm}
           WHERE cvterm_id = :trait_id
         )
         ORDER BY map_name ASC",
         [':trait_id' => $values['trait'], ':genus' => $values['genus']])
         ->fetchAllKeyed(0,1);
    }
    $form['s2']['genetic_maps']['#type'] = 'select';
    $form['s2']['genetic_maps']['#multiple'] = TRUE;
    $form['s2']['genetic_maps']['#options'] = $options;
    $form['s2']['genetic_maps']['#empty_option'] = '- Select -';
    // -- Linkage Group.
    $options = [];
    if (!empty($values['trait'])) {
      $form['s2']['linkage_group']['#disabled'] = FALSE;
      if (!empty($values['genetic_maps'])) {
        $options = chado_query(
          "SELECT linkage_group, linkage_group FROM {tripal_map_qtl_and_mtl_mview} qtl WHERE cast(qtl.feature_id as int) IN (SELECT feature_id FROM {feature_cvterm} WHERE cvterm_id=:trait_id) AND map_id IN (:map_id)
          ORDER BY linkage_group ASC",
          [':trait_id' => $values['trait'],
            ':map_id' => $values['genetic_maps']])->fetchAllKeyed(0,1);
      }
      else {
        $options = chado_query(
          "SELECT linkage_group, linkage_group FROM {tripal_map_qtl_and_mtl_mview} qtl
          WHERE cast(qtl.feature_id as int) IN (SELECT feature_id FROM {feature_cvterm} WHERE cvterm_id=:trait_id)",
          [':trait_id' => $values['trait']])->fetchAllKeyed(0,1);
      }
    }
    $form['s2']['linkage_group']['#type'] = 'select';
    $form['s2']['linkage_group']['#multiple'] = TRUE;
    $form['s2']['linkage_group']['#options'] = $options;
    $form['s2']['linkage_group']['#empty_option'] = '- Select -';
    // -- QTL.
    $options = [];
    if (!empty($values['trait'])) {
      $form['s2']['qtl']['#disabled'] = FALSE;

      $options_sql = "SELECT feature_id, marker_locus_name FROM {tripal_map_qtl_and_mtl_mview} qtl";
      $where = ["cast(qtl.feature_id as int) IN (SELECT feature_id FROM {feature_cvterm} WHERE cvterm_id=:trait_id)"];
      $options_args = [':trait_id' => $values['trait']];

      if (!empty($values['genetic_maps'])) {
        $where[] = "map_id IN (:map_id)";
        $options_args[':map_id'] = $values['genetic_maps'];
      }
      if (!empty($values['linkage_group'])) {
        $where[] = "linkage_group IN (:lg_id)";
        $options_args[':lg_id'] = $values['linkage_group'];
      }
      $options_sql .= " WHERE " . implode(' AND ', $where);
      $options = chado_query($options_sql, $options_args)->fetchAllKeyed(0,1);
    }
    $form['s2']['qtl']['#type'] = 'select';
    $form['s2']['qtl']['#multiple'] = TRUE;
    $form['s2']['qtl']['#options'] = $options;
    $form['s2']['qtl']['#empty_option'] = '- Select -';

    return $form;
  }

  /**
   * Allows custom searches to validate the form results.
   * Use form_set_error() to signal invalid values.
   */
  public function validateForm($form, $form_state) {
    //$values = $form_state['values'];
  }

  /**
   * Determine the query for the genetic marker search.
   *
   * @param string $query
   *   The full SQL query to execute. This will be executed using chado_query()
   *   so use curly brackets appropriately. Use :placeholders for any values.
   * @param array $args
   *   An array of arguments to pass to chado_query(). Keys must be the
   *   placeholders in the query and values should be what you want them set to.
   */
  public function getQuery(&$query, &$args, $offset) {

    // Retrieve the filter results already set.
    $filter_results = $this->values;
    // @debug dpm($filter_results, 'filter_results');

    $type_ids = $this->getTypeIDS();

    if ($filter_results['trait']) {
      $query = "SELECT
        marker.genetic_marker_name as marker,
        qtl.linkage_group as lg,
        min(qtlstart.value) as min_qtl_start,
        json_agg(json_build_object(
         'marker_pos', marker.marker_pos,
         'map_name', qtl.map_name,
         'qtl_name', qtl.marker_locus_name,
         'qtl_peak', qtl.marker_pos,
         'qtl_start', qtlstart.value,
         'qtl_end', qtlend.value
        )) as qtl
      FROM {tripal_map_qtl_and_mtl_mview} qtl
      LEFT JOIN {feature_cvterm} tlink
        ON tlink.feature_id = cast(qtl.feature_id as bigint)
      LEFT JOIN {cvterm} trait
        ON tlink.cvterm_id = trait.cvterm_id
      LEFT JOIN {featureposprop} qtlstart
        ON qtlstart.featurepos_id = cast(qtl.featurepos_id as bigint)
        AND qtlstart.type_id = :start_type
      LEFT JOIN {featureposprop} qtlend
        ON qtlend.featurepos_id = cast(qtl.featurepos_id as bigint)
        AND qtlend.type_id = :end_type
      LEFT JOIN {tripal_map_genetic_markers_mview} marker
        ON marker.map_id = qtl.map_id
        AND marker.linkage_group_id = qtl.linkage_group_id
      WHERE
        trait.cvterm_id = :trait_id
        AND qtl.marker_pos_type = 'qtl_peak'
        AND cast(marker.marker_pos as double precision)
          BETWEEN cast(qtlstart.value as double precision)
          AND cast(qtlend.value as double precision)";
      $args[':trait_id'] = $filter_results['trait'];
      $args[':start_type'] = $type_ids['start'];
      $args[':end_type'] = $type_ids['end'];

      $where = [];
      $joins = [];

      // -- Genetic Maps.
      if (!empty($filter_results['genetic_maps'])) {
        $where[] = "qtl.map_id IN (:map_id)";
        $args[':map_id'] = $filter_results['genetic_maps'];
      }
      // -- Linkage Group.
      if (!empty($filter_results['linkage_group'])) {
        $where[] = "qtl.linkage_group IN (:lgid)";
        $args[':lgid'] = $filter_results['linkage_group'];
      }
      // -- QTL.
      if (!empty($filter_results['qtl'])) {
        $where[] = "qtl.feature_id IN (:qtl_id)";
        $args[':qtl_id'] = $filter_results['qtl'];
      }

      // Finally, add it to the query.
      if (!empty($joins)) {
        $query .= "\n" . implode("\n",$joins);
      }
      if (!empty($where)) {
        $query .= "\n" . ' AND ' . implode(' AND ',$where);
      }

      // Group into markers.
      $query .= "\n" . ' GROUP BY marker.genetic_marker_name, qtl.linkage_group';

      // Sort even though it is SLOW b/c ppl expect it.
      $query .= "\n" . ' ORDER BY qtl.linkage_group ASC, min_qtl_start ASC';

      // Handle paging.
      $limit = $this::$num_items_per_page + 1;
      $query .= "\n" . ' LIMIT ' . $limit . ' OFFSET ' . $offset;
    }
    // @debug dpm(strtr(str_replace(['{','}'], ['chado.', ''], $query), $args), 'query');
  }

  /**
   * Format the results within the $form array.
   *
   * The base class will format the results as a table.
   *
   * @param array $form
   *   The current form array.
   * @param array $results
   *   The results to format. This will be an array of standard objects where
   *   the keys map to the keys in $info['fields'].
   */
  public function formatResults(&$form, $results) {
    parent::formatResults($form, $results);

    // @debug dpm($results, 'results');
    if (isset($form['results'])) {
      $header = [
        'map' => 'Map',
        'marker_pos' => 'Marker Position',
        'qtl_name' => 'QTL Name',
        'qtl_peak' => 'QTL Peak',
        'qtl_start' => 'QTL Start',
        'qtl_end' => 'QTL End',
      ];
      $qtl_rows = [];
      foreach($form['results']['#rows'] as $k => $row) {
        $qtl = json_decode($row['qtl']);

        $qtl_rows = [];
        foreach($qtl as $q) {
          $qtl_rows[] = [
            'map' => $q->map_name,
            'marker_pos' => $q->marker_pos,
            'qtl_name' => $q->qtl_name,
            'qtl_peak' => $q->qtl_peak,
            'qtl_start' => $q->qtl_start,
            'qtl_end' => $q->qtl_end,
          ];
        }

        $form['results']['#rows'][$k]['qtl'] = theme('table', [
          'header' => $header,
          'rows' => $qtl_rows,
          'sticky' => FALSE,
        ]);
      }
    }
  }

  /**
   * Retrieve the type ids we need for our query.
   */
  private function getTypeIDS() {
    $ids = [];
    $term_identifiers = [
      'start' => ['id' => 'MAIN:start'],
      'end' => ['id' => 'MAIN:end'],
      //'locus' => ['id' => 'MAIN:marker_locus'],
      //'lod' => ['id' => 'MAIN:lod'],
      //'addt_effect' => ['id' => 'MAIN:additive_effect'],
      //'parent' => ['id' => 'MAIN:direction'],
      //'instance_of' => ['id' => 'OBO_REL:instance_of'],
    ];
    foreach ($term_identifiers as $name => $identifiers) {
      $term = chado_get_cvterm($identifiers);
      if ($term) {
        $ids[$name] = $term->cvterm_id;
      }
      else {
        $ids[$name] = 0;
      }
    }
    return $ids;
  }

  /**
   * Retrieve the default values for the form based on form_state and build_args.
   *
   * @param array $form_state
   *   The current state of the form.
   * @return array
   *   The default values for the form based on the form_state.
   */
  public function getDefaultValues($form_state) {
    $values = [];

    $q = drupal_get_query_parameters();
    $filter_keys = array_keys(self::$info['filters']);
    foreach ($filter_keys as $key) {
      $values[$key] = '';
      if (isset($q[$key])) {
        $values[$key] = $q[$key];
      }
      if (isset($form_state['values'])) {
        if (isset($form_state['values'][$key]) && !empty($form_state['values'][$key])) {
          $values[$key] = $form_state['values'][$key];
        }
      }
    }

    return $values;
  }
}

/**
 * AJAX Callback: refresh trait fieldset after genus is selected.
 */
function ajax_tqtlm_refreshtrait_callback($form, $form_state) {
  return $form['s1'];
}

/**
 * AJAX Callback: refresh qtl fieldset after trait is selected.
 */
function ajax_tqtlm_refreshqtl_callback($form, $form_state) {
  return $form['s2'];
}

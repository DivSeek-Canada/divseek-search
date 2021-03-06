<?php

/**
 * Provides a search for
 *
 */
class GermplasmAccessionSearch extends ChadoCustomSearch {

  /**
   * The human readable title of this search. This will be used in listings
   * and shown on the search page as the title.
   */
  public static $title = 'Germplasm Accessions';

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
  public static $require_submit = FALSE;

  /**
   * Add a pager to search results
   * with $num_items_per_page results on a single page.
   * NOTE: Your query has to handle paging.
   */
  public static $pager = TRUE;
  public static $num_items_per_page = 25;

  /**
   * This defined the hook_menu definition for this search. At a minimum the
   * path is required.
   */
  public static $menu = [
    'path' => 'search/germplasm',
    // @todo support menu items.
  ];

  /**
   * Add CSS/JS to the form/results page.
   * NOTE: paths supplied should be relative to $module.
   */
  public static $attached = [
    'css' => [
      'css/all_searches.css',
    ],
    'js' => [],
  ];

  /**
   * Information regarding the fields and filters for this search.
   */
  public static $info = [
    // Lists the columns in your results table.
    'fields' => [
      'name' => [
        'title' => 'Name',
        'entity_link' => [
          'chado_table' => 'stock',
          'id_column' => 'stock_id'
        ],
      ],
      'uniquename' => [
        'title' => 'Accession',
        'entity_link' => [
          'chado_table' => 'stock',
          'id_column' => 'stock_id'
        ],
      ],
      'biological_status' => [
        'title' => 'Biological Status'
      ],
      'pedigree' => [
        'title' => 'Pedigree',
      ],
      'source_institute' => [
        'title' => 'Source Institute',
      ],
      'origin' => [
        'title' => 'Origin',
      ],
      'species' => [
        'title' => 'Source Species',
        'entity_link' => [
          'chado_table' => 'organism',
          'id_column' => 'organism_id'
        ],
      ],
    ],
    // The filter criteria available to the user.
    // This is used to generate a search form which can be altered.
    'filters' => [
      'genus' => [
        'title' => 'Genus',
        'help' => 'The genus the germplasm belongs to.',
      ],
      'species' => [
        'title' => 'Species',
        'help' => 'The species the germplasm belongs to.',
      ],
      'biological_status' => [
        'title' => 'Biological Status',
        'help' => 'The biological status of the germplasm you are looking for (e.g. wild, landrace, cultivar)',
      ],
      'origin' => [
        'title' => 'Germplasm Origin',
        'help' => 'The country the seed originated from.'
      ],
      'source_institute' => [
        'title' => 'Source Institute',
        'help' => 'The institute the germplasm was developed or collected by.'
      ],
      'phenotypic_value' => [
        'title' => 'Phenotype',
        'help' => 'Germplasm whose collected phenotypic data matches the criteria.'
      ],
      'name' => [
        'title' => 'Name',
        'help' => 'The name or accession of the germplasm (e.g. CDC Redberry; partial names are accepted).',
      ],
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

    // Get Default Values.
    //---------------------------------
    $q = drupal_get_query_parameters();
    if (isset($form_state['values'])) {
      $genus_default = $form_state['values']['genus'];
      $species_default = $form_state['values']['species'];
      $phenovalue_default = $form_state['values']['phenotypic_value'];
    }
    elseif (isset($q['genus'])) {
      $genus_default = $q['genus'];
      $species_default = $q['species'];
      $phenovalue_default = $q['phenotypic_value'];
    }
    else {
      $genus_default = NULL;
      $species_default = NULL;
      $phenovalue_default = NULL;
    }

    $form['genus']['#default_value'] = $genus_default;
    $form['species']['#default_value'] = $species_default;
    $form['phenotypic_value']['#default_value'] = $phenovalue_default;


    // We want a drop-down for:
    //---------------------------------
    $term_ids = $this->getTypeIDS();

    // -- Genus.
    $options = chado_query(
      "SELECT genus FROM {organism}
       WHERE organism_id IN (SELECT organism_id FROM {stock})
       ORDER BY genus ASC")->fetchAllKeyed(0,0);
    $form['genus']['#type'] = 'select';
    $form['genus']['#options'] = $options;
    $form['genus']['#empty_option'] = '- Select -';


    // -- Biological Status.
    $options = chado_query(
      "SELECT value FROM {stockprop}
       WHERE type_id = :id
       GROUP BY value
       ORDER BY value ASC",
       [':id' => $term_ids['biological_status']])->fetchAllKeyed(0,0);
    $form['biological_status']['#type'] = 'select';
    $form['biological_status']['#options'] = $options;
    $form['biological_status']['#empty_option'] = '- Select -';

    // -- Origin.
    $options = chado_query(
      "SELECT value FROM {stockprop}
       WHERE type_id = :id
       GROUP BY value
       ORDER BY value ASC",
       [':id' => $term_ids['origin']])->fetchAllKeyed(0,0);
    $form['origin']['#type'] = 'select';
    $form['origin']['#options'] = $options;
    $form['origin']['#empty_option'] = '- Select -';

    // -- Source Institute.
    $options = chado_query(
      "SELECT value FROM {stockprop}
       WHERE type_id = :id
       GROUP BY value
       ORDER BY value ASC",
       [':id' => $term_ids['source_institute']])->fetchAllKeyed(0,0);
    $form['source_institute']['#type'] = 'select';
    $form['source_institute']['#options'] = $options;
    $form['source_institute']['#empty_option'] = '- Select -';

    // Scaffold our fields into rows.
    //---------------------------------
    // Genus     |  Species            |
    // Status | Institute  | Country   |
    // Name                            |
    //---------------------------------

    // First ensure they are in the right order.
    $form['genus']['#weight'] = -11;
    $form['species']['#weight'] = -10;
    $form['biological_status']['#weight'] = -6;
    $form['source_institute']['#weight'] = -5;
    $form['origin']['#weight'] = -4;
    $form['name']['#weight'] = 10;

    // Now start grouping each row.
    $form['genus']['#prefix'] = '<div class="search-row">';
    $form['species']['#suffix'] = '</div>';
    $form['biological_status']['#prefix'] = '<div class="search-row">';;
    $form['origin']['#suffix'] = '</div>';
    $form['name']['#prefix'] = '<div class="search-row">';;
    $form['name']['#suffix'] = '</div>';

    // Add AJAX to genus since phenotypic value needs to be refreshed.
    //-----------------------------------------------------------------
		$form['genus']['#ajax'] = [
			'wrapper' => 'multi-phenotypicvalue-field-wrapper',
			'callback' => 'ajax_phenotype_value_field_refresh',
			'element' => 'phenotypic_value',
		];

    // Ensure phenotypic value field uses the correct form element.
    //--------------------------------------------------------------
    $form['phenotypic_value']['#type'] = 'multi_phenotypic_value';
    $form['phenotypic_value']['#genus'] = $genus_default;
    $form['phenotypic_value']['#description'] = '';
    if (isset($form['phenotypic_value']['#default_value'])) {
      $defaultval = $form['phenotypic_value']['#default_value'];
      if (isset($defaultval['add_trait'])) {
        unset($defaultval['add_trait']);
      }
      if (isset($defaultval['remove_trait'])) {
        unset($defaultval['remove_trait']);
      }
      $form['phenotypic_value']['#num_traits'] = sizeof($defaultval);
      $form['phenotypic_value']['#default_value'] = $defaultval;
    }
    // @debug dpm($form_state, 'Germplasm Search: form state');

    return $form;
  }

  /**
   * Allows custom searches to validate the form results.
   * Use form_set_error() to signal invalid values.
   */
  public function validateForm($form, $form_state) {

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
    global $user;

    // Retrieve the filter results already set.
    $filter_results = $this->values;
    // @debug dpm($filter_results, '$filter_results');

    // Retrieve the cvterm_ids we need for our query.
    $term_ids = $this->getTypeIDS();
    // @debug dpm($term_ids, 'Term IDS');

    $query = "SELECT
        o.genus||' '||o.species as species,
        o.organism_id,
        s.name, s.uniquename,
        s.stock_id,
        origin.value as origin,
        source.value as source_institute,
        pedigree.value as pedigree,
        status.value as biological_status
      FROM {stock} s
      LEFT JOIN {organism} o ON o.organism_id=s.organism_id
      LEFT JOIN {stockprop} source ON source.stock_id=s.stock_id AND source.type_id=:source_institute_type
      LEFT JOIN {stockprop} origin ON origin.stock_id=s.stock_id AND origin.type_id=:origin_type
      LEFT JOIN {stockprop} pedigree ON pedigree.stock_id=s.stock_id AND pedigree.type_id=:pedigree_type
      LEFT JOIN {stockprop} status ON status.stock_id=s.stock_id AND status.type_id=:biological_status_type ";

    $where = [
      // Restrict the dataset to only accessions.
      "s.type_id IN (SELECT cvterm_id FROM {cvterm} WHERE name~'accession')"
    ];
    $joins = [];

    $args[':source_institute_type'] = $term_ids['source_institute'];
    $args[':pedigree_type'] = $term_ids['pedigree'];
    $args[':biological_status_type'] = $term_ids['biological_status'];
    $args[':origin_type'] = $term_ids['origin'];

    // -- Genus.
    if ($filter_results['genus'] != '') {
      $where[] = "o.genus ~* :genus";
      $args[':genus'] = $filter_results['genus'];
    }

    // -- Species.
    if ($filter_results['species'] != '') {
      $where[] = "o.species ~* :species";
      $args[':species'] = $filter_results['species'];
    }

    // -- Source Institute.
    if ($filter_results['source_institute'] != '') {
      $where[] = "source.value ~* :source_institute";
      $args[':source_institute'] = $filter_results['source_institute'];
    }

    // -- Origin.
    if ($filter_results['origin'] != '') {
      $where[] = "origin.value ~* :origin";
      $args[':origin'] = $filter_results['origin'];
    }

    // -- Biological Status.
    if ($filter_results['biological_status'] != '') {
      $where[] = "status.value ~* :biological_status";
      $args[':biological_status'] = $filter_results['biological_status'];
    }

    // -- Phenotypic Value.
    if (is_array($filter_results['phenotypic_value'])) {
      unset($filter_results['phenotypic_value']['add_trait']);
      unset($filter_results['phenotypic_value']['remove_trait']);

      foreach($filter_results['phenotypic_value'] as $k => $singletrait) {

        // Check if this trait is empty.
        if (!is_array($singletrait)) {
          continue;
        }
        if ($singletrait['trait'] == '') {
          continue;
        }
        if ($singletrait['unit'] == '') {
          continue;
        }

        $op = $singletrait['operator'];
        $plhd_trait = ':' . $k . 'trait';
        $plhd_unit = ':' . $k . 'unit';
        $plhd_value = ':' . $k . 'value';
        $plhd_scope = ':' . $k . 'scope';
        switch ($singletrait['scope']) {
          case 'measurement':
            $where[] = "
              s.stock_id IN (
                SELECT stock_id FROM {phenotype}
                WHERE attr_id = $plhd_trait AND unit_id = $plhd_unit
                  AND value $op $plhd_value)";
            break;
          case 'siteyear':
          case 'experiment':
          case 'trait':
            $where[] = "
              s.stock_id IN (
                SELECT stock_id FROM {mview_phenotype_average}
                WHERE trait_id = $plhd_trait AND unit_id = $plhd_unit
                  AND scope = $plhd_scope AND
                  (mean_value $op $plhd_value OR qual_values $op $plhd_value))";
            $args[$plhd_scope] = $singletrait['scope'];
            break;
          default:
            $where[] = "
              s.stock_id IN (
                SELECT stock_id FROM {mview_phenotype_average}
                WHERE trait_id = $plhd_trait AND unit_id = $plhd_unit
                  AND scope = $plhd_scope AND
                  (mean_value $op $plhd_value OR qual_values $op $plhd_value))";
            $args[$plhd_scope] = 'trait';
            break;
        }

        $args[$plhd_trait] = $singletrait['trait'];
        $args[$plhd_unit] = $singletrait['unit'];
        $args[$plhd_value] = $singletrait['value'];
      }
    }

    // -- Name.
    if ($filter_results['name'] != '') {
      $where[] = "(s.name ~* :name OR s.uniquename = :name)";
      $args[':name'] = $filter_results['name'];
    }

    // Finally, add it to the query.
    if (!empty($joins)) {
      $query .= "\n" . implode("\n",$joins);
    }
    if (!empty($where)) {
      $query .= "\n" . ' WHERE ' . implode(' AND ',$where);
    }

    // Sort even though it is SLOW b/c ppl expect it.
    $query .= "\n" . ' ORDER BY s.name ASC';

    // Handle paging.
    $limit = $this::$num_items_per_page + 1;
    $query .= "\n" . ' LIMIT ' . $limit . ' OFFSET ' . $offset;

    // @debug dpm(strtr(str_replace(['{','}'], ['chado.', ''], $query), $args), 'query');
  }

  /**
   * Retrieve the type ids we need for our query.
   */
  private function getTypeIDS() {
    $ids = [];

    $term_identifiers = [
      'biological_status' => ['id' => 'CO_010:0000022'],
      'source_institute' => ['id' => 'CO_020:0000097'],
      'origin' => ['id' => 'CO_010:0000010'],
      'pedigree' => ['id' => 'CO_010:0000052'],
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

}

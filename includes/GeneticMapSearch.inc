<?php

/**
 * Provides a search for chado featuremap-based Tripal Content.
 *
 */
class GeneticMapSearch extends ChadoCustomSearch {

  /**
   * The human readable title of this search. This will be used in listings
   * and shown on the search page as the title.
   */
  public static $title = 'Genetic Map Search';

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
  public static $num_items_per_page = 50;

  /**
   * This defined the hook_menu definition for this search. At a minimum the
   * path is required.
   */
  public static $menu = [
    'path' => 'search/genetic-maps',
    // @todo support menu items.
  ];

  /**
   * Add CSS/JS to the form/results page.
   * NOTE: paths supplied should be relative to $module.
   */
  public static $attached = [
    'css' => [],
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
          'chado_table' => 'featuremap',
          'id_column' => 'featuremap_id'
        ],
      ],
      'pop_name' => [
        'title' => 'Name'
      ],
      'pop_type' => [
        'title' => 'Type'
      ],
      'pop_size' => [
        'title' => 'Size'
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
        'help' => 'The crop species of the population the genetic map was created for (e.g. Lens).',
      ],
      'pop_type' => [
        'title' => 'Population Type',
        'help' => 'The type of population the genetic map was made for.',
      ],
      'pop_name' => [
        'title' => 'Population Name',
        'help' => 'The name of population the genetic map was made for.',
      ],
      'name' => [
        'title' => 'Map Name',
        'help' => 'The name of the map you are interested in (partial names are accepted).',
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

    // Retrieve the cvterm_ids we need for our query.
    $term_ids = $this->getTypeIDS();
    // @debug dpm($term_ids, 'Term IDS');

    // -- Genus.
    $options = chado_query(
      "SELECT genus FROM {organism}
       WHERE organism_id IN (SELECT organism_id FROM {stock})
       ORDER BY genus ASC")->fetchAllKeyed(0,0);
    $form['genus']['#type'] = 'select';
    $form['genus']['#options'] = $options;
    $form['genus']['#empty_option'] = '- Select -';

    // Make the Population type filter a drop-down.
    $options = chado_query("SELECT value
      FROM {featuremapprop}
      WHERE type_id=:pop_type
      ORDER BY value ASC",
      [':pop_type' => $term_ids['pop_type']])->fetchAllKeyed(0,0);
    $form['pop_type']['#type'] = 'select';
    $form['pop_type']['#options'] = $options;
    $form['pop_type']['#empty_option'] = ' - Select -';

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
    // @debug dpm($filter_results, '$filter_results');

    // Retrieve the cvterm_ids we need for our query.
    $term_ids = $this->getTypeIDS();
    // @debug dpm($term_ids, 'Term IDS');

    $query = "
      SELECT
        map.featuremap_id, map.name,
        o.organism_id, o.genus||' '||o.species as species,
        pop_name.value as pop_name,
        pop_type.value as pop_type,
        pop_size.value as pop_size
      FROM {featuremap} map
      LEFT JOIN {featuremap_organism} forg ON forg.featuremap_id=map.featuremap_id
      LEFT JOIN {organism} o ON o.organism_id=forg.organism_id
      LEFT JOIN {featuremapprop} pop_name ON pop_name.featuremap_id=map.featuremap_id AND pop_name.type_id=:pop_name
      LEFT JOIN {featuremapprop} pop_type ON pop_type.featuremap_id=map.featuremap_id AND pop_type.type_id=:pop_type
      LEFT JOIN {featuremapprop} pop_size ON pop_size.featuremap_id=map.featuremap_id AND pop_size.type_id=:pop_size";

    $where = [];
    $joins = [];
    $args = [
      ':pop_name' => $term_ids['pop_name'],
      ':pop_type' => $term_ids['pop_type'],
      ':pop_size' => $term_ids['pop_size'],
    ];

    // -- Genus.
    if ($filter_results['genus'] != '') {
      $where[] = "o.genus ~* :genus";
      $args[':genus'] = $filter_results['genus'];
    }

    // -- Population Type.
    if ($filter_results['pop_type'] != '') {
      $where[] = "pop_type.value ~ :pop_type";
      $args[':pop_type'] = $filter_results['pop_type'];
    }

    // -- Population Name.
    if ($filter_results['pop_name'] != '') {
      $where[] = "pop_name.value ~ :pop_name";
      $args[':pop_name'] = $filter_results['pop_name'];
    }

    // -- Name.
    if ($filter_results['name'] != '') {
      $where[] = "map.name ~ :name";
      $args[':name'] = $filter_results['name'];
    }

    // Finally, add it to the query.
    if (!empty($joins)) {
      $query .= implode("\n",$joins);
    }
    if (!empty($where)) {
      $query .= "\n" . ' WHERE ' . implode(' AND ',$where);
    }

    // Sort even though it is SLOW b/c ppl expect it.
    $query .= "\n" . ' ORDER BY map.name ASC';

    // Handle paging.
    $limit = $this::$num_items_per_page + 1;
    $query .= "\n" . ' LIMIT ' . $limit . ' OFFSET ' . $offset;

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

    if (isset($form['results'])) {
      $header1 = [
        'name' => '',
        'pop' => [
          'data' => 'Population',
          'colspan' => 4,
          'style' => 'text-align: center',
        ],
      ];
      $header2 = $form['results']['#header'];
      foreach ($header2 as $key => $label) {
        $header2[$key] = [
          'data' => $label,
          'header' => TRUE,
        ];
      }

      $form['results']['#header'] = $header1;
      array_unshift($form['results']['#rows'], $header2);
    }
  }

  /**
   * Retrieve the type ids we need for our query.
   */
  private function getTypeIDS() {
    $ids = [];
    $term_identifiers = [
      'pop_type' => ['id' => 'MAIN:population_type'],
      'pop_size' => ['id' => 'MAIN:population_size'],
      'pop_name' => ['id' => 'MAIN:published_map_name'],
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

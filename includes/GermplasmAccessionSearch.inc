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

    // Move name to the bottom.
    $form['name']['#weight'] = 10;

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

    $where = [];
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

    // -- Name.
    if ($filter_results['name'] != '') {
      $where[] = "(s.name ~* :name OR s.uniquename = :name)";
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

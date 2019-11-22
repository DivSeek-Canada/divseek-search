<?php
/**
 * Provides a search for
 *
 */
class TraitSearch extends ChadoCustomSearch {

  /**
   * The human readable title of this search. This will be used in listings
   * and shown on the search page as the title.
   */
  public static $title = 'Phenotypic Traits';

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
    'path' => 'search/traits',
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
        'title' => 'Trait',
        'entity_link' => [
          'chado_table' => 'cvterm',
          'id_column' => 'cvterm_id'
        ],
      ],
    ],
    // The filter criteria available to the user.
    // This is used to generate a search form which can be altered.
    'filters' => [
      'name' => [
        'title' => 'Trait Name',
        'help' => 'The name of the phenotypic trait you are interested in (e.g. Days to Flowering; partial names are accepted).',
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
    $trait_ids = array_keys($this->getTraitIDS());
    if (empty($trait_ids)) {
      $query = "SELECT FALSE";
      return $query;
    }
    $query = "SELECT
        trait.cvterm_id, trait.name
      FROM {cvterm} trait";

    $where = [
      // Restrict the dataset to only accessions.
      "trait.cv_id IN (:trait_ids)"
    ];
    $args[':trait_ids'] = $trait_ids;
    $joins = [];

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
    $query .= "\n" . ' ORDER BY trait.name ASC';

    // Handle paging.
    $limit = $this::$num_items_per_page + 1;
    $query .= "\n" . ' LIMIT ' . $limit . ' OFFSET ' . $offset;

    // @debug dpm(strtr(str_replace(['{','}'], ['chado.', ''], $query), $args), 'query');
  }

  /**
   * Retrieve the cv_ids for traits.
   */
  private function getTraitIDS() {
    $ids = [];

    $ids = chado_query("
      SELECT cb.type_value, tb.label
      FROM chado_bundle cb
      LEFT JOIN tripal_bundle tb ON cb.bundle_id=tb.id WHERE data_table='cvterm'")->fetchAllKeyed(0,1);

    return $ids;
  }

}

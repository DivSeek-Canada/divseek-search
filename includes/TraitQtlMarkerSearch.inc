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
  public static $num_items_per_page = 50;

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
      'name' => [
        'title' => 'Marker Name',
        'entity_link' => [
          'chado_table' => 'feature',
          'id_column' => 'feature_id'
        ],
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
    $form['s2']['genetic_maps']['#prefix'] = '<div id="tqtlm-s2qtl-flex">';
    $form['s2']['qtl'] = $form['qtl'];
    $form['s2']['qtl']['#weight'] = -9;
    $form['s2']['qtl']['#suffix'] = '</div>';
    unset($form['genetic_maps'], $form['qtl']);

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


    // Make into Drop-downs.
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
      $options = chado_query(
        "SELECT name FROM {cvterm}
         WHERE cvterm_id IN (
           SELECT trait_id
           FROM {mview_phenotype_average}
           WHERE organism_genus=:genus)
         ORDER BY name ASC",
         [':genus' => $values['genus']])->fetchAllKeyed(0,0);
    }
    $form['s1']['trait']['#type'] = 'select';
    $form['s1']['trait']['#options'] = $options;
    $form['s1']['trait']['#empty_option'] = '- Select -';

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

    }
  }

  /**
   * Retrieve the type ids we need for our query.
   */
  private function getTypeIDS() {
    $ids = [];
    $term_identifiers = [
      //'pop_type' => ['id' => 'MAIN:population_type'],
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

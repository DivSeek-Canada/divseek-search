<?php

/**
 * Provides a search for SO:genetic_marker chado feature-based Tripal Content.
 *
 */
class GeneticMarkerSearch extends KPSEARCH {

  /**
   * The human readable title of this search. This will be used in listings
   * and shown on the search page as the title.
   */
  public static $title = 'Genetic Marker Search';

  /**
   * The machine name of the module providing this search.
   */
  public static $module = 'kp_searches';

  /**
   * A description of this search. This is shown at the top of the search page
   * and used for the menu item.
   */
  public static $description = 'While the term variant refers to a position in the genome with variation, genetic marker refers specifically to an assay which samples that variation.';

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
    'path' => 'search/markers',
    // @todo support menu items.
  ];

  /**
   * Add CSS/JS to the form/results page.
   * NOTE: paths supplied should be relative to $module.
   */
  public static $attached = [
    'css' => [
      'css/all_kp_searches.css',
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
          'chado_table' => 'feature',
          'id_column' => 'feature_id'
        ],
      ],
      'type' => [
        'title' => 'Marker Type',
      ],
      'original_loc' => [
        'title' => 'Original Location',
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
        'help' => 'The legume species the genetic marker was designed for (e.g. Lens culinaris).',
      ],
      'species' => [
        'title' => 'Species',
        'help' => '',
      ],
      'type' => [
        'title' => 'Marker Type',
        'help' => 'The type of marker you are interested in (e.g. Illumina Golden Gate).',
        //'default' => 'Exome Capture',
      ],
      'project' => [
        'title' => 'Experiment',
        'help' => 'The experiment you are interested in (e.g. AGILE; partial names are accepted).',
      ],
      'start-backbone' => [
        'title' => 'Start: Backbone',
        'help' => 'The name of the sequence the markers are located on.'
      ],
      'start-pos' => [
        'title' => 'Start: Position',
        'help' => 'The numeric start position of the range you are interested in.'
      ],
      'end-pos' => [
        'title' => 'End: Position',
        'help' => 'The numeric end position of the range you are interested in.'
      ],
      'name' => [
        'title' => 'Name',
        'help' => 'The name or accession of the marker (e.g. LcC00002p390; partial names are accepted).',
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

    // Add a crop selector.
    $this->addCropFormElement($form, $form_state);

    // Add a Scientific name selector.
    $this->addSpeciesFormElement($form, $form_state);

    // Make the marker type a drop-down.
    $options = unserialize(variable_get('kp_searches_marker_types', 'a:0{}'));
    $form['type']['#type'] = 'select';
    $form['type']['#options'] = $options;
    $form['type']['#empty_option'] = ' - Select -';

    // Make the project an autocomplete.
    // DEPENDENCY: nd_genotypes.
    $form['project']['#autocomplete_path'] = 'tripal_ajax/nd_genotypes/project/name';

    // Add a sequence range element.
    $description = 'The range of the genome you would like to display variants from.';
    $form['seq_range'] = array(
      '#type' => 'markup',
      '#prefix' => '<div class="seq-range-element form-item"><label>Genome Range</label>',
      '#suffix' => '<div class="description">' . $description . '</div></div>',
    );
    // Group the sequence range elements.
    $form['seq_range']['start-label'] = [
      '#type' => 'item',
      '#markup' => 'From',
    ];
    $form['seq_range']['start-backbone'] = $form['start-backbone'];
    $form['seq_range']['start-backbone']['#attributes']['placeholder'] = 'Sequence Name';
    unset($form['start-backbone']);
    $form['seq_range']['start-pos'] = $form['start-pos'];
    $form['seq_range']['start-pos']['#attributes']['placeholder'] = 'Start Position';
    unset($form['start-pos']);
    $form['seq_range']['end-label'] = [
      '#type' => 'item',
      '#markup' => 'to',
    ];
    $form['seq_range']['end-pos'] = $form['end-pos'];
    $form['seq_range']['end-pos']['#attributes']['placeholder'] = 'End Position';
    unset($form['end-pos']);

    return $form;
  }

  /**
   * Allows custom searches to validate the form results.
   * Use form_set_error() to signal invalid values.
   */
  public function validateForm($form, $form_state) {
    $values = $form_state['values'];

    // If any of the range elements are filled out...
    if ($values['start-backbone'] != ''
      OR $values['start-pos'] != ''
      OR $values['end-pos'] != '') {

      // Then they all need to be!
      if ($values['start-backbone'] == '') {
        form_set_error('start-backbone',
          'Genome Range: The sequence name is required.');
      }
      if ($values['start-pos'] == '') {
        form_set_error('start-pos',
          'Genome Range: The range start position is required.');
      }
      if ($values['end-pos'] == '') {
        form_set_error('end-pos',
          'Genome Range: The range end position is required.');
      }
    }
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

    $query = "
      SELECT
        marker.feature_id, marker.name as name,
        type.value as type,
        olocparent.name||':'||original_loc.fmin||'..'||original_loc.fmax as original_loc,
        o.organism_id, o.genus||' '||o.species as species
      FROM {feature} marker
      LEFT JOIN {organism} o ON o.organism_id=marker.organism_id
      LEFT JOIN {featureloc} original_loc ON original_loc.feature_id = marker.feature_id AND original_loc.locgroup=0
      LEFT JOIN {feature} olocparent ON olocparent.feature_id=original_loc.srcfeature_id
      LEFT JOIN {featureprop} type ON type.feature_id = marker.feature_id
        AND type.type_id IN
          (SELECT cvterm_id FROM chado.cvterm WHERE name='additionalType') ";

    $where = ["marker.type_id IN (SELECT cvterm_id FROM {cvterm} WHERE name='genetic_marker')"];
    $joins = [];

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

    // -- Type.
    if ($filter_results['type'] != '') {
      $where[] = "type.value ~ :type";
      $args[':type'] = $filter_results['type'];
    }

    // -- Project.
    if ($filter_results['project'] != '') {
      $where[] = "project.name ~ :project";
      $args[':project'] = $filter_results['project'];
      $joins[] = 'LEFT JOIN {project_feature} pf ON pf.feature_id=marker.feature_id';
      $joins[] = 'LEFT JOIN {project} project ON project.project_id=pf.project_id';
    }

    // -- Name.
    if ($filter_results['name'] != '') {
      $where[] = "(marker.name ~ :name OR marker.uniquename = :name)";
      $args[':name'] = $filter_results['name'];
    }

    // -- Genome Range.
    if ($filter_results['start-pos'] != ''
      AND $filter_results['end-pos'] != '') {

        $joins[] = 'LEFT JOIN {featureloc} loc
          ON loc.feature_id=marker.feature_id';
        $where[] = 'loc.fmin BETWEEN :start AND :end';
        $args[':start'] = $filter_results['start-pos'];
        $args[':end'] = $filter_results['end-pos'];
        $where[] = 'loc.srcfeature_id IN
          (SELECT feature_id FROM {feature}
          WHERE name=:backbone OR uniquename=:backbone)';
        $args[':backbone'] = $filter_results['start-backbone'];
    }

    // Finally, add it to the query.
    if (!empty($joins)) {
      $query .= implode("\n",$joins);
    }
    if (!empty($where)) {
      $query .= "\n" . ' WHERE ' . implode(' AND ',$where);
    }

    // Sort even though it is SLOW b/c ppl expect it.
    $query .= "\n" . ' ORDER BY marker.name ASC';

    // Handle paging.
    $limit = $this::$num_items_per_page + 1;
    $query .= "\n" . ' LIMIT ' . $limit . ' OFFSET ' . $offset;

    // @debug dpm(strtr(str_replace(['{','}'], ['chado.', ''], $query), $args), 'query');
  }
}

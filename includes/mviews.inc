<?php

/**
 *
 */

/**
 *
 */
function divseek_search_mview_phenotype_means() {
	$schema = array(
	  'description' => 'Caches phenotypic data for easier retrieval of means. Data replicates are combined.',
	  'table' => 'mview_phenotype_average',
	  'fields' => array(
	    'organism_genus' => array(
	      'type' => 'varchar',
	      'length' => '255',
	      'not null' => true,
	    ),
	    'trait_id' => array(
	      'size' => 'big',
	      'type' => 'int',
	      'not null' => true,
	    ),
	    'experiment_id' => array(
	      'size' => 'big',
	      'type' => 'int',
	      'not null' => false,
	    ),
	    'method_id' => array(
	      'size' => 'big',
	      'type' => 'int',
	      'not null' => true,
	    ),
	    'unit_id' => array(
	      'size' => 'big',
	      'type' => 'int',
	      'not null' => true,
	    ),
	    'location' => array(
	      'type' => 'text',
	      'not null' => false,
	    ),
	    'year' => array(
	      'type' => 'text',
	      'not null' => false,
	    ),
	    'stock_id' => array(
	      'size' => 'big',
	      'type' => 'int',
	      'not null' => true,
	    ),
	    'scope' => array(
	      'type' => 'varchar',
	      'length' => '255',
	      'not null' => true,
	    ),
	    'value' => array(
	      'type' => 'varchar',
	      'length' => '255',
	      'not null' => true,
	    ),
	  ),
	  'indexes' => array(
	  ),
	);
	$query = "    SELECT
      o.genus         AS organism_genus,
      p.attr_id       AS trait_id,
      p.project_id    AS project_id,
      p.assay_id      AS method_id,
      p.unit_id       AS unit_id,
      loc.value       AS location,
      yr.value        AS year,
      'siteyear'      AS scope,
      p.stock_id      AS stock_id,
      avg(p.value) AS value
    FROM {phenotype} p
      LEFT JOIN {organism} o ON o.organism_id=s.organism_id
      LEFT JOIN {phenotypeprop} loc ON loc.phenotype_id=p.phenotype_id AND loc.type_id = 3036
      LEFT JOIN {phenotypeprop} yr ON yr.phenotype_id=p.phenotype_id AND yr.type_id = 141
    GROUP BY
      p.attr_id,
      p.project_id,
      p.assay_id,
      p.unit_id,
      loc.value,
      yr.value,
      p.stock_id,
      o.genus
UNION
    SELECT
      o.genus         AS organism_genus,
      p.attr_id       AS trait_id,
      p.project_id    AS project_id,
      p.assay_id      AS method_id,
      p.unit_id       AS unit_id,
      NULL            AS location,
      NULL            AS year,
      'experiment'    AS scope,
      p.stock_id      AS stock_id,
      avg(p.value) AS value
    FROM {phenotype} p
      LEFT JOIN {organism} o ON o.organism_id=s.organism_id
    GROUP BY
      p.attr_id,
      p.project_id,
      p.assay_id,
      p.unit_id,
      p.stock_id,
      o.genus
UNION
    SELECT
      o.genus         AS organism_genus,
      p.attr_id       AS trait_id,
      NULL            AS project_id,
      p.assay_id      AS method_id,
      p.unit_id       AS unit_id,
      NULL            AS location,
      NULL            AS year,
      'trait'         AS scope,
      p.stock_id      AS stock_id,
      avg(p.value) AS value
    FROM {phenotype} p
      LEFT JOIN {organism} o ON o.organism_id=s.organism_id
    GROUP BY
      p.attr_id,
      p.assay_id,
      p.unit_id,
      p.stock_id,
      o.genus";

	chado_add_mview(
		'mview_phenotype_average',
		'divseek_search',
		$schema,
		$query,
		NULL,
		FALSE
	);

	$mview_id = chado_get_mview_id('mview_phenotype_average');
	chado_populate_mview($mview_id);
}

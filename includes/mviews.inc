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
      'mean_value' => array (
        'type' => 'float',
      ),
      'std_dev' => array (
        'type' => 'float',
      ),
      'qual_values' => array (
        'type' => 'text',
      ),
	  ),
	  'indexes' => array(
	  ),
	);

	$sysvar = ap_get_variablenames(
    array('variablename' => 'varset'),
    array('set' => 'terms')
  );
  $loc_typeid = variable_get($sysvar['ap_location']);
  $yr_typeid = variable_get($sysvar['ap_year']);
	$query = "    SELECT
      o.genus         AS organism_genus,
      p.attr_id       AS trait_id,
      p.project_id    AS project_id,
      p.assay_id      AS method_id,
      p.unit_id       AS unit_id,
      loc.value       AS location,
      yr.value        AS year,
      p.stock_id      AS stock_id,
      'siteyear'      AS scope,
      avg(cast(p.value as real)) AS mean_value,
      stddev(cast(p.value as real)) AS std_dev,
			NULL as qual_values
    FROM {phenotype} p
      LEFT JOIN {stock} s ON s.stock_id=p.stock_id
      LEFT JOIN {organism} o ON o.organism_id=s.organism_id
      LEFT JOIN {phenotypeprop} loc ON loc.phenotype_id=p.phenotype_id AND loc.type_id = ".$loc_typeid."
      LEFT JOIN {phenotypeprop} yr ON yr.phenotype_id=p.phenotype_id AND yr.type_id = ".$yr_typeid."
    WHERE
      p.unit_id IN (SELECT cvterm_id FROM {cvtermprop} WHERE value='quantitative')
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
      p.stock_id      AS stock_id,
      'experiment'    AS scope,
      avg(cast(p.value as real)) AS mean_value,
      stddev(cast(p.value as real)) AS std_dev,
			NULL as qual_values
    FROM {phenotype} p
      LEFT JOIN {stock} s ON s.stock_id=p.stock_id
      LEFT JOIN {organism} o ON o.organism_id=s.organism_id
    WHERE
      p.unit_id IN (SELECT cvterm_id FROM {cvtermprop} WHERE value='quantitative')
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
      p.stock_id      AS stock_id,
      'trait'         AS scope,
      avg(cast(p.value as real)) AS mean_value,
      stddev(cast(p.value as real)) AS std_dev,
			NULL as qual_values
    FROM {phenotype} p
      LEFT JOIN {stock} s ON s.stock_id=p.stock_id
      LEFT JOIN {organism} o ON o.organism_id=s.organism_id
    WHERE
      p.unit_id IN (SELECT cvterm_id FROM {cvtermprop} WHERE value='quantitative')
    GROUP BY
      p.attr_id,
      p.assay_id,
      p.unit_id,
      p.stock_id,
      o.genus
UNION
	SELECT
		o.genus         AS organism_genus,
		p.attr_id       AS trait_id,
		p.project_id    AS project_id,
		p.assay_id      AS method_id,
		p.unit_id       AS unit_id,
		loc.value       AS location,
		yr.value        AS year,
		p.stock_id      AS stock_id,
		'siteyear'      AS scope,
		NULL AS mean_value,
		NULL AS std_dev,
		mode() within group (order by p.value) AS qual_values
	FROM {phenotype} p
		LEFT JOIN {stock} s ON s.stock_id=p.stock_id
		LEFT JOIN {organism} o ON o.organism_id=s.organism_id
		LEFT JOIN {phenotypeprop} loc ON loc.phenotype_id=p.phenotype_id AND loc.type_id = ".$loc_typeid."
		LEFT JOIN {phenotypeprop} yr ON yr.phenotype_id=p.phenotype_id AND yr.type_id = ".$yr_typeid."
	WHERE
		p.unit_id IN (SELECT cvterm_id FROM {cvtermprop} WHERE value='qualitative')
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
		p.stock_id      AS stock_id,
		'experiment'    AS scope,
		NULL AS mean_value,
		NULL AS std_dev,
		mode() within group (order by p.value) AS qual_values
	FROM {phenotype} p
		LEFT JOIN {stock} s ON s.stock_id=p.stock_id
		LEFT JOIN {organism} o ON o.organism_id=s.organism_id
	WHERE
		p.unit_id IN (SELECT cvterm_id FROM {cvtermprop} WHERE value='qualitative')
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
		p.stock_id      AS stock_id,
		'trait'         AS scope,
		NULL AS mean_value,
		NULL AS std_dev,
		mode() within group (order by p.value) AS qual_values
	FROM {phenotype} p
		LEFT JOIN {stock} s ON s.stock_id=p.stock_id
		LEFT JOIN {organism} o ON o.organism_id=s.organism_id
	WHERE
		p.unit_id IN (SELECT cvterm_id FROM {cvtermprop} WHERE value='qualitative')
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
			//mode() within group (order by p.value) AS qual_values

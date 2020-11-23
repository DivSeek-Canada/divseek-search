<?php

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
      WHERE trait_id=:trait_id AND unit_id=:unit_id AND scope='trait'",
      [':trait_id' => $trait_id, ':unit_id' => $unit_id])->fetchObject();
    $diff = $range->max - $range->min;

    if ($diff <= 10) {
      $data = divseek_search_complile_smrange_quant_heatmap_data(
        $trait_id, $unit_id, $range->min, $range->max);
    }
    else {
      $data = divseek_search_complile_lrgrange_quant_heatmap_data(
        $trait_id, $unit_id, $range->min, $range->max);
    }

  }
  // Otherwise it's qualitative.
  else {
    $data = divseek_search_complile_qualitative_heatmap_data(
      $trait_id,
      $unit_id
    );
  }

  return drupal_json_output($data);
}

/**
 * Data-driven heatmap: Quantitative (i.e. scaled) data with a large range.
 */
function divseek_search_complile_lrgrange_quant_heatmap_data($trait_id, $unit_id, $min, $max) {
  $data = [];

  $num_bins = 10;
  $diff = $max - $min;
  $start = floor($min);
  $step = ceil(($diff / $num_bins));
  $end = ceil($start + ($step * $num_bins));

  $sql = "SELECT count(*) FROM {mview_phenotype_average}
    WHERE trait_id=:trait_id AND unit_id=:unit_id AND scope = 'trait'
    AND mean_value > :start AND mean_value <= :end";
  $i_end = $start - 1;
  while ($i_end <= $end) {
    $i_start = $i_end;
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

  return $data;
}

/**
 * Data-driven heatmap: Quantitative (i.e. scaled) data with a small range.
 */
function divseek_search_complile_smrange_quant_heatmap_data($trait_id, $unit_id, $min, $max) {
  $data = [];

  $num_bins = 5;
  $diff = $max - $min;
  $start = $min;
  $end = $max;
  $step = round(($diff / $num_bins), 2);

  $sql = "SELECT count(*) FROM {mview_phenotype_average}
    WHERE trait_id=:trait_id AND unit_id=:unit_id AND scope = 'trait'
    AND mean_value BETWEEN :start AND :end";
  $i_end = $start - 0.01;
  while ($i_end <= $end) {
    $i_start = $i_end + 0.01;
    $i_end = $i_start + $step - 0.01;

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

  return $data;
}

/**
 * Data-driven heatmap: Qualitative (i.e. scaled) data.
 */
function divseek_search_complile_qualitative_heatmap_data($trait_id, $unit_id) {
  $data = [];

  $sql = "SELECT qual_values, count(*) as num FROM {mview_phenotype_average}
    WHERE trait_id=:trait_id AND unit_id=:unit_id AND scope='trait'
    GROUP BY qual_values ORDER BY qual_values";
  $resource = chado_query($sql,
    [':trait_id' => $trait_id, ':unit_id' => $unit_id]);
  while ($r = $resource->fetchObject()) {
    $data[] = [
      'group' => $r->qual_values,
      'variable' => $unit_id,
      'value' => $r->num,
    ];
  }

  return $data;
}

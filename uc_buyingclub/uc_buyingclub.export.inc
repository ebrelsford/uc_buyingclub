<?php

function uc_buyingclub_download_finalized($ordering_period_id, $distributor_id) {
  // select all products with the total ordered
  $distributor_db_info = content_database_info(content_fields('field_distributor')); 
  $distributor_table = $distributor_db_info['table'];
  $distributor_column = $distributor_db_info['columns']['nid']['column'];

  $minimum_order_db_info = content_database_info(content_fields('field_minimum_order')); 
  $minimum_order_table = $minimum_order_db_info['table'];
  $minimum_order_column = $minimum_order_db_info['columns']['value']['column'];

  $unit_db_info = content_database_info(content_fields('field_unit')); 
  $unit_table = $unit_db_info['table'];
  $unit_column = $unit_db_info['columns']['value']['column'];
  
  $query = "SELECT p.title AS 'item', ctd2.". $minimum_order_column ." AS `bulk req`, ctd3.". $unit_column ." AS `unit`, up.cost * ctd2.". $minimum_order_column ." AS `cast cost`, ubp.qty_ordered / ctd2.". $minimum_order_column ." AS `cases ordered`, up.cost * ubp.qty_ordered AS `total cost` ".
    "FROM {uc_buyingclub_products} ubp, {uc_products} up, {node} p, {node} d, {". $distributor_table ."} ctd, {". $minimum_order_table ."} ctd2, {". $unit_table ."} ctd3 ".
    "WHERE ubp.product_id = p.nid ".
    "AND ctd.vid = p.vid ".
    "AND ctd.". $distributor_column ." = %d ".
    "AND ctd2.vid = p.vid ".
    "AND ctd3.vid = p.vid ".
    "AND ubp.ordering_period_id = %d ".
    "GROUP BY p.nid ".
    "ORDER BY d.title, p.title";

  $result = db_query($query, $distributor_id, $ordering_period_id);

  drupal_set_header('Content-Type: text/csv');
  drupal_set_header('Content-Disposition: attachment; filename=test.csv');

  // output as CSV
  $header = array(
    'item',
    'bulk req',
    'unit',
    'case cost',
    'cases ordered',
    'total cost',
  );
  print implode(',', $header) ."\r\n";
   
  while ($row = db_fetch_array($result)) {
    foreach ($row as $value) {
      $values[] = '"' . str_replace('"', '""', decode_entities(strip_tags($value))) . '"'; 
    }
    print implode(',', $values) ."\r\n";
    unset($values);
  }
}


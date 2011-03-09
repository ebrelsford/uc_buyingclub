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
    "AND ubp.ordering_period_id = %d AND up.nid=p.nid".
    "GROUP BY p.nid ".
    "ORDER BY d.title, p.title";

  $result = db_query($query, $distributor_id, $ordering_period_id);

  while ($row = db_fetch_array($result)) {
    $table[] = $row;
  }

  // output as CSV
  _uc_buyingclub_export_table_as_csv('for_distributor.csv', array('item', 'bulk req', 'unit', 'case cost', 'cases ordered', 'total cost',), $table);
}

function uc_buyingclub_download_grid($ordering_period_id) {
  $products = _uc_buyingclub_download_grid_get_ordered_products($ordering_period_id);

  $nids = array();
  $names = array('',);
  $prices = array('PRICE',);
  $units = array('PER',);
  $ordereds = array('NEW STOCK',);
  $availables = array('TOTAL AVAIL',);

  foreach ($products as $product) {
    $nids[] = $product['nid'];
    $names[] = $product['name'];
    $prices[] = $product['price'];
    $units[] = $product['unit'];
    $ordereds[] = $product['qty_ordered'];
    $availables[] = $product['qty_ordered'];
  }

  $table[] = $names;
  $table[] = $prices;
  $table[] = $units;
  $table[] = $ordereds;
  $table[] = $availables;

  $num_columns = count($nids) + 1;

  $users = _uc_buyingclub_download_grid_get_users($ordering_period_id);

  foreach ($users as $user) {
    $table[] = _uc_buyingclub_download_grid_get_nid_qty_row($nids, $user['last'] .', '. $user['first'], _uc_buyingclub_download_grid_get_order_products($user['order_id']));
  }

  $filler_row = array();
  $filler_row = array_pad($filler_row, $num_columns, '');
  $table[] = $filler_row;
  $table[] = $filler_row;

  $table[] = _uc_buyingclub_download_grid_get_nid_qty_row($nids, 'SOLD', _uc_buyingclub_download_grid_get_products_sold($ordering_period_id));
  $table[] = _uc_buyingclub_download_grid_get_nid_qty_row($nids, 'OVERSTOCK', _uc_buyingclub_download_grid_get_products_overstock($ordering_period_id));

  _uc_buyingclub_export_table_as_csv('grid.csv', NULL, $table);
}

/*
 * Get a row for the grid--array passed should be db rows with labels 'nid' and 'qty'
 */
function _uc_buyingclub_download_grid_get_nid_qty_row($nids, $label, $db_results) {
  $table_row = array($label,);
  $table_row = array_pad($table_row, count($nids) + 1, '');

  foreach ($db_results as $db_result) {
    $nid_index = array_search($db_result['nid'], $nids);
    if ($nid_index !== FALSE) {
      $table_row[$nid_index + 1] = $db_result['qty'];
    }
  }
  return $table_row;
}

function _uc_buyingclub_download_grid_get_products_overstock($ordering_period_id) {
  $query = "select bp.product_id as `nid`, bp.qty_overstock as `qty` ".
    "from {uc_buyingclub_products} bp ".
    "where bp.ordering_period_id=%d";

  $result = db_query($query, $ordering_period_id);

  while ($row = db_fetch_array($result)) {
    $overstock[] = $row;
  }
  return $overstock;
}

function _uc_buyingclub_download_grid_get_products_sold($ordering_period_id) {
  $query = "select p.nid, sum(bop.qty) as `qty` ".
    "from {node} p, {uc_buyingclub_order_products} bop, {uc_buyingclub_ordering_periods} period, {uc_orders} o, {uc_order_products} op ".
    "where (o.order_status='pending' OR o.order_status='completed') and o.created > period.start and o.created < period.end and op.order_id=o.order_id and bop.order_product_id=op.order_product_id and op.nid=p.nid and period.ordering_period_id=%d ".
    "group by p.nid";

  $result = db_query($query, $ordering_period_id);

  while ($row = db_fetch_array($result)) {
    $sold[] = $row;
  }
  return $sold;
}

function _uc_buyingclub_download_grid_get_order_products($order_id) {
  $query = "select op.nid, bop.qty ".
    "from {uc_order_products} op, {uc_buyingclub_order_products} bop ".
    "where bop.order_product_id=op.order_product_id and op.order_id=%d";

  $result = db_query($query, $order_id);

  while ($row = db_fetch_array($result)) {
    $products[] = $row;
  }
  return $products;
}

function _uc_buyingclub_download_grid_get_users($ordering_period_id) {
  $query = "select o.order_id, o.billing_last_name AS `last`, o.billing_first_name AS `first` ".
    "from {uc_orders} o, {uc_buyingclub_ordering_periods} period ".
    "where (o.order_status='pending' OR o.order_status='completed') and o.created > period.start and o.created < period.end and period.ordering_period_id=%d ".
    "order by o.billing_last_name, o.billing_first_name";

  $result = db_query($query, $ordering_period_id);

  while ($row = db_fetch_array($result)) {
    $users[] = $row;
  }
  return $users;
}

function _uc_buyingclub_download_grid_get_ordered_products($ordering_period_id) {
  $unit_db_info = content_database_info(content_fields('field_unit')); 
  $unit_table = $unit_db_info['table'];
  $unit_column = $unit_db_info['columns']['value']['column'];

  $query = "SELECT p.nid, p.title as `name`, pr.sell_price as `price`, unit_table." . $unit_column ." as `unit`, bp.qty_ordered ".
    "FROM {node} p, {uc_products} pr, {uc_buyingclub_products} bp, {". $unit_table ."} unit_table ".
    "WHERE pr.nid=p.nid AND bp.product_id=p.nid AND bp.ordering_period_id=%d AND unit_table.vid=p.vid ".
    "ORDER BY p.title";

  $result = db_query($query, $ordering_period_id);

  while ($row = db_fetch_array($result)) {
    $products[] = $row;
  }

  return $products;
}

function _uc_buyingclub_export_table_as_csv($name, $header=NULL, $table) {
  drupal_set_header('Content-Type: text/csv');
  drupal_set_header('Content-Disposition: attachment; filename='. $name);

  if ($header) {
    print implode(',', $header) ."\r\n";
  }

  foreach ($table as $row) {
    foreach ($row as $value) {
      $values[] = '"' . str_replace('"', '""', decode_entities(strip_tags($value))) . '"'; 
    }
    print implode(',', $values) ."\r\n";
    unset($values);
  }
}

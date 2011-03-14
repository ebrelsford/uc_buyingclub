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
    "AND ubp.ordering_period_id = %d AND up.nid=p.nid ".
    "GROUP BY p.nid ".
    "ORDER BY d.title, p.title";

  $result = db_query($query, $distributor_id, $ordering_period_id);

  while ($row = db_fetch_array($result)) {
    $table[] = $row;
  }

  // output as CSV
  _uc_buyingclub_export_table_as_csv('for_distributor.csv', array('item', 'bulk req', 'unit', 'case cost', 'cases ordered', 'total cost',), $table);
}

/*
 * Download a sign-in sheet that numbers orders and gives the name of the person who placed the order.
 */
function uc_buyingclub_download_order_list($ordering_period_id) {
  $query = "SELECT o.billing_first_name as first, o.billing_last_name as last ".
    "FROM {uc_orders} o, {uc_buyingclub_ordering_periods} ordering_period ".
    "WHERE o.created > ordering_period.start AND o.created < ordering_period.end ".
    "AND ordering_period.ordering_period_id=%d  AND (o.order_status = 'completed' OR o.order_status = 'pending') ".
    "ORDER BY o.billing_last_name, o.billing_first_name";

  $result = db_query($query, $ordering_period_id);
  $new_order_id = 1;
  while ($obj = db_fetch_object($result)) {
    $orders[] = array($new_order_id, $obj->last, $obj->first,);
    $new_order_id++;
  }

  _uc_buyingclub_export_table_as_csv('order_list_'. date('Ymd') .'.csv', array('order', 'last', 'first'), $orders);
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
 * Get invoices for the given ordering period as an excel file
 */
function uc_buyingclub_download_invoices($ordering_period_id) {
  require_once 'Spreadsheet/Excel/Writer.php';

  $workbook = new Spreadsheet_Excel_Writer();

  $workbook->send('buyingclub_invoices_'. date('Ymd') .'.xls');

  $header_label_format =& $workbook->addFormat(array(
    'pattern'=>1,
    'fgcolor'=>23,
    'bottom'=>1,
  ));

  $header_value_format =& $workbook->addFormat(array(
    'bottom'=>1,
    'align'=>'left',
  ));

  $details_category_style =& $workbook->addFormat(array(
    'pattern'=>1,
    'fgcolor'=>22,
  ));

  $details_item_style =& $workbook->addFormat(array(
    'bottom'=>1,
    'right'=>1,
  ));

  $details_item_money_style =& $workbook->addFormat(array(
    'bottom'=>1,
    'right'=>1,
  ));
  $details_item_money_style->setNumFormat('$#,##0.00');

  $details_total_style =& $workbook->addFormat(array(
    'pattern'=>1,
    'fgcolor'=>23,
  ));

  $details_total_money_style =& $workbook->addFormat(array(
    'pattern'=>1,
    'fgcolor'=>23,
  ));
  $details_total_money_style->setNumFormat('$#,##0.00');

  $worksheet =& $workbook->addWorksheet('invoices');

  $worksheet->setColumn(0, 0, 10010);
  $worksheet->setColumn(1, 1, 0x000D);
  $worksheet->setColumn(2, 2, 0x000D);
  $worksheet->setColumn(3, 3, 0x000D);
  $worksheet->setColumn(4, 4, 0x000D);
  $worksheet->hideGridlines();

  $row = 0;
  $new_order_id = 1;
  $breaks = array();
  foreach (_uc_buyingclub_download_invoices_get_orders($ordering_period_id) as $order) {
    $row = _uc_buyingclub_download_invoices_write_header($new_order_id, $order->first, $order->last, $order->email, $order->phone, $header_label_format, $header_value_format, &$worksheet, &$row);
    $row = _uc_buyingclub_download_invoices_write_details($order->order_id, $details_category_style, $details_item_style, $details_item_money_style, $details_total_style, $details_item_money_style, &$worksheet, &$row);

    $breaks[] = $row+1;
    $row++;
    $new_order_id++;
  }

  $worksheet->setHPagebreaks($breaks);
  $workbook->close();
}

function _uc_buyingclub_download_invoices_write_header($id, $first, $last, $email, $phone, $label_format, $value_format, &$sheet, &$row) {

  $sheet->write($row, 0, 'Greene Hill Food Co-op');
  $sheet->write($row, 1, 'Order Number', $label_format);
  $sheet->write($row, 3, $id, $value_format);
  $sheet->setMerge($row, 1, $row, 2);
  $sheet->setMerge($row, 3, $row, 4);
  $row++;

  $sheet->write($row, 0, 'Buying Club Order Confirmation');
  $sheet->write($row, 1, 'First Name', $label_format);
  $sheet->write($row, 3, $first, $value_format);
  $sheet->setMerge($row, 1, $row, 2);
  $sheet->setMerge($row, 3, $row, 4);
  $row++;

  $date_format = 'l\, M jS Y';
  $sheet->write($row, 0, date($date_format));
  $sheet->write($row, 1, 'Last Name', $label_format);
  $sheet->write($row, 3, $last, $value_format);
  $sheet->setMerge($row, 1, $row, 2);
  $sheet->setMerge($row, 3, $row, 4);
  $row++;

  $sheet->write($row, 1, 'Email Address', $label_format);
  $sheet->write($row, 3, $email, $value_format);
  $sheet->setMerge($row, 1, $row, 2);
  $sheet->setMerge($row, 3, $row, 4);
  $row++;

  $sheet->write($row, 1, 'Phone', $label_format);
  $sheet->write($row, 3, $phone, $value_format);
  $sheet->setMerge($row, 1, $row, 2);
  $sheet->setMerge($row, 3, $row, 4);
  $row++;
  $row++;

  $sheet->writeRow($row, 0, array('ITEM', 'UNIT', 'UNIT PRICE', 'QUANTITY', 'PRICE'), $label_format);

  $row++;
  return $row;
}

function _uc_buyingclub_download_invoices_write_details($order_id, $category_style, $item_style, $item_money_style, $total_style, $total_money_style, &$sheet, &$row) {
  $first_detail_row = $row;

  $current_category = '';
  foreach (_uc_buyingclub_download_invoices_get_order_details($order_id) as $details) {
    if ($details->qty == 0) continue;
    if ($details->category != $current_category) {
      $current_category = $details->category;
      $sheet->write($row, 0, $current_category, $category_style);
      $sheet->setMerge($row, 0, $row, 4);
      $row++;
    }

    $sheet->write($row, 0, $details->name, $item_style);
    $sheet->write($row, 1, $details->unit, $item_style);
    $sheet->write($row, 2, $details->price, $item_money_style);
    $sheet->write($row, 3, $details->qty, $item_style);
    $sheet->writeFormula($row, 4, '=C'. ($row+1) .'*D'. ($row+1), $item_money_style);

    $row++;
  }
  $last_detail_row = $row;

  $orig_subtotal_row = $row;

  $sheet->write($row, 2, 'Subtotal', $total_style);
  $sheet->setMerge($row, 2, $row, 3);
  $sheet->writeFormula($row, 4, '=SUM(E'. ($first_detail_row) .':E'. ($last_detail_row) .')', $total_money_style);

  $row += 2;

  $sheet->write($row, 0, 'overstock', $category_style);
  $sheet->setMerge($row, 0, $row, 4);

  $row++;

  $first_overstock_row = $row;
  for ($i = 0; $i < 6; $i++) {
    $sheet->write($row, 0, '', $item_style);
    $sheet->write($row, 1, '', $item_style);
    $sheet->write($row, 2, 0, $item_money_style);
    $sheet->write($row, 3, 0, $item_style);
    $sheet->writeFormula($row, 4, '=C'. ($row+1) .'*D'. ($row+1), $item_money_style);
    $row++;
  }
  $last_overstock_row = $row - 1;

  $overstock_subtotal_row = $row;
  $sheet->write($row, 2, 'Overstock Subtotal', $total_style);
  $sheet->setMerge($row, 2, $row, 3);
  $sheet->writeFormula($row, 4, '=SUM(E'. ($first_overstock_row + 1) .':E'. ($last_overstock_row + 1) .')', $total_money_style);
  $row++;

  $sheet->write($row, 2, 'Total', $total_style);
  $sheet->setMerge($row, 2, $row, 3);
  $sheet->writeFormula($row, 4, '=SUM(E'. ($orig_subtotal_row + 1) .';E'. ($overstock_subtotal_row + 1) .')', $total_money_style);
  $row++;

  return $row;
}

function _uc_buyingclub_download_invoices_get_order_details($order_id) {
  $query = "SELECT p.title as name, unit_table.field_unit_value as unit, bop.qty as qty, op.price as price, td.name as category ".
    "FROM {node} p, {uc_order_products} op, {uc_orders} o, {uc_buyingclub_order_products} bop, {content_field_unit} unit_table, {term_node} tn, {term_data} td  ".
    "WHERE op.nid = p.nid AND op.order_id=o.order_id AND bop.order_product_id=op.order_product_id AND o.order_id=%d AND unit_table.vid=p.vid  ".
    "AND tn.nid=p.nid AND td.tid=tn.tid ".
    "ORDER BY td.name, p.title";

  $result = db_query($query, $order_id);

  $details = array();
  while ($obj = db_fetch_object($result)) {
    $details[] = $obj;
  }
  return $details;
}

function _uc_buyingclub_download_invoices_get_orders($ordering_period_id) {
  $query = "SELECT o.order_id, o.primary_email as email, o.billing_first_name as first, o.billing_last_name as last, o.billing_phone as phone ".
    "FROM {uc_orders} o, {uc_buyingclub_ordering_periods ordering_period} ".
    "WHERE o.created > ordering_period.start AND o.created < ordering_period.end ".
    "AND ordering_period.ordering_period_id = %d AND (o.order_status='pending' OR o.order_status='completed') ".
    "ORDER BY o.billing_last_name, o.billing_first_name ";

  $result = db_query($query, $ordering_period_id);

  while ($obj = db_fetch_object($result)) {
    $orders[] = $obj;
  }
  return $orders;
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

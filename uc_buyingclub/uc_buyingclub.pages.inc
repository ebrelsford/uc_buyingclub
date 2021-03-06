<?php
/*
 * Read-only views for uc_buyingclub
 */


/*
 * view finalized orders (summary)
 */
function uc_buyingclub_view_finalized_orders() {
  // find start of most recent ordering period
  $period = _uc_buyingclub_get_most_recent_ordering_period();
  $start = $period->start;
  $end = $period->end;

  // query
  $distributor_table = _uc_buyingclub_get_distributor_table();
  $distributor_column = _uc_buyingclub_get_distributor_column();

  $minimum_order_table = _uc_buyingclub_get_minimum_order_table();
  $minimum_order_column = _uc_buyingclub_get_minimum_order_column();

  $query = "SELECT p.title AS product_name, d.title AS distributor_name, SUM(op.qty) AS qty_desired, ctd2.". $minimum_order_column ." AS minimum_order, SUM(bop.qty) AS qty_ordered ".
    "FROM {uc_orders} o, {uc_order_products} op, {node} p, {node} d, {". $distributor_table ."} ctd, {". $minimum_order_table ."} ctd2, {uc_buyingclub_order_products} bop ".
    "WHERE p.nid=op.nid AND op.order_id=o.order_id AND bop.order_product_id = op.order_product_id ".
      "AND o.created > %d AND o.created < %d ".
      "AND ctd.vid=p.vid AND ctd.". $distributor_column ."=d.nid AND ctd2.vid=p.vid ".
      "AND bop.uc_buyingclub_order_product_id = ( ".
        "SELECT bop2.uc_buyingclub_order_product_id ".
        "FROM {uc_buyingclub_order_products} bop2 ".
        "WHERE bop2.order_product_id = bop.order_product_id ".
        "ORDER BY bop2.finalized DESC ".
        "LIMIT 1 ".
      ") ".
    "GROUP BY p.nid ".
    "ORDER BY d.title, p.title ";

  $query_result = db_query($query, $start, $end);

  // build output
  $out = _uc_buyingclub_get_ordering_period_header($start, $end);

  // default output
  if (!$query_result || $query_result->num_rows === 0) {
    return $out . "No finalized orders to see for this period.";
  }

  // make table
  $data = array();

  if ($query_result) {
    while ($row = db_fetch_array($query_result)) {
      $data[] = $row;
    }
  }

  return $out . theme_table(array('Product', 'Distributor', 'Quantity Desired', 'Minimum Order', 'Quantity Ordered',), $data);
}

/*
 * Show a summary by-user of the most recent ordering period. Meant to be something that could be printed and taken to a pickup day.
 */
function uc_buyingclub_view_finalized_orders_by_user() {
  drupal_add_css(drupal_get_path('module', 'uc_buyingclub') .'/uc_buyingclub.css');

  // find start of most recent ordering period
  $period = _uc_buyingclub_get_most_recent_ordering_period();
  $start = $period->start;
  $end = $period->end;

  // query
  $distributor_table = _uc_buyingclub_get_distributor_table();
  $distributor_column = _uc_buyingclub_get_distributor_column();

  $query = "SELECT o.billing_first_name AS member_first_name, o.billing_last_name AS member_last_name, p.title AS product_name, d.title AS distributor_name, SUM(op.qty) AS qty_desired, SUM(bop.qty) AS qty_ordered, op.price AS price ".
    "FROM {uc_orders} o, {uc_order_products} op, {node} p, {node} d, {". $distributor_table ."} ctd, {uc_buyingclub_order_products} bop ".
    "WHERE p.nid=op.nid AND op.order_id=o.order_id AND bop.order_product_id = op.order_product_id ".
      "AND o.created > %d AND o.created < %d ".
      "AND ctd.vid=p.vid AND ctd.". $distributor_column ."=d.nid ".
      "AND bop.uc_buyingclub_order_product_id = ( ".
        "SELECT bop2.uc_buyingclub_order_product_id ".
        "FROM {uc_buyingclub_order_products} bop2 ".
        "WHERE bop2.order_product_id = bop.order_product_id ".
        "ORDER BY bop2.finalized DESC ".
        "LIMIT 1 ".
      ") ".
    "GROUP BY o.billing_last_name, o.billing_first_name, p.nid ".
    "ORDER BY o.billing_last_name, o.billing_first_name, d.title, p.title ";

  $query_result = db_query($query, $start, $end);

  // build output
  $out = _uc_buyingclub_get_ordering_period_header($start, $end);

  // default output
  if (!$query_result || $query_result->num_rows === 0) {
    return $out . "No finalized orders to see for this period.";
  }

  // start first table
  $current_member_name = '';
  $total_price = 0;
  $table = array();
  $table_headers = array('Product', 'Distributor', 'Quantity Desired', 'Quantity to Receive', 'Price',);

  while ($row = db_fetch_array($query_result)) {
    $member_name = $row['member_first_name'] .' '. $row['member_last_name'];
    if ($member_name !== $current_member_name) {
      // output old table
      if (count($table) > 0) {
        $table[] = array('', '', '', 'Total Due', money_format('$%.2i', $total_price));
        $out .= theme_table($table_headers, $table);
      }

      // start a new table
      $out .= '<div class="uc_buyingclub_member_name">'. $member_name .'</div>';
      $current_member_name = $member_name;
      $table = array();
      $total_price = 0;
    }

    // add this row to the table
    unset($row['member_first_name']);
    unset($row['member_last_name']);
    $price = (float)$row['price'] * (float)$row['qty_ordered'];
    $total_price += $price;
    $row['price'] = money_format('$%.2i', $price);
    $table[] = $row;
  }

  // output last table
  if (count($table) > 0) {
    $table[] = array('', '', '', 'Total Due', money_format('$%.2i', $total_price));
    $out .= theme_table($table_headers, $table);
  }

  return $out;
}

function uc_buyingclub_view_finalized_orders_by_distributor() {
  drupal_add_css(drupal_get_path('module', 'uc_buyingclub') .'/uc_buyingclub.css');

  // find start of most recent ordering period
  $period = _uc_buyingclub_get_most_recent_ordering_period();
  $ordering_period_id = $period->ordering_period_id;
  $start = $period->start;
  $end = $period->end;

  // query
  $distributor_table = _uc_buyingclub_get_distributor_table();
  $distributor_column = _uc_buyingclub_get_distributor_column();

  $query = "SELECT d.title AS distributor_name, p.title AS product_name, o.billing_first_name AS member_first_name, o.billing_last_name AS member_last_name, SUM(bop.qty) AS qty_ordered, d.nid AS distributor_id ".
    "FROM {uc_orders} o, {uc_order_products} op, {node} p, {node} d, {". $distributor_table ."} ctd, {uc_buyingclub_order_products} bop ".
    "WHERE p.nid=op.nid AND op.order_id=o.order_id AND bop.order_product_id = op.order_product_id ".
      "AND o.created > %d AND o.created < %d ".
      "AND ctd.vid=p.vid AND ctd.". $distributor_column ."=d.nid ".
      "AND bop.qty > 0 ".
      "AND bop.uc_buyingclub_order_product_id = ( ".
        "SELECT bop2.uc_buyingclub_order_product_id ".
        "FROM {uc_buyingclub_order_products} bop2 ".
        "WHERE bop2.order_product_id = bop.order_product_id ".
        "ORDER BY bop2.finalized DESC ".
        "LIMIT 1 ".
      ") ".
    "GROUP BY d.title, p.nid, o.billing_last_name, o.billing_first_name ".
    "ORDER BY d.title, p.title, o.billing_last_name, o.billing_first_name ";

  $query_result = db_query($query, $start, $end);

  // build output
  $out = _uc_buyingclub_get_ordering_period_header($start, $end);

  // default output
  if (!$query_result || $query_result->num_rows === 0) {
    return $out . "No finalized orders to see for this period.";
  }

  $current_distributor_name = '';
  $current_product_name = '';
  $total_qty = 0;
  $table = array();
  $table_headers = array('Member', 'Quantity to Receive', );

  while ($row = db_fetch_array($query_result)) {
    if ($row['product_name'] !== $current_product_name) {
      // output old table
      if (count($table) > 0) {
        $table[] = array('Total quantity', $total_qty);
        $out .= theme_table($table_headers, $table);
      }

      // might also have a new distributor
      if ($row['distributor_name'] !== $current_distributor_name) {
        $distributor_id = $row['distributor_id'];
        $out .= '<div class="uc_buyingclub_distributor_name">'. $row['distributor_name'] .'&nbsp;(';
        $out .= l('download sheet for this distributor', 'buyingclub/export/finalized/distributor/'. $ordering_period_id .'/'. $distributor_id .'/csv');
        $out .= ')</div>';
        $current_distributor_name = $row['distributor_name'];
      }

      $out .= '<div class="uc_buyingclub_product_name">'. $row['product_name'] .'</div>';
      $current_product_name = $row['product_name'];
      $table = array();
      $total_qty = 0;
    }

    $total_qty += (int)$row['qty_ordered'];
    $table[] = array($row['member_first_name'] .' '. $row['member_last_name'], $row['qty_ordered']);
  }

  // output old table
  if (count($table) > 0) {
    $table[] = array('Total quantity', $total_qty);
    $out .= theme_table($table_headers, $table);
  }

  return $out;
}

/*
 * view ordering periods
 */
function uc_buyingclub_view_ordering_periods() {
  // query
  $query = "SELECT p.start, p.end ".
    "FROM {uc_buyingclub_ordering_periods} p ".
    "WHERE 1 ".
    "ORDER BY p.start DESC ";

  $query_result = db_query($query);

  // make table
  $data = array();
  $date_format = "l\, M jS Y \a\\t ga";

  if ($query_result) {
    while ($row = db_fetch_array($query_result)) {
      $data[] = array(
        'start' => date($date_format, (int)$row['start']),
        'end' => format_date($date_format, (int)$row['end']),
      );
    }
  }

  // theme table
  return theme_table(array('Start', 'End'), $data);
}

function uc_buyingclub_view_overstock() {
  // find start of most recent ordering period
  $period = _uc_buyingclub_get_most_recent_ordering_period();
  $out = _uc_buyingclub_get_ordering_period_header($period->start, $period->end);

  $query_result = _uc_buyingclub_get_overstock($period->ordering_period_id);
  if ($query_result) {
    $overall_price = 0;
    while ($row = db_fetch_array($query_result)) {
      $total_price = $row['member price'] * $row['expected overstock'];
      $overall_price += $total_price;
      $row[] = $total_price;

      $data[] = array(
        $row['item'],
        $row['unit'],
        $row['expected overstock'],
        uc_currency_format($row['member price']),
        uc_currency_format($total_price),
      );
    }
    $data[] = array('total', '', '', '', uc_currency_format($overall_price));
  }
  else {
    return $out ."No orders or overstock to show";
  }

  // theme table
  return $out . theme_table(array('Item', 'Unit', 'Expected Overstock', 'Member Price', 'Total Price'), $data);
}

function uc_buyingclub_view_reports() {
  // find start of most recent ordering period
  $period = _uc_buyingclub_get_most_recent_ordering_period();

  $out = _uc_buyingclub_get_ordering_period_header($period->start, $period->end);

  $out .= theme_item_list(array(
      l(t('Download grid view'), 'buyingclub/export/grid/'. $period->ordering_period_id .'/csv'),
      l(t('View overstock'), 'buyingclub/view/overstock/'),
      l(t('Download overstock'), 'buyingclub/export/overstock/'. $period->ordering_period_id .'/xls'),
      l(t('View finalized orders, by member'), 'buyingclub/view/finalized/by_user'),
      l(t('View finalized orders, by distributor'), 'buyingclub/view/finalized/by_distributor'),
      l(t('Download invoices'), 'buyingclub/export/invoices/'. $period->ordering_period_id .'/xls'),
      l(t('Download order list'), 'buyingclub/export/order_list/'. $period->ordering_period_id .'/csv'),
      l(t('Download check in list'), 'buyingclub/export/check_in/'. $period->ordering_period_id .'/xls'),
      l(t('Download check out list'), 'buyingclub/export/check_out/'. $period->ordering_period_id .'/xls'),
      l(t('View price list'), 'buyingclub/view/prices'),
      l(t('Download price list'), 'buyingclub/view/prices/xls'),
      l(t('Download by-product list'), 'buyingclub/export/by_product/'. $period->ordering_period_id .'/xls'),
    ), NULL, 'ul');

  return $out;
}

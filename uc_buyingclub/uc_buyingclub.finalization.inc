<?php
/*
 * Form and logic for finalizing an ordering period.
 */


/*
 * Create the finalization table and form.
 */
function uc_buyingclub_finalization_form($rows) {
  $ordering_period = _uc_buyingclub_get_most_recent_ordering_period();
  $start = $ordering_period->start;
  $end = $ordering_period->end;

  // warn user if this ordering period has already been (perhaps partially) finalized or should not yet be finalized
  if (_uc_buyingclub_already_finalized($start, $end)) {
    drupal_set_message('Someone has already finalized this ordering period. Please contact an admin if changes need to be made.', $type='error', $repeat=FALSE);
  }
  if (time() < $end) {
    $date_format = "ga \o\\n l\, M jS Y";
    drupal_set_message('This ordering period is not over yet. Please wait until '. date($date_format, $end) .' to finalize it!', $type='error', $repeat=FALSE);
  }

  // make queries
  $query_result = _uc_buyingclub_get_orders_to_finalize($start, $end);
  $would_buy_more = _uc_buyingclub_get_would_buy_more_qty($start, $end);

  // build table/form
  $form = array();

  if ($query_result) {
    while ($row = db_fetch_object($query_result)) {
      $would_buy_more_qty = $would_buy_more[$row->product_id];
      if ($would_buy_more_qty == NULL) $would_buy_more_qty = 0;

      $ordered_by_members = $row->order_product_qty;
      $case_size = $row->minimum_order;

      // let's make a conservative suggestion of how much to order
      $case_order_suggestion = floor($ordered_by_members / $case_size);
      $unit_order_suggestion = $case_order_suggestion * $case_size;
      $expected_overstock = $unit_order_suggestion - $ordered_by_members - $would_buy_more_qty;
      if ($expected_overstock < 0) $expected_overstock = 0;

      $form[$row->product_id]['product_id'] = array('#value' => stripslashes($row->product_id));
      $form[$row->product_id]['product_name'] = array('#value' => stripslashes($row->product_name));
      $form[$row->product_id]['order_product_qty'] = array(
        '#value' => stripslashes($ordered_by_members),
      );
      $form[$row->product_id]['would_buy_more'] = array(
        '#value' => $would_buy_more_qty,
      );
      $form[$row->product_id]['distributor_name'] = array('#value' => stripslashes($row->distributor_name));
      $form[$row->product_id]['minimum_order'] = array(
        '#value' => stripslashes($case_size),
      );
      $form[$row->product_id]['current_overstock_'. $row->product_id] = array(
        '#type' => 'textfield',
        '#default_value' => '0',
        '#maxlength' => 3,
        '#size' => 3,
        '#required' => FALSE,
        '#attributes' => array('class' => 'ucBuyingClubOverstock'),
      );
      $form[$row->product_id]['ordered_'. $row->product_id] = array(
        '#type' => 'textfield',
        '#default_value' => $unit_order_suggestion,
        '#maxlength' => 3,
        '#size' => 3,
        '#required' => FALSE,
        '#attributes' => array('class' => 'ucBuyingClubUnitsToOrder'),
      );
      $form[$row->product_id]['cases_ordered_'. $row->product_id] = array(
        '#type' => 'textfield',
        '#default_value' => $case_order_suggestion,
        '#maxlength' => 3,
        '#size' => 3,
        '#required' => FALSE,
        '#disabled' => TRUE,
        '#attributes' => array('class' => 'ucBuyingClubCasesToOrder'),
      );
      $form[$row->product_id]['expected_overstock_'. $row->product_id] = array(
        '#type' => 'textfield',
        '#default_value' => $expected_overstock,
        '#maxlength' => 3,
        '#size' => 3,
        '#required' => FALSE,
        '#disabled' => TRUE,
        '#attributes' => array('class' => 'ucBuyingClubExpectedOverstock'),
      );
    }
  }

  $form['start'] = array(
    '#type' => 'hidden',
    '#value' => $start,
  );
  $form['end'] = array(
    '#type' => 'hidden',
    '#value' => $end,
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );
  $form['cancel'] = array(
    '#type' => 'markup',
    '#value' => l(t('Cancel'), 'dashboard'),
  );

  drupal_add_js(drupal_get_path('module', 'uc_buyingclub') .'/uc_buyingclub.js');
  drupal_add_css(drupal_get_path('module', 'uc_buyingclub') .'/uc_buyingclub.css');
  return $form;
}

/*
 * Theme finalization form as a table
 */
function theme_uc_buyingclub_finalization_form($form) {
  $rows = array();

  foreach (element_children($form) as $key) {
    if (isset($form[$key]['product_id'])) {
      $product_id = $form[$key]['product_id'];
      $row = array();
      $row[] = array('data' => drupal_render($form[$key]['product_name']));
      $row[] = array('data' => drupal_render($form[$key]['distributor_name']));
      $row[] = array(
        'data' => drupal_render($form[$key]['order_product_qty']),
        'class' => 'ucBuyingClubUnitsOrdered',
      );
      $row[] = array(
        'data' => drupal_render($form[$key]['would_buy_more']),
        'class' => 'ucBuyingClubWouldBuyMore',
      );
      $row[] = array(
        'data' => drupal_render($form[$key]['minimum_order']),
        'class' => 'ucBuyingClubCaseSize',
      );
      $row[] = array('data' => drupal_render($form[$key]['current_overstock_' . $key]), 'class' => 'textfield');
      $row[] = array('data' => drupal_render($form[$key]['ordered_' . $key]), 'class' => 'textfield');
      $row[] = array('data' => drupal_render($form[$key]['cases_ordered_' . $key]), 'class' => 'textfield');
      $row[] = array('data' => drupal_render($form[$key]['expected_overstock_' . $key]), 'class' => 'textfield');
      $rows[] = $row;
      $form[$key]['#printed'] = TRUE;
      $form[$key]['product_id']['#printed'] = TRUE; // avoid re-rendering these
    }
  }

  $header = array();
  $header[] = t('Product');
  $header[] = t('Distributor');
  $header[] = t('Ordered by Members');
  $header[] = t('Would Buy More');
  $header[] = t('Case Size (units)');
  $header[] = array('data' => t('Current Overstock (units)'));
  $header[] = array('data' => t('Qty to Order (units)'));
  $header[] = array('data' => t('Qty to Order (cases)'));
  $header[] = array('data' => t('Expected Overstock (units)'));

  // show which ordering period we're using
  $start = (int)$form['start']['#value'];
  $end = (int)$form['end']['#value'];
  $output = _uc_buyingclub_get_ordering_period_header($start, $end);

  $output .= theme('table', $header, $rows);
  $output .= drupal_render($form);
  return $output;
}

/*
 * validate finalization form
 */
function uc_buyingclub_finalization_form_validate($form, &$form_state) {
  foreach ($form_state['values'] as $key => $value) {
    if (stripos($key, 'ordered_') === 0) {
      if ($value < 0) {
        form_set_error('', t('Negative orders are not allowed'));
      }
      if (!is_numeric($value)) {
        form_set_error('', t('Orders must be numbers'));
      }
    }
    if (stripos($key, 'current_overstock_') === 0) {
      if ($value < 0) {
        form_set_error('', t('Negative overstocks are not allowed'));
      }
      if (!is_numeric($value)) {
        form_set_error('', t('Overstocks must be numbers'));
      }
    }
  }
}

/*
 * submit finalization form
 */
function uc_buyingclub_finalization_form_submit($form_id, $form) {
  $form_values = $form['values'];
  $overstock_prefix = 'current_overstock_';
  $ordered_prefix = 'ordered_';
  $start = $form_values['start'];
  $end = $form_values['end'];

  $would_buy_more = _uc_buyingclub_get_would_buy_more_qty($start, $end);

  // finalize each product submitted with the quantity ordered as indicated
  foreach ($form_values as $key => $value) {
    if (stripos($key, $ordered_prefix) === 0) {
      $product_id = substr($key, strlen($ordered_prefix));
      $qty_ordered = $value;
      $qty_overstock = $form_values[$overstock_prefix . strval($product_id)];
      $qty_would_buy_more = $would_buy_more[$product_id];

      uc_buyingclub_finalize_product_order($product_id, $qty_would_buy_more, $qty_ordered, $qty_overstock, $start, $end, 'ordered');
    }
  }

  // finalize each the total order for each user who ordered in this period
  uc_buyingclub_finalize_users($start, $end);

  drupal_set_message(t('Ordered the given amounts for each product and notified purchasers'), $type='status', $repeat=FALSE);
}

/*
 * Finalize all orders for a given product by recording the quantity allocated.
 *
 * @param $product_id
 *    The nid of the product node
 * @param $qty_ordered
 *    The quantity ordered through the finalization form
 * @param $qty_old_overstock
 *    The quantity that is currently at hand--should be given to members but NOT ordered from distributors
 * @param $start
 *    The start of the ordering period (in seconds)
 * @param $end
 *    The end of the ordering period (in seconds)
 */
function uc_buyingclub_finalize_product_order($product_id, $qty_would_buy_more, $qty_ordered, $qty_old_overstock, $start, $end, $status='ordered') {
  /*
   * first, give everyone who ordered this product their share
   */
  $query = "SELECT op.order_product_id AS id, op.qty AS qty_desired ".
    "FROM {uc_order_products} op, {uc_orders} o ".
    "WHERE op.order_id = o.order_id AND op.nid = %d ".
    "AND o.created > %d AND o.created < %d AND o.order_status='pending' ".
    "ORDER BY o.created";

  $query_result = db_query($query, $product_id, $start, $end);

  $qty_available = $qty_ordered + $qty_old_overstock;
  $order_product_qty_given = array();
  if ($query_result) {
    while ($row = db_fetch_object($query_result)) {
      // determine how much of this order for this product will be fulfilled
      $qty_desired = $row->qty_desired;
      $qty_given = 0;

      if ($qty_available > 0) {
        if ($qty_desired <= $qty_available) {
          $qty_given = $qty_desired;
        }
        else { // give as much as we can
          $qty_given = $qty_available;
        }
      }

      // save
      $order_product_qty_given[$row->id] = $qty_given;
      $to_write = array(
        'order_product_id' => $row->id,
        'qty' => $qty_given,
        'finalized' => time(),
        'status' => $status,
      );
      drupal_write_record('uc_buyingclub_order_products', $to_write);
      
      $qty_available -= $qty_given;
    }
  }

  /*
   * now, if there is anything left, give it first to the people who said they would take more
   */
  if ($qty_available > 0 && $qty_would_buy_more > 0) {
    // increment amount to give for each person who said they'd take more
    $query = "SELECT op.order_product_id AS order_product_id ".
      "FROM {uc_orders} o, {uc_order_products} op ".
      "WHERE op.order_id=o.order_id ".
        "AND o.created > %d AND o.created < %d AND o.order_status='pending' ".
        "AND op.nid=%d ".
        "AND op.data LIKE '%%%s%%' ";

    $query_result = db_query($query, $start, $end, $product_id, UC_BUYINGCLUB_WOULD_BUY_MORE_LABEL);
    if ($query_result) {
      while (($row = db_fetch_object($query_result)) && $qty_available > 0) {
        // find out how much the order-product got and increment
        $to_write = array(
          'order_product_id' => $row->order_product_id,
          'qty' => $order_product_qty_given[$row->order_product_id] + 1,
          'finalized' => time(),
          'status' => $status,
        );
        
        if (drupal_write_record('uc_buyingclub_order_products', $to_write, 'order_product_id') != FALSE ) {
          $qty_available--;
        }
      }
    }
  }

  /*
   * finalize the product as a whole for this ordering period.
   * record overstock, if any.
   */
  $ordering_period = _uc_buyingclub_get_ordering_period($start, $end);
  $to_write = array(
    'product_id' => $product_id,
    'qty_ordered' => $qty_ordered,
    'qty_old_overstock' => $qty_old_overstock,
    'qty_overstock' => $qty_available,
    'ordering_period_id' => $ordering_period->ordering_period_id,
    'finalized' => time(),
    'status' => $status,
  );
  drupal_write_record('uc_buyingclub_products', $to_write);
}

/*
 * Finalize orders for all users in the given ordering period.
 *
 * @param $start
 *    The start of the ordering period (in seconds)
 * @param $end
 *    The end of the ordering period (in seconds)
 */
function uc_buyingclub_finalize_users($start, $end) {
  $query = "SELECT DISTINCT o.uid AS user_id ".
    "FROM {uc_orders} o ".
    "WHERE o.created > %d AND o.created < %d AND (o.order_status='pending' OR o.order_status='completed') ";

  $query_result = db_query($query, $start, $end);

  if ($query_result) {
    while ($row = db_fetch_object($query_result)) {
      uc_buyingclub_finalize_user($row->user_id, $start, $end);
    }
  }
}

/*
 * Finalize orders for a given user in the given ordering period.
 *
 * @param $user_id
 *    The id of the user
 * @param $start
 *    The start of the ordering period (in seconds)
 * @param $end
 *    The end of the ordering period (in seconds)
 */
function uc_buyingclub_finalize_user($user_id, $start, $end) {
  $query = "SELECT op.title AS title, op.qty AS qty_desired, bop.qty AS qty_given, op.price AS price, o.billing_first_name as first_name ".
    "FROM {uc_orders} o, {uc_order_products} op, {uc_buyingclub_order_products} bop ".
    "WHERE o.uid=%d AND o.created > %d AND o.created < %d AND o.order_status='pending' ".
      "AND op.order_id = o.order_id AND bop.order_product_id = op.order_product_id ".
      "AND bop.uc_buyingclub_order_product_id = ( ".
        "SELECT bop2.uc_buyingclub_order_product_id ".
        "FROM {uc_buyingclub_order_products} bop2 ".
        "WHERE bop2.order_product_id = bop.order_product_id ".
        "ORDER BY bop2.finalized DESC ".
        "LIMIT 1 ".
      ") ";

  $query_result = db_query($query, $user_id, $start, $end);

  $total_cost = 0;
  $products_given = array();
  $products_desired = array();
  $products_cost = array();
  $first_name = '';

  if ($query_result) {
    while ($row = db_fetch_object($query_result)) {
      // get first name if we don't already have it
      if ($first_name == '') {
        $first_name = $row->first_name;
      }

      // assemble summary, new price
      $price = (float)$row->price;
      $qty_given = (int)$row->qty_given;
      $qty_desired = (int)$row->qty_desired;

      $cost = $price * $qty_given;
      $total_cost += $cost;

      if (!$products_given[$row->title]) $products_given[$row->title] = 0;
      $products_given[$row->title] += $qty_given;

      if (!$products_desired[$row->title]) $products_desired[$row->title] = 0;
      $products_desired[$row->title] += $qty_desired;

      if (!$products_cost[$row->title]) $products_cost[$row->title] = 0;
      $products_cost[$row->title] += $cost;
    }

    $buyingclub_values = uc_buyingclub_token_values('order');
    $params = array(
      'first_name' => $first_name,
      'total_cost' => $total_cost,
      'products_given' => $products_given,
      'products_desired' => $products_desired,
      'products_cost' => $products_cost,
      'pickup_date' => $buyingclub_values['order-pickup-date'],
    );

    $account = user_load($user_id);
    $store_values = uc_store_token_values('global');
    job_queue_add('drupal_mail', 'Finalization mail', array('uc_buyingclub', 'final_order', $account->mail, user_preferred_language($account), $params, $store_values['store-email']), '', TRUE);
  }
}

/*
 * Create finalization table/form after items are received
 */
function uc_buyingclub_received_items_form($rows) {
  $ordering_period = _uc_buyingclub_get_most_recent_ordering_period();
  $start = $ordering_period->start;
  $end = $ordering_period->end;

  // make queries
  $query_result = _uc_buyingclub_received_items_get_orders_to_finalize($ordering_period->ordering_period_id);
  $would_buy_more = _uc_buyingclub_get_would_buy_more_qty($start, $end);

  // build table/form
  $form = array();

  if ($query_result) {
    while ($product = db_fetch_object($query_result)) {
      $id = $product->product_id;
      $would_buy_more_qty = $would_buy_more[$id];
      if ($would_buy_more_qty == NULL) $would_buy_more_qty = 0;

      $ordered_by_members = $product->order_product_qty;
      $case_size = $product->minimum_order;

      // what do we already know about the quantities?
      $unit_order_expected = $product->qty_ordered;
      $case_order_expected = $unit_order_expected / $case_size;
      $expected_overstock = $product->qty_overstock;

      // pack up the form
      $form[$id]['product_id'] = array('#value' => stripslashes($id));
      $form[$id]['product_name'] = array('#value' => stripslashes($product->product_name));
      $form[$id]['order_product_qty'] = array('#value' => stripslashes($ordered_by_members));
      $form[$id]['would_buy_more'] = array('#value' => $would_buy_more_qty);
      $form[$id]['distributor_name'] = array('#value' => stripslashes($product->distributor_name));
      $form[$id]['minimum_order'] = array('#value' => stripslashes($case_size));
      $form[$id]['case_size_'. $id] = array(
        '#type' => 'hidden',
        '#value' => $case_size,
      );
      $form[$id]['current_overstock_'. $id] = array(
        '#type' => 'textfield',
        '#default_value' => '0',
        '#maxlength' => 3,
        '#size' => 3,
        '#required' => FALSE,
        '#attributes' => array('class' => 'ucBuyingClubOverstock'),
      );
      $form[$id]['units_received_'. $id] = array(
        '#type' => 'textfield',
        '#default_value' => $unit_order_expected,
        '#maxlength' => 3,
        '#size' => 3,
        '#required' => FALSE,
        '#attributes' => array('class' => 'ucBuyingClubUnitsToOrder'),
      );
      $form[$id]['cases_received_'. $id] = array(
        '#type' => 'textfield',
        '#default_value' => $case_order_expected,
        '#maxlength' => 3,
        '#size' => 3,
        '#required' => FALSE,
        '#disabled' => TRUE,
        '#attributes' => array('class' => 'ucBuyingClubCasesToOrder'),
      );
      $form[$id]['overstock_units_received_'. $id] = array(
        '#type' => 'textfield',
        '#default_value' => $expected_overstock,
        '#maxlength' => 3,
        '#size' => 3,
        '#required' => FALSE,
        '#disabled' => TRUE,
        '#attributes' => array('class' => 'ucBuyingClubExpectedOverstock'),
      );
      $case_cost = round($product->cost * $case_size, 3);
      $form[$id]['case_cost'] = array('#value' => uc_currency_format($case_cost));
      $form[$id]['case_cost_'. $id] = array(
        '#type' => 'hidden',
        '#value' => $case_cost,
      );
      $form[$id]['updated_case_cost_'. $id] = array(
        '#type' => 'textfield',
        '#default_value' => $case_cost,
        '#maxlength' => 5,
        '#size' => 5,
        '#required' => FALSE,
        '#field_prefix' => '$',
      );

    }
  }

  $form['ordering_period_id'] = array(
    '#type' => 'hidden',
    '#value' => $ordering_period->ordering_period_id,
  );
  $form['start'] = array(
    '#type' => 'hidden',
    '#value' => $start,
  );
  $form['end'] = array(
    '#type' => 'hidden',
    '#value' => $end,
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );
  $form['cancel'] = array(
    '#type' => 'markup',
    '#value' => l(t('Cancel'), 'dashboard'),
  );

  drupal_add_js(drupal_get_path('module', 'uc_buyingclub') .'/uc_buyingclub.js');
  drupal_add_css(drupal_get_path('module', 'uc_buyingclub') .'/uc_buyingclub.css');
  return $form;
}

/*
 * Theme received items form
 */
function theme_uc_buyingclub_received_items_form($form) {
  $rows = array();

  foreach (element_children($form) as $key) {
    if (isset($form[$key]['product_id'])) {
      $product_id = $form[$key]['product_id'];
      $row = array();
      $row[] = array('data' => drupal_render($form[$key]['product_name']));
      $row[] = array('data' => drupal_render($form[$key]['distributor_name']));
      $row[] = array('data' => drupal_render($form[$key]['order_product_qty']), 'class' => 'ucBuyingClubUnitsOrdered');
      $row[] = array('data' => drupal_render($form[$key]['would_buy_more']), 'class' => 'ucBuyingClubWouldBuyMore');
      $row[] = array('data' => drupal_render($form[$key]['minimum_order']), 'class' => 'ucBuyingClubCaseSize');
      $row[] = array('data' => drupal_render($form[$key]['current_overstock_' . $key]), 'class' => 'textfield');
      $row[] = array('data' => drupal_render($form[$key]['units_received_' . $key]), 'class' => 'textfield');
      $row[] = array('data' => drupal_render($form[$key]['cases_received_' . $key]), 'class' => 'textfield');
      $row[] = array('data' => drupal_render($form[$key]['overstock_units_received_' . $key]), 'class' => 'textfield');
      $row[] = array('data' => drupal_render($form[$key]['case_cost']));
      $row[] = array('data' => drupal_render($form[$key]['updated_case_cost_' . $key]), 'class' => 'textfield');
      $rows[] = $row;
      $form[$key]['#printed'] = TRUE;
      $form[$key]['product_id']['#printed'] = TRUE; // avoid re-rendering these
    }
  }

  $header = array(
    t('Product'),
    t('Distributor'),
    t('Qty Ordered (units)'),
    t('Would Buy More'),
    t('Case Size (units)'),
    t('Current Overstock (units)'),
    t('Qty Received (units)'),
    t('Qty Received (cases)'),
    t('Expected Overstock (units)'),
    t('Cost (case)'),
    t('Updated Cost (case)'),
  );

  // show which ordering period we're using
  $start = (int)$form['start']['#value'];
  $end = (int)$form['end']['#value'];
  $output = _uc_buyingclub_get_ordering_period_header($start, $end);

  $output .= theme('table', $header, $rows);
  $output .= drupal_render($form);
  return $output;
}

function _uc_buyingclub_received_items_get_orders_to_finalize($ordering_period_id) {
  $distributor_table = _uc_buyingclub_get_distributor_table();
  $distributor_column = _uc_buyingclub_get_distributor_column();

  $minimum_order_table = _uc_buyingclub_get_minimum_order_table();
  $minimum_order_column = _uc_buyingclub_get_minimum_order_column();

  $query = "SELECT p.title AS product_name, p.nid AS product_id, SUM(op.qty) AS order_product_qty, d.title AS distributor_name, ctd2.". $minimum_order_column ." AS minimum_order, pr.cost, bp.qty_ordered AS qty_ordered, bp.qty_overstock AS qty_overstock ".
    "FROM {uc_orders} o, {uc_order_products} op, {node} p, {node} d, {". $distributor_table ."} ctd, {". $minimum_order_table ."} ctd2, {uc_products} pr, {uc_buyingclub_products} bp, {uc_buyingclub_ordering_periods} period ".
    "WHERE op.order_id=o.order_id AND p.nid=op.nid ".
      // products in this ordering period
      "AND period.ordering_period_id=%d ".
      "AND o.created > period.start AND o.created < period.end AND (o.order_status='pending' OR o.order_status='completed') ".

      // get associated CCK fields
      "AND ctd.vid=p.vid AND ctd.". $distributor_column ."=d.nid ".
      "AND ctd2.vid=p.vid ".

      // and show the most recent quantities from uc_buyingclub_products
      "AND pr.nid=p.nid ".
      "AND bp.ordering_period_id=period.ordering_period_id AND bp.product_id=pr.nid AND bp.uc_buyingclub_product_id = ( ".
        "SELECT bp2.uc_buyingclub_product_id ".
        "FROM uc_buyingclub_products bp2 ".
        "WHERE bp2.ordering_period_id=bp.ordering_period_id AND bp2.product_id=bp.product_id ".
        "ORDER BY finalized DESC ".
        "LIMIT 1 ".
      ") ".
    "GROUP BY p.nid ".
    "ORDER BY d.title, p.title ";

  return db_query($query, $ordering_period_id);
}

/*
 * submit received items form
 */
function uc_buyingclub_received_items_form_submit($form_id, $form) {

  $form_values = $form['values'];
  $start = $form_values['start'];
  $end = $form_values['end'];

  $received_prefix = 'units_received_';
  $overstock_prefix = 'current_overstock_';

  $case_cost_prefix = 'case_cost_';
  $updated_case_cost_prefix = 'updated_case_cost_';
  $case_size_prefix = 'case_size_';

  $would_buy_more = _uc_buyingclub_get_would_buy_more_qty($start, $end);

  // update received quantities for each product submitted
  foreach ($form_values as $key => $value) {
    if (stripos($key, $received_prefix) === 0) {
      $product_id = substr($key, strlen($received_prefix));
      $qty_received = $value;
      $qty_overstock = $form_values[$overstock_prefix . strval($product_id)];
      $qty_would_buy_more = $would_buy_more[$product_id];

      uc_buyingclub_finalize_product_order($product_id, $qty_would_buy_more, $qty_received, $qty_overstock, $start, $end, 'received');

      $expected_case_cost = $form_values[$case_cost_prefix . strval($product_id)];
      $updated_case_cost = $form_values[$updated_case_cost_prefix . strval($product_id)];
      $case_size = $form_values[$case_size_prefix . strval($product_id)];
      uc_buyingclub_received_items_update_price($form_values['ordering_period_id'], $product_id, $case_size, $expected_case_cost, $updated_case_cost);
    }
  }

  /*
   * Update prices of items.
   */

  drupal_set_message(t('Updated the amounts of each product given to members'), $type='status', $repeat=FALSE);
}

function uc_buyingclub_received_items_update_price($ordering_period_id, $product_id, $case_size, $expected_case_cost, $updated_case_cost) {
  if ($expected_case_cost === $updated_case_cost || $updated_case_cost === '') return;

  $cost = $updated_case_cost / $case_size;
  $price = 1.2 * $cost;

  // update uc_order_products
  $query = 
    "UPDATE uc_orders o, uc_order_products op, uc_buyingclub_order_products bop, uc_buyingclub_ordering_periods period ".
    "SET op.cost=%f, op.price=%f ".
    "WHERE bop.order_product_id=op.order_product_id AND op.order_id=o.order_id ".
    "AND o.created > period.start AND o.created < period.end AND period.ordering_period_id=%d  ".
    "AND op.nid=%d";
  db_query($query, $cost, $price, $ordering_period_id, $product_id);

  // update uc_products
  $uc_products_update = array(
    'nid' => $product_id,
    'list_price' => $price,
    'sell_price' => $price,
    'cost' => $cost,
  );
  drupal_write_record('uc_products', $uc_products_update, 'nid');

}

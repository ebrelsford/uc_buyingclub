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
    drupal_set_message('This ordering period is not over yet. Please wait until '. format_date($end, $type='custom', $format=$date_format) .' to finalize it!', $type='error', $repeat=FALSE);
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

      $form[$row->product_id]['product_id'] = array('#value' => stripslashes($row->product_id));
      $form[$row->product_id]['product_name'] = array('#value' => stripslashes($row->product_name));
      $form[$row->product_id]['order_product_qty'] = array('#value' => stripslashes($row->order_product_qty));
      $form[$row->product_id]['would_buy_more'] = array('#value' => $would_buy_more_qty);
      $form[$row->product_id]['distributor_name'] = array('#value' => stripslashes($row->distributor_name));
      $form[$row->product_id]['minimum_order'] = array('#value' => stripslashes($row->minimum_order));
      $form[$row->product_id]['ordered_'. $row->product_id] = array(
        '#type' => 'textfield',
        '#default_value' => '0',
        '#maxlength' => 3,
        '#size' => 3,
        '#required' => FALSE,
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
      $row[] = array('data' => drupal_render($form[$key]['order_product_qty']));
      $row[] = array('data' => drupal_render($form[$key]['would_buy_more']));
      $row[] = array('data' => drupal_render($form[$key]['minimum_order']));
      $row[] = array('data' => drupal_render($form[$key]['ordered_' . $key]), 'class' => 'textfield');
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
  $header[] = t('Minimum Order');
  $header[] = array('data' => t('Qty to Order'));

  // show which ordering period we're using
  $start = (int)$form['start']['#value'];
  $end = (int)$form['end']['#value'];
  $date_format = "ga l\, M jS Y";
  $output = 'Orders placed from '. format_date($start, $type='custom', $format=$date_format) .' to '. format_date($end, $type='custom', $format=$date_format);

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
  }
}

/*
 * submit finalization form
 */
function uc_buyingclub_finalization_form_submit($form_id, $form) {
  $form_values = $form['values'];
  $prefix = 'ordered_';
  $start = $form_values['start'];
  $end = $form_values['end'];

  $would_buy_more = _uc_buyingclub_get_would_buy_more_qty($start, $end);

  // finalize each product submitted with the quantity ordered as indicated
  foreach ($form_values as $key => $value) {
    if (stripos($key, $prefix) === 0) {
      $product_id = substr($key, strlen($prefix));
      $qty_ordered = $value;
      $qty_would_buy_more = $would_buy_more[$product_id];

      uc_buyingclub_finalize_product_order($product_id, $qty_would_buy_more, $qty_ordered, $start, $end);
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
 * @param $start
 *    The start of the ordering period (in seconds)
 * @param $end
 *    The end of the ordering period (in seconds)
 */
function uc_buyingclub_finalize_product_order($product_id, $qty_would_buy_more, $qty_ordered, $start, $end) {
  /*
   * first, give everyone who ordered this product their share
   */
  $query = "SELECT op.order_product_id AS id, op.qty AS qty_desired ".
    "FROM {uc_order_products} op, {uc_orders} o ".
    "WHERE op.order_id = o.order_id AND op.nid = %d ".
    "AND o.created > %d AND o.created < %d AND o.order_status='pending' ".
    "ORDER BY o.created";

  $query_result = db_query($query, $product_id, $start, $end);

  $qty_available = $qty_ordered;
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
    'qty_overstock' => $qty_available,
    'ordering_period_id' => $ordering_period->ordering_period_id,
    'finalized' => time(),
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
    "WHERE o.created > %d AND o.created < %d AND o.order_status='pending' ";

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
      "AND op.order_id = o.order_id AND bop.order_product_id = op.order_product_id";

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

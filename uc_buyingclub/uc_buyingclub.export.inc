<?php

function uc_buyingclub_download_check_in_list($ordering_period_id) {
  require_once 'PHPExcel.php';
  require_once 'PHPExcel/Writer/Excel2007.php';

  $objPHPExcel = new PHPExcel();

  // Set properties
  $objPHPExcel->getProperties()->setTitle("Check-in List");
  $objPHPExcel->getProperties()->setSubject("Check-in List");
  $objPHPExcel->getProperties()->setDescription("Check-in List");


  // Add some data
  $objPHPExcel->setActiveSheetIndex(0);

  $query = "SELECT o.billing_first_name as first, o.billing_last_name as last ".
    "FROM {uc_orders} o, {uc_buyingclub_ordering_periods} ordering_period ".
    "WHERE o.created > ordering_period.start AND o.created < ordering_period.end ".
    "AND ordering_period.ordering_period_id=%d  AND (o.order_status = 'completed' OR o.order_status = 'pending') ".
    "ORDER BY o.billing_last_name, o.billing_first_name";

  $result = db_query($query, $ordering_period_id);
  $new_order_id = 1;
  $row = 1;

  $sheet = $objPHPExcel->getActiveSheet();

  $sheet->getColumnDimension('A')->setAutoSize(true);
  $sheet->getColumnDimension('B')->setAutoSize(true);
  $sheet->getColumnDimension('C')->setAutoSize(true);
  $sheet->getColumnDimension('D')->setAutoSize(true);
  
  $date = gmmktime(0, 0, 0, gmdate('n'), gmdate('j'), gmdate('Y'));
  $sheet->setCellValueByColumnAndRow(3, $row, PHPExcel_Shared_Date::PHPToExcel($date));
  $sheet->getStyle('D1')->getNumberFormat()->setFormatCode(PHPExcel_Style_NumberFormat::FORMAT_DATE_XLSX14);
  $row++;

  $sheet->setCellValueByColumnAndRow(0, $row, 'Order #');
  $sheet->setCellValueByColumnAndRow(1, $row, 'Last');
  $sheet->setCellValueByColumnAndRow(2, $row, 'First');
  $sheet->setCellValueByColumnAndRow(3, $row, 'Checked in?');
  $sheet->getStyle('A'.$row.':D'.$row)->applyFromArray(array(
    'font' => array(
      'bold' => true, 
      'color' => array('rgb' => 'FFFFFF'),
    ),
    'fill' => array(
      'type' => PHPExcel_Style_Fill::FILL_SOLID, 
      'color' => array('rgb' => '000000'),
    ),
  ));
  $row++;

  while ($obj = db_fetch_object($result)) {
    $sheet->setCellValueByColumnAndRow(0, $row, $new_order_id);
    $sheet->setCellValueByColumnAndRow(1, $row, $obj->last);
    $sheet->setCellValueByColumnAndRow(2, $row, $obj->first);
    $new_order_id++;
    $row++;
  }

  // Rename sheet
  $sheet->setTitle('Order List');

  _uc_buyingclub_set_excel_headers('check in ' . date('Ymd') . '.xlsx');
      
  // Save Excel 2007 file
  $objWriter = new PHPExcel_Writer_Excel2007($objPHPExcel);
  $objWriter->save('php://output');

  $objPHPExcel->disconnectWorksheets();
  unset($objPHPExcel);
}

function _uc_buyingclub_download_check_out_list_get_subtotal($order_id) {
  $query = "SELECT bop.qty * op.price as price ".
    "FROM {uc_order_products} op, {uc_buyingclub_order_products} bop ".
    "WHERE op.order_id=%d AND bop.order_product_id=op.order_product_id ".

      // make sure we get the most recent uc_buyingclub_order_product
      "AND bop.uc_buyingclub_order_product_id = ( ".
        "SELECT bop2.uc_buyingclub_order_product_id ".
        "FROM {uc_buyingclub_order_products} bop2 ".
        "WHERE bop2.order_product_id = bop.order_product_id ".
        "ORDER BY bop2.finalized DESC ".
        "LIMIT 1 ".
      ") ";
  $result = db_query($query, $order_id);

  $sum = 0;
  while ($arr = db_fetch_array($result)) {
    $sum += $arr['price'];
  }

  return $sum;
}

function uc_buyingclub_download_check_out_list($ordering_period_id) {
  require_once 'PHPExcel.php';
  require_once 'PHPExcel/Writer/Excel2007.php';

  $objPHPExcel = new PHPExcel();

  // Set properties
  $objPHPExcel->getProperties()->setTitle("Check-out List");
  $objPHPExcel->getProperties()->setSubject("Check-out List");
  $objPHPExcel->getProperties()->setDescription("Check-out List");


  // Add some data
  $objPHPExcel->setActiveSheetIndex(0);

  $query = "SELECT o.order_id as order_id, o.billing_first_name as first, o.billing_last_name as last ".
    "FROM {uc_orders} o, {uc_buyingclub_ordering_periods} ordering_period ".
    "WHERE o.created > ordering_period.start AND o.created < ordering_period.end ".
    "AND ordering_period.ordering_period_id=%d  AND (o.order_status = 'completed' OR o.order_status = 'pending') ".
    "ORDER BY o.billing_last_name, o.billing_first_name";

  $result = db_query($query, $ordering_period_id);
  $new_order_id = 1;
  $row = 1;

  $sheet = $objPHPExcel->getActiveSheet();

  $sheet->getColumnDimension('A')->setAutoSize(true);
  $sheet->getColumnDimension('B')->setAutoSize(true);
  $sheet->getColumnDimension('C')->setAutoSize(true);
  $sheet->getColumnDimension('D')->setAutoSize(true);
  $sheet->getColumnDimension('E')->setAutoSize(true);
  $sheet->getColumnDimension('F')->setAutoSize(true);
  $sheet->getColumnDimension('G')->setAutoSize(true);
  $sheet->getColumnDimension('H')->setAutoSize(true);
  
  $date = gmmktime(0, 0, 0, gmdate('n'), gmdate('j'), gmdate('Y'));
  $sheet->setCellValueByColumnAndRow(3, $row, PHPExcel_Shared_Date::PHPToExcel($date));
  $sheet->getStyle('D1')->getNumberFormat()->setFormatCode(PHPExcel_Style_NumberFormat::FORMAT_DATE_XLSX14);
  $row++;

  $sheet->setCellValueByColumnAndRow(0, $row, 'Order #');
  $sheet->setCellValueByColumnAndRow(1, $row, 'Last');
  $sheet->setCellValueByColumnAndRow(2, $row, 'First');
  $sheet->setCellValueByColumnAndRow(3, $row, 'Subtotal');
  $sheet->setCellValueByColumnAndRow(4, $row, 'Overstock Purchase');
  $sheet->setCellValueByColumnAndRow(5, $row, 'Grand Total');
  $sheet->setCellValueByColumnAndRow(6, $row, 'Check #');
  $sheet->setCellValueByColumnAndRow(7, $row, 'Name on Check');
  $sheet->getStyle('A'.$row.':H'.$row)->applyFromArray(array(
    'font' => array(
      'bold' => true, 
      'color' => array('rgb' => 'FFFFFF'),
    ),
    'fill' => array(
      'type' => PHPExcel_Style_Fill::FILL_SOLID, 
      'color' => array('rgb' => '000000'),
    ),
  ));
  $row++;

  $first_user_row = $row;
  while ($obj = db_fetch_object($result)) {
    $sheet->setCellValueByColumnAndRow(0, $row, $new_order_id);
    $sheet->setCellValueByColumnAndRow(1, $row, $obj->last);
    $sheet->setCellValueByColumnAndRow(2, $row, $obj->first);
    $sheet->setCellValueByColumnAndRow(3, $row, _uc_buyingclub_download_check_out_list_get_subtotal($obj->order_id));
    $sheet->setCellValueByColumnAndRow(4, $row, 0);
    $sheet->setCellValueByColumnAndRow(5, $row, '=D'.$row.'+E'.$row);
    $sheet->setCellValueByColumnAndRow(7, $row, $obj->first .' '. $obj->last);
    $new_order_id++;
    $row++;
  }
  $sheet->getStyle('D'.$first_user_row.':F'.$row)->getNumberFormat()->setFormatCode('$#,##0.00');

  // Rename sheet
  $sheet->setTitle('Order List');

  _uc_buyingclub_set_excel_headers('check out ' . date('Ymd') . '.xlsx');
      
  // Save Excel 2007 file
  $objWriter = new PHPExcel_Writer_Excel2007($objPHPExcel);
  $objWriter->save('php://output');

  $objPHPExcel->disconnectWorksheets();
  unset($objPHPExcel);
}

function uc_buyingclub_download_overstock($ordering_period_id) {
  require_once 'PHPExcel.php';
  require_once 'PHPExcel/Writer/Excel2007.php';

  $objPHPExcel = new PHPExcel();

  // Set properties
  $objPHPExcel->getProperties()->setTitle("Overstock");
  $objPHPExcel->getProperties()->setSubject("Overstock");
  $objPHPExcel->getProperties()->setDescription("Overstock");

  // Add some data
  $objPHPExcel->setActiveSheetIndex(0);

  $result = _uc_buyingclub_get_overstock($ordering_period_id);
  $row = 1;

  $sheet = $objPHPExcel->getActiveSheet();

  $sheet->getColumnDimension('A')->setAutoSize(true);
  $sheet->getColumnDimension('B')->setAutoSize(true);
  $sheet->getColumnDimension('C')->setAutoSize(true);
  $sheet->getColumnDimension('D')->setAutoSize(true);
  $sheet->getColumnDimension('E')->setAutoSize(true);
  
  $sheet->setCellValueByColumnAndRow(0, $row, 'Item');
  $sheet->setCellValueByColumnAndRow(1, $row, 'Unit');
  $sheet->setCellValueByColumnAndRow(2, $row, 'Expected Overstock');
  $sheet->setCellValueByColumnAndRow(3, $row, 'Member Price');
  $sheet->setCellValueByColumnAndRow(4, $row, 'Total Price');
  $sheet->getStyle('A'.$row.':E'.$row)->applyFromArray(array(
    'font' => array(
      'bold' => true, 
      'color' => array('rgb' => 'FFFFFF'),
    ),
    'fill' => array(
      'type' => PHPExcel_Style_Fill::FILL_SOLID, 
      'color' => array('rgb' => '000000'),
    ),
  ));
  $row++;

  $first_item_row = $row;
  while ($arr = db_fetch_array($result)) {
    $sheet->setCellValueByColumnAndRow(0, $row, $arr['item']);
    $sheet->setCellValueByColumnAndRow(1, $row, $arr['unit']);
    $sheet->setCellValueByColumnAndRow(2, $row, $arr['expected overstock']);
    $sheet->setCellValueByColumnAndRow(3, $row, $arr['member price']);
    $sheet->setCellValueByColumnAndRow(4, $row, '=C'.$row.'*D'.$row);
    $row++;
  }
  $sheet->getStyle('D'.$first_item_row.':E'.$row)->getNumberFormat()->setFormatCode('$#,##0.00');

  $sheet->setCellValueByColumnAndRow(0, $row, 'total');
  $sheet->setCellValueByColumnAndRow(4, $row, '=SUM(E'. $first_item_row .':E'. ($row-1) .')');

  // Rename sheet
  $sheet->setTitle('Overstock');

  _uc_buyingclub_set_excel_headers('overstock ' . date('Ymd') . '.xlsx');
      
  // Save Excel 2007 file
  $objWriter = new PHPExcel_Writer_Excel2007($objPHPExcel);
  $objWriter->save('php://output');

  $objPHPExcel->disconnectWorksheets();
  unset($objPHPExcel);
}

function uc_buyingclub_download_by_product($ordering_period_id) {
  require_once 'PHPExcel.php';
  require_once 'PHPExcel/Writer/Excel2007.php';

  $objPHPExcel = new PHPExcel();

  // Set properties
  $objPHPExcel->getProperties()->setTitle("By-product");
  $objPHPExcel->getProperties()->setSubject("By-product");
  $objPHPExcel->getProperties()->setDescription("By-product");

  // Add some data
  $objPHPExcel->setActiveSheetIndex(0);

  $result = _uc_buyingclub_get_overstock($ordering_period_id);

  $new_order_ids = _uc_buyingclub_download_by_product_get_new_order_ids($ordering_period_id);


  $sheet = $objPHPExcel->getActiveSheet();

  $sheet->getColumnDimension('A')->setAutoSize(true);
  $sheet->getColumnDimension('B')->setAutoSize(true);
  $sheet->getColumnDimension('C')->setAutoSize(true);

  $product_start_rows = array();
  $row = 1;

  $products = _uc_buyingclub_download_by_product_get_products_ordered($ordering_period_id);
  foreach ($products as $product) {
    $sheet->setCellValueByColumnAndRow(0, $row, $product->name ." (". $product->unit .")");
    $sheet->mergeCells('A'. $row .':C'. $row);
    $row += 2;
  
    $sheet->setCellValueByColumnAndRow(0, $row, 'Order #');
    $sheet->setCellValueByColumnAndRow(1, $row, 'Name');
    $sheet->setCellValueByColumnAndRow(2, $row, 'Quantity');
    $sheet->getStyle('A'.$row.':C'.$row)->applyFromArray(array(
      'font' => array(
        'bold' => true, 
        'color' => array('rgb' => 'FFFFFF'),
      ),
      'fill' => array(
        'type' => PHPExcel_Style_Fill::FILL_SOLID, 
        'color' => array('rgb' => '000000'),
      ),
    ));
    $row++;

    $order_products = _uc_buyingclub_download_by_product_get_order_products_ordered($ordering_period_id, $product->id);
    foreach ($order_products as $order_product) {
      $sheet->setCellValueByColumnAndRow(0, $row, $new_order_ids[$order_product->order_id]);
      $sheet->setCellValueByColumnAndRow(1, $row, $order_product->last .', '. $order_product->first);
      $sheet->setCellValueByColumnAndRow(2, $row, $order_product->qty);
      $row++;
    }

    $sheet->setCellValueByColumnAndRow(1, $row, "OVERSTOCK");
    $sheet->setCellValueByColumnAndRow(2, $row, $product->qty_overstock);
    $row++;

    $sheet->setBreak('A'. ($row-1), PHPExcel_Worksheet::BREAK_ROW);
  }

  // Rename sheet
  $sheet->setTitle('By product');

  _uc_buyingclub_set_excel_headers('by product ' . date('Ymd') . '.xlsx');
      
  // Save Excel 2007 file
  $objWriter = new PHPExcel_Writer_Excel2007($objPHPExcel);
  $objWriter->save('php://output');

  $objPHPExcel->disconnectWorksheets();
  unset($objPHPExcel);
}

function _uc_buyingclub_set_excel_headers($filename) {
  header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT");
  header("Cache-Control: no-store, no-cache, must-revalidate");
  header("Cache-Control: post-check=0, pre-check=0", false);
  header("Pragma: no-cache");
  header('Content-Type: application/vnd.ms-excel');
  header('Content-Disposition: attachment;filename="'. $filename .'"');
}

function uc_buyingclub_download_finalized($ordering_period_id, $distributor_id) {
  // select all products with the total ordered
  $distributor_table = _uc_buyingclub_get_distributor_table();
  $distributor_column = _uc_buyingclub_get_distributor_column();

  $minimum_order_table = _uc_buyingclub_get_minimum_order_table();
  $minimum_order_column = _uc_buyingclub_get_minimum_order_column();

  $unit_table = _uc_buyingclub_get_unit_table();
  $unit_column = _uc_buyingclub_get_unit_column();
  
  $query = "SELECT p.title AS 'item', ctd2.". $minimum_order_column ." AS `bulk req`, ctd3.". $unit_column ." AS `unit`, up.cost * ctd2.". $minimum_order_column ." AS `cast cost`, ubp.qty_ordered / ctd2.". $minimum_order_column ." AS `cases ordered`, up.cost * ubp.qty_ordered AS `total cost` ".
    "FROM {uc_buyingclub_products} ubp, {uc_products} up, {node} p, {node} d, {". $distributor_table ."} ctd, {". $minimum_order_table ."} ctd2, {". $unit_table ."} ctd3 ".
    "WHERE ubp.product_id = p.nid ".
    "AND ctd.vid = p.vid ".
    "AND ctd.". $distributor_column ." = %d ".
    "AND ctd2.vid = p.vid ".
    "AND ctd3.vid = p.vid ".
    "AND ubp.ordering_period_id = %d AND up.nid=p.nid ".
    "AND ubp.uc_buyingclub_product_id = ( ".
        "SELECT ubp2.uc_buyingclub_product_id ".
        "FROM {uc_buyingclub_products} ubp2 ".
        "WHERE ubp2.product_id = p.nid ".
        "ORDER BY ubp2.finalized DESC ".
        "LIMIT 1 ".
      ") ".
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
  require_once 'PHPExcel.php';
  require_once 'PHPExcel/Writer/Excel2007.php';

  $objPHPExcel = new PHPExcel();

  $objPHPExcel->getProperties()->setTitle("Invoices");
  $objPHPExcel->getProperties()->setSubject("Invoices");
  $objPHPExcel->getProperties()->setDescription("Invoices");

  $objPHPExcel->setActiveSheetIndex(0);

  $sheet = $objPHPExcel->getActiveSheet();

  $header_label_format = array(
    'font' => array(
      'bold' => true, 
      'color' => array('rgb' => 'FFFFFF'),
    ),
    'fill' => array(
      'type' => PHPExcel_Style_Fill::FILL_SOLID, 
      'color' => array('rgb' => '000000'),
    ),
  );
  /*
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
    'align'=>'left',
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
   */

  $sheet->getColumnDimension('A')->setAutoSize(true);
  $sheet->getColumnDimension('B')->setAutoSize(true);
  $sheet->getColumnDimension('C')->setAutoSize(true);
  $sheet->getColumnDimension('D')->setAutoSize(true);
  $sheet->getColumnDimension('E')->setAutoSize(true);

  $row = 1;
  $new_order_id = 1;
  foreach (_uc_buyingclub_download_invoices_get_orders($ordering_period_id) as $order) {
    $row = _uc_buyingclub_download_invoices_write_header($new_order_id, $order->first, $order->last, $order->email, $order->phone, $sheet, &$row);
    $row = _uc_buyingclub_download_invoices_write_details($order->order_id, $sheet, &$row);

    $row++;
    $new_order_id++;

    $sheet->setBreak('A'. ($row-1), PHPExcel_Worksheet::BREAK_ROW);
  }

  $sheet->setTitle('Invoices');

  _uc_buyingclub_set_excel_headers('invoices ' . date('Ymd') . '.xlsx');
      
  // Save Excel 2007 file
  $objWriter = new PHPExcel_Writer_Excel2007($objPHPExcel);
  $objWriter->save('php://output');

  $objPHPExcel->disconnectWorksheets();
  unset($objPHPExcel);
}

function _uc_buyingclub_download_invoices_write_header($id, $first, $last, $email, $phone, $sheet, &$row) {
  $first_header_row = $row;

  $sheet->setCellValueByColumnAndRow(0, $row, 'Greene Hill Food Co-op');
  $sheet->setCellValueByColumnAndRow(1, $row, 'Order Number');
  $sheet->setCellValueByColumnAndRow(3, $row, $id);
  $sheet->mergeCells('B'. $row .':C'. $row);
  $sheet->mergeCells('D'. $row .':E'. $row);
  $row++;

  $sheet->setCellValueByColumnAndRow(0, $row, 'Buying Club Order Confirmation');
  $sheet->setCellValueByColumnAndRow(1, $row, 'First Name');
  $sheet->setCellValueByColumnAndRow(3, $row, $first);
  $sheet->mergeCells('B'. $row .':C'. $row);
  $sheet->mergeCells('D'. $row .':E'. $row);
  $row++;

  $date_format = 'l\, M jS Y';
  $sheet->setCellValueByColumnAndRow(0, $row, date($date_format));
  $sheet->setCellValueByColumnAndRow(1, $row, 'Last Name');
  $sheet->setCellValueByColumnAndRow(3, $row, $last);
  $sheet->mergeCells('B'. $row .':C'. $row);
  $sheet->mergeCells('D'. $row .':E'. $row);
  $row++;

  $sheet->setCellValueByColumnAndRow(1, $row, 'Email Address');
  $sheet->setCellValueByColumnAndRow(3, $row, $email);
  $sheet->mergeCells('B'. $row .':C'. $row);
  $sheet->mergeCells('D'. $row .':E'. $row);
  $row++;

  $sheet->setCellValueByColumnAndRow(1, $row, 'Phone');
  $sheet->setCellValueByColumnAndRow(3, $row, $phone);
  $sheet->mergeCells('B'. $row .':C'. $row);
  $sheet->mergeCells('D'. $row .':E'. $row);

  $sheet->getStyle('B'. $first_header_row .':C'. $row)->applyFromArray(array(
    'fill' => array(
      'type' => PHPExcel_Style_Fill::FILL_SOLID,
      'color' => array('rgb' => '929292'),
    ),
    'borders' => array(
      'horizontal' => array(
        'style' => PHPExcel_Style_Border::BORDER_THIN,
      ),
      'bottom' => array(
        'style' => PHPExcel_Style_Border::BORDER_THIN,
      ),
    ),
  ));
  $sheet->getStyle('D'. $first_header_row .':E'. $row)->applyFromArray(array(
    'borders' => array(
      'horizontal' => array(
        'style' => PHPExcel_Style_Border::BORDER_THIN,
      ),
      'bottom' => array(
        'style' => PHPExcel_Style_Border::BORDER_THIN,
      ),
    ),
    'alignment' => array(
      'horizontal' => PHPExcel_Style_Alignment::HORIZONTAL_LEFT,
    ),
  ));

  $row++;
  $row++;

  $sheet->setCellValueByColumnAndRow(0, $row, 'QUANTITY');
  $sheet->setCellValueByColumnAndRow(1, $row, 'ITEM');
  $sheet->setCellValueByColumnAndRow(2, $row, 'UNIT');
  $sheet->setCellValueByColumnAndRow(3, $row, 'UNIT PRICE');
  $sheet->setCellValueByColumnAndRow(4, $row, 'PRICE');

  $sheet->getStyle('A'. $row .':E'. $row)->applyFromArray(array(
    'fill' => array(
      'type' => PHPExcel_Style_Fill::FILL_SOLID,
      'color' => array('rgb' => '929292'),
    ),
    'borders' => array(
      'horizontal' => array(
        'style' => PHPExcel_Style_Border::BORDER_THIN,
      ),
      'bottom' => array(
        'style' => PHPExcel_Style_Border::BORDER_THIN,
      ),
    ),
  ));

  $row++;
  return $row;
}

function _uc_buyingclub_download_invoices_write_details($order_id, $sheet, &$row) {
  $category_style = array(
    'fill' => array(
      'type' => PHPExcel_Style_Fill::FILL_SOLID,
      'color' => array('rgb' => 'CBCBCB'),
    ),
  );
  $details_style = array(
    'borders' => array(
      'inside' => array(
        'style' => PHPExcel_Style_Border::BORDER_THIN,
      ),
      'bottom' => array(
        'style' => PHPExcel_Style_Border::BORDER_THIN,
      ),
      'right' => array(
        'style' => PHPExcel_Style_Border::BORDER_THIN,
      ),
    ),
  );

  $first_detail_row = $row;

  $current_category = '';
  foreach (_uc_buyingclub_download_invoices_get_order_details($order_id) as $details) {
    if ($details->qty == 0) continue;
    if ($details->category != $current_category) {
      $current_category = $details->category;
      $sheet->setCellValueByColumnAndRow(0, $row, $current_category);
      $sheet->mergeCells('A'. $row .':E'. $row);
      $sheet->getStyle('A'. $row .':E'. $row)->applyFromArray($category_style);
      $row++;
    }

    $sheet->setCellValueByColumnAndRow(0, $row, $details->qty);
    $sheet->setCellValueByColumnAndRow(1, $row, $details->name);
    $sheet->setCellValueByColumnAndRow(2, $row, $details->unit);
    $sheet->setCellValueByColumnAndRow(3, $row, $details->price);
    $sheet->setCellValueByColumnAndRow(4, $row, '=A'. $row .'*D'. $row);

    $row++;
  }
  $last_detail_row = $row - 1;

  $orig_subtotal_row = $row;

  $sheet->getStyle('A'. $first_detail_row .':E'. $last_detail_row)->applyFromArray($details_style);
  $sheet->getStyle('D'. $first_detail_row .':E'. $row)->getNumberFormat()->setFormatCode('$#,##0.00');

  $sheet->setCellValueByColumnAndRow(2, $row, 'Subtotal');
  $sheet->setCellValueByColumnAndRow(4, $row, '=SUM(E'. ($first_detail_row) .':E'. ($last_detail_row) .')');
  $sheet->mergeCells('C'. $row .':D'. $row);

  $row += 2;

  $sheet->setCellValueByColumnAndRow(0, $row, 'overstock');
  $sheet->mergeCells('A'. $row .':E'. $row);
  $sheet->getStyle('A'. $row .':E'. $row)->applyFromArray($category_style);
  $row++;

  $first_overstock_row = $row;
  for ($i = 0; $i < 6; $i++) {
    $sheet->setCellValueByColumnAndRow(0, $row, 0);
    $sheet->setCellValueByColumnAndRow(3, $row, 0);
    $sheet->setCellValueByColumnAndRow(4, $row, '=A'. $row .'*D'. $row);
    $row++;
  }
  $last_overstock_row = $row - 1;

  $sheet->getStyle('A'. $first_overstock_row .':E'. $last_overstock_row)->applyFromArray($details_style);
  $sheet->getStyle('D'. $first_overstock_row .':E'. $row)->getNumberFormat()->setFormatCode('$#,##0.00');

  $overstock_subtotal_row = $row;
  $sheet->setCellValueByColumnAndRow(2, $row, 'Overstock Subtotal');
  $sheet->setCellValueByColumnAndRow(4, $row, '=SUM(E'. $first_overstock_row .':E'. $last_overstock_row .')');
  $sheet->mergeCells('C'. $row .':D'. $row);
  $row++;

  $sheet->setCellValueByColumnAndRow(2, $row, 'Total');
  $sheet->setCellValueByColumnAndRow(4, $row, '=E'. $orig_subtotal_row .'+E'. $overstock_subtotal_row);
  $sheet->mergeCells('C'. $row .':D'. $row);
  $row++;

  $row++;
  $sheet->setCellValueByColumnAndRow(0, $row, 'comments');
  $sheet->setCellValueByColumnAndRow(1, $row, _uc_buyingclub_download_invoices_get_order_comment($order_id));
  $row++;

  return $row;
}

function _uc_buyingclub_download_invoices_get_order_comment($order_id) {
  $query = "SELECT message ".
    "FROM uc_order_comments ".
    "WHERE order_id=%d ";
  $result = db_query($query, $order_id);

  $comments = db_fetch_array($result);
  if ($comments != NULL && count($comments) >= 1) {
    return $comments['message'];
  }
  return "";
}

function _uc_buyingclub_download_invoices_get_order_details($order_id) {
  $unit_table = _uc_buyingclub_get_unit_table();
  $unit_column = _uc_buyingclub_get_unit_column();

  $query = "SELECT p.title as name, unit_table.". $unit_column ." as unit, bop.qty as qty, op.price as price, td.name as category ".
    "FROM {node} p, {uc_order_products} op, {uc_orders} o, {uc_buyingclub_order_products} bop, {". $unit_table ."} unit_table, {term_node} tn, {term_data} td  ".
    "WHERE op.nid = p.nid AND op.order_id=o.order_id AND bop.order_product_id=op.order_product_id AND o.order_id=%d AND unit_table.vid=p.vid  ".
      "AND tn.nid=p.nid AND td.tid=tn.tid ".
      "AND bop.uc_buyingclub_order_product_id = ( ".
        "SELECT bop2.uc_buyingclub_order_product_id ".
        "FROM {uc_buyingclub_order_products} bop2 ".
        "WHERE bop2.order_product_id = bop.order_product_id ".
        "ORDER BY bop2.finalized DESC ".
        "LIMIT 1 ".
      ") ".
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
    "FROM {uc_orders} o, {uc_buyingclub_ordering_periods} ordering_period ".
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
  $query = "SELECT bp.product_id AS `nid`, bp.qty_overstock AS `qty` ".
    "FROM {uc_buyingclub_products} bp ".
    "WHERE bp.ordering_period_id=%d ".
      "AND bp.uc_buyingclub_product_id = ( ".
        "SELECT bp2.uc_buyingclub_product_id ".
        "FROM {uc_buyingclub_products} bp2 ".
        "WHERE bp2.product_id = bp.product_id ".
        "ORDER BY bp2.finalized DESC ".
        "LIMIT 1 ".
      ") ";

  $result = db_query($query, $ordering_period_id);

  while ($row = db_fetch_array($result)) {
    $overstock[] = $row;
  }
  return $overstock;
}

function _uc_buyingclub_download_grid_get_products_sold($ordering_period_id) {
  $query = "SELECT p.nid, sum(bop.qty) AS `qty` ".
    "FROM {node} p, {uc_buyingclub_order_products} bop, {uc_buyingclub_ordering_periods} period, {uc_orders} o, {uc_order_products} op ".
    "WHERE (o.order_status='pending' OR o.order_status='completed') AND o.created > period.start AND o.created < period.end ".
      "AND op.order_id=o.order_id AND bop.order_product_id=op.order_product_id AND op.nid=p.nid AND period.ordering_period_id=%d ".
      "AND bop.uc_buyingclub_order_product_id = ( ".
        "SELECT bop2.uc_buyingclub_order_product_id ".
        "FROM {uc_buyingclub_order_products} bop2 ".
        "WHERE bop2.order_product_id = bop.order_product_id ".
        "ORDER BY bop2.finalized DESC ".
        "LIMIT 1 ".
      ") ".
    "GROUP BY p.nid";

  $result = db_query($query, $ordering_period_id);

  while ($row = db_fetch_array($result)) {
    $sold[] = $row;
  }
  return $sold;
}

function _uc_buyingclub_download_grid_get_order_products($order_id) {
  $query = "SELECT op.nid, bop.qty ".
    "FROM {uc_order_products} op, {uc_buyingclub_order_products} bop ".
    "WHERE bop.order_product_id=op.order_product_id AND op.order_id=%d ".
      "AND bop.uc_buyingclub_order_product_id = ( ".
        "SELECT bop2.uc_buyingclub_order_product_id ".
        "FROM {uc_buyingclub_order_products} bop2 ".
        "WHERE bop2.order_product_id = bop.order_product_id ".
        "ORDER BY bop2.finalized DESC ".
        "LIMIT 1 ".
      ") ";

  $result = db_query($query, $order_id);

  while ($row = db_fetch_array($result)) {
    $products[] = $row;
  }
  return $products;
}

function _uc_buyingclub_download_grid_get_users($ordering_period_id) {
  $query = "SELECT o.order_id, o.billing_last_name AS `last`, o.billing_first_name AS `first` ".
    "FROM {uc_orders} o, {uc_buyingclub_ordering_periods} period ".
    "WHERE (o.order_status='pending' OR o.order_status='completed') AND o.created > period.start AND o.created < period.end AND period.ordering_period_id=%d ".
    "ORDER BY o.billing_last_name, o.billing_first_name";

  $result = db_query($query, $ordering_period_id);

  while ($row = db_fetch_array($result)) {
    $users[] = $row;
  }
  return $users;
}

function _uc_buyingclub_download_grid_get_ordered_products($ordering_period_id) {
  $unit_table = _uc_buyingclub_get_unit_table();
  $unit_column = _uc_buyingclub_get_unit_column();

  $query = "SELECT p.nid, p.title as `name`, pr.sell_price as `price`, unit_table." . $unit_column ." as `unit`, bp.qty_ordered ".
    "FROM {node} p, {uc_products} pr, {uc_buyingclub_products} bp, {". $unit_table ."} unit_table ".
    "WHERE pr.nid=p.nid AND bp.product_id=p.nid AND bp.ordering_period_id=%d AND unit_table.vid=p.vid ".
      "AND bp.uc_buyingclub_product_id = ( ".
        "SELECT bp2.uc_buyingclub_product_id ".
        "FROM {uc_buyingclub_products} bp2 ".
        "WHERE bp2.product_id = bp.product_id ".
        "ORDER BY bp2.finalized DESC ".
        "LIMIT 1 ".
      ") ".
    "ORDER BY p.title";

  $result = db_query($query, $ordering_period_id);

  while ($row = db_fetch_array($result)) {
    $products[] = $row;
  }

  return $products;
}

/*
 * Get new order numbers, alphabetically ordered
 */
function _uc_buyingclub_download_by_product_get_new_order_ids($ordering_period_id) {
  $query = "SELECT o.order_id as id, o.billing_first_name as first, o.billing_last_name as last ".
    "FROM {uc_orders} o, {uc_buyingclub_ordering_periods} ordering_period ".
    "WHERE o.created > ordering_period.start AND o.created < ordering_period.end ".
    "AND ordering_period.ordering_period_id=%d  AND (o.order_status = 'completed' OR o.order_status = 'pending') ".
    "ORDER BY o.billing_last_name, o.billing_first_name";

  $result = db_query($query, $ordering_period_id);
  $new_order_id = 1;
  $orders = array();

  while ($obj = db_fetch_object($result)) {
    $orders[$obj->id] = $new_order_id;
    $new_order_id++;
  }

  return $orders;
}

function _uc_buyingclub_download_by_product_get_products_ordered($ordering_period_id) {
  $unit_table = _uc_buyingclub_get_unit_table();
  $unit_column = _uc_buyingclub_get_unit_column();

  $query = "SELECT p.nid as id, p.title as name, unit_table.". $unit_column ." as unit, bp.qty_overstock as qty_overstock ".
    "FROM {uc_buyingclub_products} bp, {node} p, {". $unit_table ."} unit_table ".
    "WHERE bp.product_id=p.nid AND bp.ordering_period_id=%d AND unit_table.vid=p.vid ".
    "AND bp.uc_buyingclub_product_id = ( ".
        "SELECT bp2.uc_buyingclub_product_id ".
        "FROM {uc_buyingclub_products} bp2 ".
        "WHERE bp2.product_id = p.nid ".
        "ORDER BY bp2.finalized DESC ".
        "LIMIT 1 ".
      ") ".
    "ORDER BY p.title ";

  $products = array();
  $result = db_query($query, $ordering_period_id);
  while ($obj = db_fetch_object($result)) {
    $products[] = $obj;
  }

  return $products;
}

function _uc_buyingclub_download_by_product_get_order_products_ordered($ordering_period_id, $product_id) {
  $unit_table = _uc_buyingclub_get_unit_table();
  $unit_column = _uc_buyingclub_get_unit_column();

  $query = "SELECT o.order_id as order_id, o.billing_last_name as last, o.billing_first_name as first, bop.qty as qty ".
    "FROM {uc_buyingclub_order_products} bop, {uc_orders} o, {uc_order_products} op, {uc_buyingclub_ordering_periods} period ".
    "WHERE bop.order_product_id=op.order_product_id AND op.order_id=o.order_id ".
    "AND o.created > period.start AND o.created < period.end ".
    "AND (o.order_status = 'pending' OR o.order_status = 'completed') ".
    "AND period.ordering_period_id=%d  ".
    "AND op.nid=%d  ".
    "AND bop.qty > 0 ".
    "GROUP BY op.order_product_id ".
    "ORDER BY o.billing_last_name, o.billing_first_name ";

  $order_products = array();
  $result = db_query($query, $ordering_period_id, $product_id);
  while ($obj = db_fetch_object($result)) {
    $order_products[] = $obj;
  }

  return $order_products;
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

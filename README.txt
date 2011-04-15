// $Id$

uc_buyingclub
-------------

uc_buyingclub helps your group or budding food cooperative run a buying club on top of Ubercart and Drupal. 

It gives buying club members access to an order form during ordering periods. Once an ordering period is over, a buying club coordinator can decide how many cases of each item should be ordered and download the orders for each distributor, and this sends an updated invoice to each buying club member. On distribution day, a buying club coordinator can update the quantities actually received from distributors, and this updates the invoices for each buying club member.


INSTALLATION
------------

Install required modules.

Then create a Distributor content type. Distributors are the groups and individuals from which products are ordered.

Next, create two fields with CCK on the Product content type created by Ubercart:
 * field_distributor, a node reference that can only point to the Distributor content type that you just created
 * field_minimum_order, an integer
 * field_unit, a string

Finally, put this folder into your modules directory and enable the module.


USAGE
-----

This module has a page, /buyingclub/finalize, which allows a user with uc_buyingclub admin permission to finalize all orders for the most recent ordering cycle. Here, an admin will see a table summing the quantity ordered for each product, the minimum quantity required by the product's distributor, and will have the ability to set the quantity that will be ordered from the distributor. When the form is submitted, products are given to each user on a first-come-first-served basis, and each ordering user receives an email letting them know what they should expect to pick up on the next pick up date.

To try the module, make some products with distributors and place a few orders. Then go to /buyingclub/finalize to finalize orders for the ordering period.

To see the finalized orders for the most recent/current ordering period, look at the following:
 * buyingclub/view/finalized
 * buyingclub/view/finalized/by_user
 * buyingclub/view/finalized/by_distributor

To see ordering periods, go to
 * buyingclub/view/periods


MODULES
-------

uc_buyingclub's functionality is split over a number of modules:
 * uc_buyingclub
  * The main module, containing the basics-letting coordinators "finalize" orders at the end of an ordering period.
 * uc_buyingclub_cart
  * Modifies uc_cart. Currently shows units in the cart.
 * uc_buyingclub_catalog
  * Modifies uc_catalog. Currently changes the breadcrumbs, shows members the start and end of the most recent or current ordering period.
 * uc_buyingclub_civicrm
  * Grabs members' contact information from CiviCRM.
 * uc_buyingclub_multibuy
  * Modifies uc_multibuy. Adds relevant bits (eg, distributors) to tables.
 * uc_buyingclub_product
  * Modifies uc_product. Conveniently calculates unit cost/price based on case size and cost, adding a markup as desired.


TODO
----

While the concept of ordering periods exists, the module does not allow a buying club manager to pick an ordering period.

Make updated invoices more easily themed and configured by buying club managers.

Most things need to be made configurable.

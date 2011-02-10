// $Id$

uc_buyingclub
-------------

A Drupal module built on Ubercart that performs some buying club-specific functions.


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


TODO
----

While the concept of ordering periods exists, the module does not allow a buying club manager to pick an ordering period.

Track overstock (the amount ordered from distributors that wasn't ordered by members).

Make updated invoices more easily themed and configured by buying club managers.

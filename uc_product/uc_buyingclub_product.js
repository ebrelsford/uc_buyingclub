
/*
 * enter case cost, automatically calculate the rest
 *
 * XXX: Assumes CCK fields field_minimum_order and field_case_cost exist on this product class
 */
Drupal.behaviors.ucBuyingClubProductCaseCost = function(context) {
    
    var roundNumber = function(number, digits) {
        var multiple = Math.pow(10, digits);
        var rndedNum = Math.round(number * multiple) / multiple;
        return rndedNum;
    }

    $(context).find('input[name^=field_minimum_order], input[name^=field_case_cost]').not('.ucBuyingClubProductCaseCost-processed').each(function() {
        $(this).change(function() {
            var case_cost = $(context).find('input[name^=field_case_cost]').val();
            var case_size = $(context).find('input[name^=field_minimum_order]').val();
            var per_unit_cost = 0;
            var per_unit_price = 0;

            if (!isNaN(case_cost) && case_cost > 0 &&
                !isNaN(case_size) && case_size > 0) {
                var per_unit_cost = roundNumber(case_cost / case_size, 2);
                var per_unit_price = roundNumber(per_unit_cost * 1.2, 2);
            }

            $(context).find('#edit-list-price').val(per_unit_price);
            $(context).find('#edit-sell-price').val(per_unit_price);
            $(context).find('#edit-cost').val(per_unit_cost);
        }).addClass('ucBuyingClubProductCaseCost-processed');
    });
}

/*
 * use jQuery form examples to make quantities more useable--'0' now is automatically removed, gets replaced if the field is left emtpy
 */
Drupal.behaviors.ucBuyingClubCatalogExamples = function(context) {
    $(context).find('#uc-multibuy-table .add-to-cart input[type=text]:not(.ucBuyingClubCatalogExamples-processed)').each(function() {
        $(this).attr('value', '')
            .example('0')
            .addClass('ucBuyingClubCatalogExamples-processed');
    });
}

/*
 * Keep form examples from breaking validation
 */
Drupal.behaviors.ucBuyingClubCatalogFixBeforeSubmit = function(context) {
    $(context).find('#uc-multibuy-table form:not(.ucBuyingClubCatalogFixBeforeSubmit-processed)').each(function() {
        $(this).submit(function() {
            // don't reset these inputs to empty
            $(this).find('input[type=text].example')
                .attr('value', '0');
        })
        $(this).addClass('ucBuyingClubCatalogFixBeforeSubmit-processed');
    });
}

/*
 * update number of cases to order and expected overstock on finalization form
 */
Drupal.behaviors.ucBuyingClubFinalizationFormUpdateCases = function(context) {
    $(context).find('.ucBuyingClubOverstock:not(.ucBuyingClub-processed), .ucBuyingClubUnitsToOrder:not(.ucBuyingClub-processed)').each(function() {
        $(this).change(function() {
            var row = $(this).parents('tr');

            var units_ordered = parseInt(row.find('.ucBuyingClubUnitsOrdered').text());
            var would_buy_more = parseInt(row.find('.ucBuyingClubWouldBuyMore').text());
            var case_size = parseInt(row.find('.ucBuyingClubCaseSize').text());
            var current_overstock = parseInt(row.find('.ucBuyingClubOverstock').val());
            var units_to_order = parseInt(row.find('.ucBuyingClubUnitsToOrder').val());

            // update cases field
            var cases_to_order = units_to_order / case_size;
            row.find('.ucBuyingClubCasesToOrder').val(cases_to_order);
            if (cases_to_order % 1 !== 0) {
                // not an even number of cases
                row.find('.ucBuyingClubUnitsToOrder').addClass('invalid-value');
            }
            else {
                row.find('.ucBuyingClubUnitsToOrder').removeClass('invalid-value');
            }

            // update expected overstock field
            var expected_overstock = units_to_order - units_ordered - would_buy_more + current_overstock;
            if (expected_overstock < 0) expected_overstock = 0;
            row.find('.ucBuyingClubExpectedOverstock').val(expected_overstock);
        })
        $(this).addClass('ucBuyingClubUpdateCases-processed');
    });
}

/*
 * Confirm before submitting finalization form
 */
Drupal.behaviors.ucBuyingClubFinalizationFormConfirm = function(context) {
    $(context).find('form#uc-buyingclub-finalization-form:not(.ucBuyingClubFinalizationFormConfirm-processed)').each(function() {
        $(this).submit(function() {
            return confirm('Are you sure you want to finalize this ordering period? There is no going back! Everyone will get an email when you submit this.');
        })
        $(this).addClass('ucBuyingClubFinalizationFormConfirm-processed');
    });
}

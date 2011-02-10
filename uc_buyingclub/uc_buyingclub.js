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

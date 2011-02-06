/*
 * use jQuery form examples to make quantities more useable--'0' now is automatically removed, gets replaced if the field is left emtpy
 */
Drupal.behaviors.ucBuyingClubCatalogExamples = function(context) {
    $('#uc-multibuy-table .add-to-cart input[type=text]:not(.ucBuyingClubCatalogExamples-processed)').each(function() {
        $(this).attr('value', '')
            .example('0')
            .addClass('ucBuyingClubCatalogExamples-processed');
    });
}

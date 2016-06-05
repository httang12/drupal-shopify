<?php

/**
 * admin menu look to setup admin variables
 * @return mixed
 */
function shopify_categories_settings() {

    $form = array();

    $form['description'] = array(
        '#value' => variable_get('description', t('This is the administration page')),
    );

    $form['main'] = array(
        '#type' => 'fieldset',
        '#title' => t('Service Administration'),
        '#description' => t('Admin Functions.'),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
    );

    $form['main']['sync_data_to_shopify_store'] = array(
        '#access' => user_access('administer shopify shop'),
        '#type' => 'link',
        '#title' => 'Sync Collections To Shopify',
        '#href' => 'call-back-processes/shopify/sync-collections',
    );

    return system_settings_form($form);
}

?>
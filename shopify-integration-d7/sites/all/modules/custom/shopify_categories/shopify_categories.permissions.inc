<?php

//this is the overriding of permission settings in Find A Doctor D7 module
function shopify_categories_permission() {
    return array(
        'administer shopify shop' => array(
            'title' => t('administer shopify shop'),
            'description' => t('Permission Setting for administer shopify shop.'),
        ),
    );
}
?>
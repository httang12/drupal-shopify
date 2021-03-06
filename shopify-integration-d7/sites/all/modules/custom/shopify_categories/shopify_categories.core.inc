<?php

DEFINE('shopify_categories_customcategory_key','TopTierCollections');
DEFINE('shopify_categories_drupal_key','DrupalKey');
DEFINE('shopify_categories_namespace','hmtrevolution');
DEFINE('shopify_categories_subcategories_key','drupal-sub-categories-list');
DEFINE('shopify_categories_homecategories_key','is-drupal-home-category');
DEFINE('shopify_categories_pagesize',250);

/**
 * sync shopify collections with the
 */
function shopify_categories_sync()
{
    shopify_categories_push();
    return "<h3>Configurations and Collections are now sync'd.</h3>";
}

/**
 * function hook into node save mechanism to update shopify
 * @param $node
 */
function shopify_categories_node_insert($node)
{
    if ($node->type = 'subcategory')
    {
        shopify_categories_createNewCollection($node);
    }
}

/**
 * function hook into node update mechanism
 * @param $node
 */
function shopify_categories_node_update($node)
{
    if ($node->type = 'subcategory')
    {
        shopify_categories_updateCollection($node);
    }
}

/**
 * function hook into node delete mechanism
 * @param $node
 */
function shopify_categories_node_delete($node)
{
    if ($node->type = 'subcategory')
    {
        shopify_categories_deleteCollection($node);
    }
}
//function to push category configuration to shopify!!!
function shopify_categories_push()
{
    //initialize the client object
    $client = shopify_api_client();

    //get all the collections and filter only ones with Drupal ID!
    //drupal ID is the value, shopify ID is the key
    $nonBaseCollections = shopify_categories_getNonBaseCollections();

    //start the update process
    //loop through the entity for all
    //this script will set the default value for location content type, run it as one time process only.
    $content_type = 'subcategory';

    $nodes = node_load_multiple(array(), array('type' => $content_type));
    foreach ($nodes as $node) {
        $node_wrapper = entity_metadata_wrapper('node', $node);
        //check if the node is going to be create or update

        $newCollection = true;
        if (in_array($node_wrapper->getIdentifier(),$nonBaseCollections)===true)
        {
            $newCollection = false;

        }

        //if new Collection, create new one in Shopify
        if ($newCollection)
            shopify_categories_createNewCollection($node);
    }

}

/**
 * remove any nonBase Custom Collections that contains an invalid Drupal ID
 */
function shopify_categories_cleanShopifyCollections()
{
    //initialize the client object
    $client = shopify_api_client();

    $nonBaseCollections = shopify_categories_getNonBaseCollections();

    foreach($nonBaseCollections as $collectionID => $drupalID)
    {
        if (node_load($drupalID)===false)
        {
            //delete if node does not exist
            $collection_results = $client->call('DELETE', '/admin/custom_collections/'. $collectionID .'.json', array('limit' => 250, 'fields' => 'id, metafield, title'));
        }
    }
}


/**
 * this is performed after all new collections are added and created properly
 * it updates the meta fields where the category id is stored.
 */
function shopify_categories_syncCateogryLinks()
{

}

function shopify_categories_getNonBaseCollections()
{
    $nonbase_collections = array();

    //query shopify to get the base collections
    //initialize the client object
    //query shopify to get the base collections
    $collection_results = shopify_categories_getCollections();

    foreach($collection_results as $collection) {
        $collectionID = $collection['id'];
        $collectionTitle = $collection['title'];
        
        $metaDatas = shopify_categories_getMetaFieldsByCollectionID($collectionID);

        $isBaseCategory = true;
        $drupalID = '';


        if(isset($metaDatas))
            foreach($metaDatas as $metafield)
            {
                if($metafield['key']==shopify_categories_drupal_key)
                {
                    $isBaseCategory = false;
                    $drupalID = $metafield['value'];
                    break;
                }
            }
        if (!$isBaseCategory && $drupalID != '')
        {
            $nonbase_collections[$collectionID] = $drupalID;
        }

    }
    return $nonbase_collections;
}

/**
 * Core function to retrieve all collections, support collection count more than 250
 *
 * @return array
 * @throws ShopifyApiException
 */
function shopify_categories_getCollections()
{
    //initialize the client object
    $client = shopify_api_client();

    $overallResults = array();
    $total_pages = shopify_categories_getCustomCollectionCount()/shopify_categories_pagesize;
    $total_pages = ceil($total_pages);

    $total_smartPages = shopify_categories_getSmartCollectionCount()/shopify_categories_pagesize;
    $total_smartPages = ceil($total_smartPages);

    $countIndex = 1;
    while($countIndex<=$total_pages)
    {
        $collection_results = $client->call('GET', '/admin/custom_collections.json', array('limit' => shopify_categories_pagesize, 'page'=>$countIndex ,'fields' => 'id, title'));
        $overallResults = array_merge($overallResults,$collection_results);
        $countIndex++;
    }

    $countIndex = 1;
    while($countIndex<=$total_smartPages)
    {
        $collection_results = $client->call('GET', '/admin/smart_collections.json', array('limit' => shopify_categories_pagesize, 'page'=>$countIndex ,'fields' => 'id, title'));
        $overallResults = array_merge($overallResults,$collection_results);
        $countIndex++;
    }

    return $overallResults;
}


/**
 * get the base collections for shopify system
 */
function shopify_categories_getBaseCollections()
{
    $base_collections = array();

    //query shopify to get the base collections
    $collection_results = shopify_categories_getCollections();

    foreach($collection_results as $collection) {
        $collectionID = $collection['id'];
        $collectionTitle = $collection['title'];
        $isBaseCategory = true;

        $base_collections[$collectionID] = $collectionTitle;

    }
    return $base_collections;
}

/**
 * Get the metal field of a collection
 * @param $collectionID
 * @return array
 * @throws ShopifyApiException
 */
function shopify_categories_getMetaFieldsByCollectionID($collectionID)
{
    //initialize the client object
    $client = shopify_api_client();
    $results = $client->call('GET', '/admin/metafields.json', array('limit' => 50,
        'metafield[owner_id]' => $collectionID,
        'metafield[owner_resource]' => 'collection',
        'fields' => 'key,value,id'));


    return $results;
}

function shopify_categories_getCustomCollectionCount()
{
    //initialize the client object
    $client = shopify_api_client();
    $count = $client->call('GET', '/admin/custom_collections/count.json');

    return $count;
}


function shopify_categories_getSmartCollectionCount()
{
    //initialize the client object
    $client = shopify_api_client();
    $count = $client->call('GET', '/admin/smart_collections/count.json');

    return $count;
}
?>
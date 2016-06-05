<?php

DEFINE('shopify_categories_customcategory_key','TopTierCollections');
DEFINE('shopify_categories_drupal_key','DrupalKey');
DEFINE('shopify_categories_namespace','hmtrevolution');
DEFINE('shopify_categories_subcategories_key','drupal-sub-categories-list');
DEFINE('shopify_categories_homecategories_key','is-drupal-home-category');


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

function shopify_categories_createNewCollection(&$node)
{
    //initialize the client object
    $client = shopify_api_client();
    $node_wrapper = entity_metadata_wrapper('node',$node);

    $nodeID = $node_wrapper->getIdentifier();
    $nodetitle = $node_wrapper->label();
    $nodebody = $node_wrapper->body->raw();

    $isHomeCategory = $node_wrapper->field_home_page_collection->raw();

    //set string for metadata
    if ($isHomeCategory == 0)
        $isHomeCategory = 'yes';
    else
        $isHomeCategory = 'no';
    //aggregation of all categories
    $subcategoryIDs = '';
    $rawimgpath = $node_wrapper->field_sub_categories_image->value()['uri'];
    $imagePath = file_create_url($rawimgpath);
    $imagePath = str_replace("/","\/" ,$imagePath);

    $postData = array('custom_collection' =>
        array(
            'title' => $nodetitle,
            'body' => $nodebody,

            //this only works if the drupal instance is available publically.
            //'image' => array('src' => $imagePath),

            'metafields' => array(
                array('key'=>shopify_categories_drupal_key,'value'=>$nodeID,'value_type'=>'string','namespace'=>shopify_categories_namespace),
                array('key'=>shopify_categories_customcategory_key,'value'=>'yes','value_type'=>'string','namespace'=>shopify_categories_namespace),
                array('key'=>shopify_categories_homecategories_key,'value'=>$isHomeCategory,'value_type'=>'string','namespace'=>shopify_categories_namespace),
            )
        )
    );


    $collection_results = $client->call('POST', '/admin/custom_collections.json', $postData);
    //the id is returned if successful
    $collectionShopifyID = $collection_results['id'];
    //save the id back into the node

    $node_wrapper->field_shopify_collection_id->set($collectionShopifyID);
    node_save($node);


}

/**
 * this is performed after all new collections are added and created properly
 * it updates the meta fields where the category id is stored.
 */
function shopify_categories_syncCateogryLinks()
{

}

/**
 * sync shopify collections with the
 */
function shopify_categories_sync()
{
    shopify_categories_push();
    return "<h3>Configurations and Collections are now sync'd.</h3>";
}

function shopify_categories_getNonBaseCollections()
{
    $nonbase_collections = array();

    //query shopify to get the base collections
    //initialize the client object
    $client = shopify_api_client();
    $collection_results = $client->call('GET', '/admin/custom_collections.json', array('limit' => 250, 'fields' => 'id, metafield, title'));

    //print_r($collection_results);die;

    foreach($collection_results as $collection) {
        $collectionID = $collection['id'];
        $collectionTitle = $collection['title'];
        $isBaseCategory = true;
        $drupalID = '';

        if (array_key_exists('metafields',$collection))
            $metaDatas = $collection['metafields'];
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
 * get the base collections for shopify system
 */
function shopify_categories_getBaseCollections()
{
    $base_collections = array();

    //query shopify to get the base collections
    //initialize the client object
    $client = shopify_api_client();
    $collection_results = $client->call('GET', '/admin/custom_collections.json', array('limit' => 250, 'fields' => 'id, metafield, title'));

    //print_r($collection_results);die;

    foreach($collection_results as $collection) {
        $collectionID = $collection['id'];
        $collectionTitle = $collection['title'];
        $isBaseCategory = true;


        if (array_key_exists('metafields',$collection))
            $metaDatas = $collection['metafields'];
        if(isset($metaDatas))
            foreach($metaDatas as $metafield)
            {
                if($metafield['key']==shopify_categories_customcategory_key)
                {
                    $isBaseCategory = false;
                    break;
                }
            }
        if ($isBaseCategory)
        {
            $base_collections[$collectionID] = $collectionTitle;
        }

    }
    return $base_collections;
}


?>
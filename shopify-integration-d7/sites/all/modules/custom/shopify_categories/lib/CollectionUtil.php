<?php
/**
 * Created by PhpStorm.
 * User: humingtang
 * Date: 6/5/16
 * Time: 9:35 PM
 */


/**
 * delete existing collection
 * @param $node
 */
function shopify_categories_deleteCollection($node)
{
    try
    {
        $node_wrapper = entity_metadata_wrapper('node',$node);
        $collectionID = $node_wrapper->field_shopify_collection_id->value();

        //initialize the client object
        $client = shopify_api_client();
        $collection_results = $client->call('DELETE', '/admin/custom_collections/'. $collectionID .'.json');
    }
    catch(Exception $e)
    {
        //do nothing
    }
}

/**
 * update an existing collection
 * @param $node
 */
function shopify_categories_updateCollection($node)
{
    //get collection ID
    $node_wrapper = entity_metadata_wrapper('node',$node);
    $collectionID = $node_wrapper->field_shopify_collection_id->value();

    //retrieve all subcategories
    $subCategories = shopify_categories_getSubCollectionMetaData($node);

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
    $subcategoryIDs = shopify_categories_getSubCollectionMetaData($node);

    $rawimgpath = $node_wrapper->field_sub_categories_image->file->value()->uri;

    $imagePath = file_create_url($rawimgpath);
    $imagePath = str_replace("/","\/" ,$imagePath);


    $postData = array('custom_collection' =>
        array(
            'title' => $nodetitle,
            'body' => $nodebody,
            'id' => $collectionID,
            'template_suffix' => 'nested-collection',
            //this only works if the drupal instance is available publically.
            'image' => array('src' => $imagePath),
            'metafields' => array(
                array('key'=>shopify_categories_drupal_key,'value'=>$nodeID,'value_type'=>'string','namespace'=>shopify_categories_namespace),
                array('key'=>shopify_categories_customcategory_key,'value'=>'yes','value_type'=>'string','namespace'=>shopify_categories_namespace),
                array('key'=>shopify_categories_homecategories_key,'value'=>$isHomeCategory,'value_type'=>'string','namespace'=>shopify_categories_namespace),
                array('key'=>shopify_categories_subcategories_key,'value'=>$subCategories,'value_type'=>'string','namespace'=>shopify_categories_namespace),
            )
        )
    );


    $client = shopify_api_client();
    //clean metafields
    shopify_categories_removeMetaFields($collectionID);
    //save results to shopify
    $collection_results = $client->call('PUT', '/admin/custom_collections/'. $collectionID .'.json', $postData);

}

/**
 * Create a new Collection
 * @param $node
 * @throws ShopifyApiException
 */
function shopify_categories_createNewCollection(&$node)
{
    //initialize the client object
    $client = shopify_api_client();
    $node_wrapper = entity_metadata_wrapper('node',$node);

    //retrieve all subcategories
    $subCategories = shopify_categories_getSubCollectionMetaData($node);

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
    $subcategoryIDs = shopify_categories_getSubCollectionMetaData($node);
    $rawimgpath = $node_wrapper->field_sub_categories_image->file->value()->uri;
    $imagePath = file_create_url($rawimgpath);
    $imagePath = str_replace("/","\/" ,$imagePath);

    $postData = array('custom_collection' =>
        array(
            'title' => $nodetitle,
            'body' => $nodebody,
            'template_suffix' => 'nested-collection',
            //this only works if the drupal instance is available publically.
            'image' => array('src' => $imagePath),

            'metafields' => array(
                array('key'=>shopify_categories_drupal_key,'value'=>$nodeID,'value_type'=>'string','namespace'=>shopify_categories_namespace),
                array('key'=>shopify_categories_customcategory_key,'value'=>'yes','value_type'=>'string','namespace'=>shopify_categories_namespace),
                array('key'=>shopify_categories_homecategories_key,'value'=>$isHomeCategory,'value_type'=>'string','namespace'=>shopify_categories_namespace),
                array('key'=>shopify_categories_subcategories_key,'value'=>$subCategories,'value_type'=>'string','namespace'=>shopify_categories_namespace),
            )
        )
    );


    $collection_results = $client->call('POST', '/admin/custom_collections.json', $postData);
    //the id is returned if successful
    $collectionShopifyID = $collection_results['id'];
    //save the id back into the node

    $node_wrapper->field_shopify_collection_id->set($collectionShopifyID);
    field_attach_update('node',$node);
    //node_save($node);


}

/**
 * update the metafields cheezily
 *
 * @param $metalfields
 * @param $collectionID
 */
function shopify_categories_removeMetaFields($collectionID)
{
    //initialize the client object
    $client = shopify_api_client();

    //delete all meta fields for this collection
    $metaDatas = shopify_categories_getMetaFieldsByCollectionID($collectionID);

    foreach($metaDatas as $metafield)
    {
        $metaFieldID = $metafield['id'];
        $collection_results = $client->call('DELETE', '/admin/metafields/'. $metaFieldID.'.json');
    }

}

function shopify_categories_getSubCollectionMetaData($node)
{
    $result = array();
    $subcollections = $node->field_shopify_collection['und'];

    foreach($subcollections as $subcollection)
    {
        $itemID = $subcollection['value'];
        $collection = entity_load('field_collection_item', array($itemID));
        $result[] = ($collection[$itemID]->field_shopify_data['und'][0]['value']);
    }

    $resultString = implode(",",$result);
    return $resultString;

}
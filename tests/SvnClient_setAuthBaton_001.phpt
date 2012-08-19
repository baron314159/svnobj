--TEST--
Validation on setAuthBaton
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
try { 
	$client->setAuthBaton(new stdClass());
} catch (Exception $e) {
	echo get_class($e);
}
?>
--EXPECT--
SvnInvalidArgumentException

--TEST--
Validation on setProgressCallback
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
try { 
	$client->setProgressCallback(123);
} catch (Exception $e) {
	echo get_class($e);
}
?>
--EXPECT--
SvnInvalidArgumentException

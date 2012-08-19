--TEST--
auth_providers contains invalid element
--FILE--
<?php
include 'bootstrap.php';
try {
	$baton = new SvnAuthBaton(array(1));
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}
?>
--EXPECT--
auth_providers must contain only SvnAuthProvider instances
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>

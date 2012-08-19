--TEST--
Cancel callback works
--FILE--
<?php
include 'bootstrap.php';
function cancel_callback($baton) { var_dump($baton); return true; }
$client = new SvnClient();
set_test_auth_baton($client);
$client->setCancelBaton('123');
$client->setCancelCallback('cancel_callback');
try {
	$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
} catch (Exception $e) {
	echo $e->getMessage(),"\n";
}
?>
--EXPECT--
string(3) "123"
Operation was cancelled by user
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>

--TEST--
Update return value (3)
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);

$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR, null, 
	array(SvnClient::OPT_REVISION_NUMBER, 1));
$r = $client->update(TEST_REPO_CO_DIR, 
	array(SvnClient::OPT_REVISION_NUMBER, 3));
echo $r;
?>
--EXPECT--
3
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>

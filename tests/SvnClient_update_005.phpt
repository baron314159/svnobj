--TEST--
Update return value (1)
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);

$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR, null, 
	array(SvnClient::OPT_REVISION_NUMBER, 1));
$r = $client->update(array(TEST_REPO_CO_DIR), 
	array(SvnClient::OPT_REVISION_NUMBER, 2));
printf("%d %d", count($r), $r[0]);
?>
--EXPECT--
1 2
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>

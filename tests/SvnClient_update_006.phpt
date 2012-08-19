--TEST--
Update return value (2)
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);

$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR, null, 
	array(SvnClient::OPT_REVISION_NUMBER, 1));
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR.'2', null, 
	array(SvnClient::OPT_REVISION_NUMBER, 1));
$r = $client->update(array(TEST_REPO_CO_DIR, TEST_REPO_CO_DIR.'2'), 
	array(SvnClient::OPT_REVISION_NUMBER, 2));
printf("%d %d %d", count($r), $r[0], $r[1]);
?>
--EXPECT--
2 2 2
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>

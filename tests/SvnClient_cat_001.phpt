--TEST--
Cat a file via URL
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
echo $client->cat(TEST_REPO_URL . '/' . 'sports/soccer');
?>
--EXPECTF--
soccer
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>

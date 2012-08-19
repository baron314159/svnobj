--TEST--
Merge changes between files
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
list($file1_rel, $file1) = uniq_file_put_contents("line1");
list($file2_rel, $file2) = uniq_file_put_contents("line1\nline2");
list($file3_rel, $file3) = uniq_file_put_contents("line1");
$client->add($file1);
$client->add($file2);
$client->add($file3);
$rev = $client->commit(TEST_REPO_CO_DIR, "msg");
cleanup();
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
$client->merge($file1, null, $file2, null, $file3);
echo file_get_contents($file3);
?>
--EXPECTF--
line1
line2
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>

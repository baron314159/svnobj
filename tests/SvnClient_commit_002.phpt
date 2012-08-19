--TEST--
Commit without log message callback
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
list($file_rel, $file) = uniq_file_put_contents('content');
$client->add($file);
$r = $client->commit(TEST_REPO_CO_DIR);
if (is_null($r) or (is_array($r) and $r['revision'] == -1)) echo '1';
?>
--EXPECT--
1
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>

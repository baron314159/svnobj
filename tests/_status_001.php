<?php
include 'bootstrap.php';

$client = new SvnClient();
set_test_auth_baton($client);

$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);

list($file_rel, $file) = uniq_file_put_contents('content');

$client->add($file);
$client->commit(TEST_REPO_CO_DIR, "log msg!");
echo $file,"\n";
cleanup();

$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
file_put_contents($file, 'updated content');

var_dump($client->status(TEST_REPO_CO_DIR));

?>

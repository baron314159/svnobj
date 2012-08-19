<?php
include 'bootstrap.php';

$client = new SvnClient();
set_test_auth_baton($client);

$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
list ($file_rel, $file) = uniq_file_put_contents('content');
$client->add($file);
$client->commit(TEST_REPO_CO_DIR, "committing");
cleanup();
var_dump($client->cat(TEST_REPO_URL . '/' . $file_rel));

?>

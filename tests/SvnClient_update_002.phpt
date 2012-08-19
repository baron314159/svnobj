--TEST--
Update multiple working copies to head 
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);

$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR, null, 1);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR.'2', null, 1);
echo_file_exists('numbers.txt');
echo_file_exists('numbers.txt', TEST_REPO_CO_DIR.'2');
$client->update(array(TEST_REPO_CO_DIR, TEST_REPO_CO_DIR.'2'));
echo_file_exists('numbers.txt');
echo_file_exists('numbers.txt', TEST_REPO_CO_DIR.'2');
?>
--EXPECT--
0
0
1
1
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>

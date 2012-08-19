--TEST--
Cancel callback works (2)
--FILE--
<?php
include 'bootstrap.php';
function cancel_callback($baton) { return false; }
$client = new SvnClient();
set_test_auth_baton($client);
$client->setCancelCallback('cancel_callback');
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
echo_file_contents('justaline.txt');
?>
--EXPECT--
this is a line
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>

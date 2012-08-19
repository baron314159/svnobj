--TEST--
svn_ls calling generates Assertion is_canonical failure (pecl 11305)
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
$result = $client->list(TEST_REPO_URL . '/sports/');
echo count($result),"\n";
?>
--EXPECT--
3
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>

--TEST--
Merge changes between files (peg)
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
list($file1_rel, $file1) = uniq_file_put_contents("line1");
list($file2_rel, $file2) = uniq_file_put_contents("line1");
$client->add($file1);
$client->add($file2);
$rev_info1 = $client->commit(TEST_REPO_CO_DIR, "msg");
file_put_contents($file1, "line1\nline2");
$rev_info2 = $client->commit(TEST_REPO_CO_DIR, "msg");
cleanup();
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
$client->mergePeg($file1, 
	array(SvnClient::OPT_REVISION_NUMBER, $rev_info1['revision']), 
	array(SvnClient::OPT_REVISION_NUMBER, $rev_info2['revision']), 
	array(SvnClient::OPT_REVISION_NUMBER, $rev_info2['revision']), 
	$file2);
echo file_get_contents($file2);
?>
--EXPECT--
line1
line2
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>

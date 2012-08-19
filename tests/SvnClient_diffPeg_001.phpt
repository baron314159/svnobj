--TEST--
Compute file differences (peg revision)
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
echo $client->diffPeg(
	sys_get_temp_dir() . DS . 'difftmp',
	TEST_REPO_URL . '/this_file_changes',
	array(SvnClient::OPT_REVISION_NUMBER, 5),
	array(SvnClient::OPT_REVISION_NUMBER, 5),
	array(SvnClient::OPT_REVISION_NUMBER, 6));
?>
--EXPECTF--
Index: this_file_changes
===================================================================
--- this_file_changes	(revision 5)
+++ this_file_changes	(revision 6)
@@ -1 +1,2 @@
 line 1
+line 2
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>

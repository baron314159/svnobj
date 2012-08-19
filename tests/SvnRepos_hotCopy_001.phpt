--TEST--
Hotcopy a repository
--FILE--
<?php
include 'bootstrap.php';
SvnRepos::create(TEST_NEW_REPO_DIR.'2');
SvnRepos::hotCopy(
	TEST_NEW_REPO_DIR.'2', 
	TEST_NEW_REPO_DIR, 
	true);
echo_repo_file_exists('README.txt');
?>
--EXPECT--
1
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>

--TEST--
Create a repository (fsfs)
--FILE--
<?php
include 'bootstrap.php';
$repos = SvnRepos::create(TEST_NEW_REPO_DIR);
echo_repo_file_exists('README.txt');
?>
--EXPECTF--
1
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>

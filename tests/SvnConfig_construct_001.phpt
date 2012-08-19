--TEST--
Construct SvnConfig object
--FILE--
<?php
include 'bootstrap.php';
$config = new SvnConfig(TEST_CFG_DIR);
echo_file_exists(TEST_CFG_DIR . DS . 'config');
?>
--EXPECTF--
1
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>

--TEST--
getVersion works
--FILE--
<?php
include 'bootstrap.php';
var_dump(SvnClient::getVersion());
?>
--EXPECTF--
array(4) {
  ["major"]=>
  int(%d)
  ["minor"]=>
  int(%d)
  ["patch"]=>
  int(%d)
  ["tag"]=>
  string(%d) %s
}

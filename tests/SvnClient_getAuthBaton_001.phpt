--TEST--
Check reference counting on getAuthBaton
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
$client->setAuthBaton(new SvnAuthBaton());
$baton = $client->getAuthBaton();
unset($client);
echo get_class($baton);
?>
--EXPECT--
SvnAuthBaton

<?php
include 'bootstrap.php';

$client = new SvnClient();
set_test_auth_baton($client);

var_dump($client->log($argv[1], null, null, 0, 0, true));

?>

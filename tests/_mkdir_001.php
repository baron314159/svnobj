<?php
include 'bootstrap.php';

function log_msg_callback($baton, $commit_items)
{
    var_dump($baton);
    var_dump($commit_items);

    echo "log prompt: ";

    return array(
        'log_msg'  => trim(fgets(STDIN)),
        'tmp_file' => "foobar"
    );
}

$client = new SvnClient();
set_test_auth_baton($client);
$client->setLogMsgCallback('log_msg_callback');
$client->setLogMsgBaton(101);
$dir_name = uniqid();
var_dump($client->mkdir(TEST_REPO_URL . '/' . $dir_name));
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
echo $dir_name,"\n";
?>

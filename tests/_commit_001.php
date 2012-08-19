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
$client->setProgressCallback('foo');

$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);

$rel_file_name = uniqid();
$file_name = TEST_REPO_CO_DIR . DS . $rel_file_name;
file_put_contents($file_name, 'content');

$client->add($file_name);
var_dump($client->commit(TEST_REPO_CO_DIR));
echo $file_name,"\n";
cleanup();

$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
?>

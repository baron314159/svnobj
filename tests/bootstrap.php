<?php
define('DS', DIRECTORY_SEPARATOR);
define('TEST_DIR', dirname(__FILE__));
define('TEST_REPO_URL', 'file:///tmp/test_repo');
define('TEST_REPO_RELOC_URL', 'file:///tmp/test_repo_reloc');
define('TEST_REPO_CO_DIR', '/tmp/test_repo_co');
define('TEST_CFG_DIR', '/tmp/test_config');

define('TEST_NEW_REPO_DIR', '/tmp/new_repo');
define('TEST_NEW_REPO_URL', 'file:///tmp/new_repo');

function username_prompt() {
    return array(
        'username' => 'test',
        'may_save' => false
    );
}

function set_test_auth_baton($client) {
    $baton = new SvnAuthBaton(array(
        SvnAuthProvider::createUsernamePromptProvider('username_prompt')
    ));
    $client->setAuthBaton($baton);
}

function cleanup() {
    shell_exec('rm -rf ' . TEST_REPO_CO_DIR . '*');
    shell_exec('rm -rf ' . TEST_CFG_DIR);
    shell_exec('rm -rf ' . TEST_NEW_REPO_DIR . '*');
}

function uniq_file_put_contents($content) {
    $file_rel = uniqid();
    $file = TEST_REPO_CO_DIR . DS . $file_rel;
    file_put_contents($file, $content);
    return array($file_rel, $file);
}

function echo_file_contents($file_name) {
    if ($file_name[0] == DS) {
        echo file_get_contents($file_name);
    } else {
        echo file_get_contents(TEST_REPO_CO_DIR . DS . $file_name);
    }
}

function echo_file_exists($file_name) {
    if ($file_name[0] == DS) {
        printf("%d\n", file_exists($file_name));
    } else {
        printf("%d\n", file_exists(TEST_REPO_CO_DIR . DS . $file_name));
    }
}

function echo_repo_file_exists($file_name) {
    if ($file_name[0] == DS) {
        printf("%d\n", file_exists($file_name));
    } else {
        printf("%d\n", file_exists(TEST_NEW_REPO_DIR . DS . $file_name));
    }
}

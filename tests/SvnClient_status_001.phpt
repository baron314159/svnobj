--TEST--
Status - updated file
--FILE--
<?php
include 'bootstrap.php';
$client = new SvnClient();
set_test_auth_baton($client);
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
list($file_rel, $file) = uniq_file_put_contents('content');
$client->add($file);
$client->commit(TEST_REPO_CO_DIR, "log msg!");
cleanup();
$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
file_put_contents($file, 'updated content');
var_dump($client->status(TEST_REPO_CO_DIR, true, false));
?>
--EXPECTF--
array(1) {
  [0]=>
  array(2) {
    ["path"]=>
    string(%d) "%s"
    ["status"]=>
    array(10) {
      ["entry"]=>
      array(31) {
        ["name"]=>
        string(%d) "%s"
        ["revision"]=>
        int(%d)
        ["url"]=>
        string(%d) "%s"
        ["repos"]=>
        string(%d) "%s"
        ["uuid"]=>
        string(%d) "%s"
        ["kind"]=>
        int(1)
        ["schedule"]=>
        int(0)
        ["copied"]=>
        bool(false)
        ["deleted"]=>
        bool(false)
        ["absent"]=>
        bool(false)
        ["incomplete"]=>
        bool(false)
        ["copyfrom_url"]=>
        NULL
        ["copyfrom_rev"]=>
        int(-1)
        ["conflict_old"]=>
        NULL
        ["conflict_new"]=>
        NULL
        ["conflict_wrk"]=>
        NULL
        ["prejfile"]=>
        NULL
        ["text_time"]=>
        string(%d) "%d %d"
        ["prop_time"]=>
        string(%d) "%d %d"
        ["checksum"]=>
        string(%d) "%s"
        ["cmt_rev"]=>
        int(%d)
        ["cmt_date"]=>
        string(%d) "%d %d"
        ["cmt_author"]=>
        string(%d) "%s"
        ["lock_token"]=>
        NULL
        ["lock_owner"]=>
        NULL
        ["lock_comment"]=>
        NULL
        ["lock_creation_date"]=>
        string(%d) "%d %d"
        ["has_props"]=>
        bool(%s)
        ["has_prop_mods"]=>
        bool(%s)
        ["cachable_props"]=>
        string(%d) "%s"
        ["present_props"]=>
        NULL
      }
      ["locked"]=>
      bool(false)
      ["copied"]=>
      bool(false)
      ["switched"]=>
      bool(false)
      ["repos_lock"]=>
      NULL
      ["url"]=>
      string(%d) "%s"
      ["ood_last_cmt_rev"]=>
      int(-1)
      ["ood_last_cmt_date"]=>
      string(%d) "%d %d"
      ["ood_kind"]=>
      int(0)
      ["ood_last_cmt_author"]=>
      NULL
    }
  }
}
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>

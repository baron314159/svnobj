--TEST--
Throwing an exception in prompt_callback works
--FILE--
<?php
include 'bootstrap.php';
function prompt_callback() {
	throw new Exception('foo');
}
$authBaton = new SvnAuthBaton(array(
	SvnAuthProvider::createUsernamePromptProvider('prompt_callback')
));
$client = new SvnClient();
$client->setAuthBaton($authBaton);
try {
	$client->checkout(TEST_REPO_URL, TEST_REPO_CO_DIR);
} catch(Exception $e) {
	echo $e->getMessage(), "\n";
}
?>
--EXPECT--
foo
--CLEAN--
<?php include 'bootstrap.php'; cleanup(); ?>

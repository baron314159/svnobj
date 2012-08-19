<?php

$paths = explode(PATH_SEPARATOR, getenv('PATH'));

foreach ($paths as $path) {
    $bin_path = $path . DIRECTORY_SEPARATOR . 'php';

    if (is_executable($bin_path)) {
        putenv("TEST_PHP_EXECUTABLE={$bin_path}");
        passthru("php run-tests.php " . implode(' ', array_map('escapeshellarg', array_slice($argv, 1))));
        break;
    }
}

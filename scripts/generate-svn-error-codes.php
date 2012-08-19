<?php

$input = file_get_contents('php://stdin');

$num_matches = preg_match_all('/SVN_ERRDEF\(([a-zA-Z0-9_]{4,})/',
    $input,
    $all_captures,
    PREG_SET_ORDER);

foreach ($all_captures as $captures) {
    $svn_const = $captures[1];
    $php_const = substr($svn_const, 4);
    echo "SVNCLIENT_CONST_LONG({$php_const}, {$svn_const});\n";
}

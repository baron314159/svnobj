<?php

$client = new SvnClient();

function progress_callback($progress, $total, $baton)
{
    echo "$progress $total $baton\n";
}

$client->setProgressCallback('progress_callback');
$client->setProgressBaton('hi');

$diff = $client->diffPeg('/tmp/diff',
    'http://svnobj.googlecode.com/svn/trunk',
    SvnClient::OPT_REVISION_HEAD,
    array(SvnClient::OPT_REVISION_NUMBER, 48),
    array(SvnClient::OPT_REVISION_NUMBER, 49));

header('Content-type: text/plain');
var_dump($diff);

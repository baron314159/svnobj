--TEST--
Construct an SvnLibraryException
--FILE--
<?php
$e = new SvnLibraryException();
echo get_class($e),"\n";
?>
--EXPECT--
SvnLibraryException

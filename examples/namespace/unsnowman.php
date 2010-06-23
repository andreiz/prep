#!/usr/bin/env php
<?php
/**
 * @see snowman.php
 */
if (isset($argv[1])) {
	$source = $argv[1];
} else {
	$source = 'php://stdin';
}
echo str_replace('☃', '\\', file_get_contents($source));

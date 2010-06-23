#!/usr/bin/env php
<?php
/**
 * @see input.php
 */
echo str_replace("FOO", "BAR", file_get_contents($argv[1]));

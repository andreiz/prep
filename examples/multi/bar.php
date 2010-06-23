#!/usr/bin/env php
<?php
/**
 * @see input.php
 */
echo str_replace("BAR", "OINK", file_get_contents($argv[1]));

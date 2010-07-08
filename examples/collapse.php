#!/usr/bin/env php
<?php

$source = file_get_contents($argv[1]);

$source = preg_replace('!^\s+$!', '', $source);
$source = preg_replace('!\n+!', "\n", $source);
echo $source;

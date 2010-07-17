#!/usr/bin/env php
<?php
$source = file_get_contents($argv[1]);
$img_base = dirname(__FILE__);
$img_rgxp = '!<img\s*src="([^"]+)"[^>]+>!';
$images = preg_match_all($img_rgxp, $source, $match, PREG_SET_ORDER);
if (!$images) {
	echo $source;
	exit;
}

$find = array();
$replace = array();
foreach ($match as $entry) {
	$path = $img_base.'/'.$entry[1];
	$info = getimagesize($path);
	$size = filesize($path);
	if ($info[0] * $info[1] > 512) {
		continue;
	}

	$data = file_get_contents($path);
	$data = base64_encode($data);
	$dataURL = sprintf('<img src="data:%s;base64,%s">', $info['mime'], $data);
	$find[] = $entry[0]; 
	$replace[] = $dataURL;
}

$source = str_replace($find, $replace, $source);
echo $source;

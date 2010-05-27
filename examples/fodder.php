<?php

function DOSTUFF()
{
	echo "doing stuff...\n";
}

for ($i=1; $i<=10; $i++) {
	echo "{$i}\n";
	/*<remove:*/DOSTUFF();/*:remove>*/
}


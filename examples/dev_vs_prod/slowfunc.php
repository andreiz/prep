<?php
/**
 * This example illustrates a poorly-performing bit of code that a developer 
 * might want on the development server, but certainly wouldn't want to be 
 * executed in the production environment.
 *
 * Without prep (development environment):
 *  php slowfunc.php
 *
 * With prep (production environment):
 *  php php -d'extension=prep.so' -d'prep.command=/path/to/prep/examples/dev_vs_prod/remove.php' slowfunc.php
 *
 * The DOSTUFF() function is never actually called on production.
 */

function DOSTUFF()
{
	echo "doing stuff...\n";
	sleep(1);
}

for ($i=1; $i<=10; $i++) {
	echo "{$i}\n";
	/*<remove:*/DOSTUFF();/*:remove>*/
}


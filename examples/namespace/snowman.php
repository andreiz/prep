<?php
/**
 * There's been a lot of complaining in the PHP world about the choice of 
 * backslash (\) as the namespace operator. Most of the complainers don't 
 * understand the reasons *why* backslash was chosen, but nevertheless, we have
 * a solution. Just use unsnowman.php as a prep command, and you can use the 
 * snowman character ( ☃ ) as the namespace separator:
 *  php php -d'extension=prep.so' -d'prep.command=/path/to/prep/examples/namespace/unsnowman.php' snowman.php
 *
 * Note: this actually works *without* prep, but not as a nested namespace (the
 * snowman characters are actually _part_ of the namespace, not separators).
 */
namespace i☃am☃a☃bad☃developer;
class Bad_Developer
{
	public function __construct()
	{
		echo "We're bad people.\n";
	}
}

new Bad_Developer;

<?php
/**
 * Demo of using multiple prep commands with one input.
 *
 * This file is fed to wrapper, which feeds the input to foo.php and stores the
 * output in a temporary file.
 *
 * The temporary file is then fed into bar.php, and the output is fed back to 
 * PHP.
 *
 * Multple prep commands can be chained in this manner.
 *
 *  php -d'extension=prep.so' -d'prep.command=/path/to/prep/examples/multi/wrapper' examples/multi/input.php
 */
echo "Here's the word: FOO\n";

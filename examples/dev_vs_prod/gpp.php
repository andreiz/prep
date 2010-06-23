<?php
/**
 * This example uses GPP (Generic Preprocessor) as the prep command
 * (http://en.nothingisreal.com/wiki/GPP)
 *
 * There are two examples herein:
 * 1) ifdef, the same as it would work in C-land
 *  i) call php + prep + gpp in the dev environment like this:
 *   php -d'extension=prep.so' -d'prep.command="gpp -D DEV=1 -C"' /path/to/prep/examples/dev_vs_prod/gpp.php
 *  ii) call php + prep + gpp in the production environment like this:
 *   php -d'extension=prep.so' -d'prep.command="gpp -C"' /path/to/prep/examples/dev_vs_prod/gpp.php
 *
 * 2) Macros! This file contains a macro for DEVSLEEP, which could be set 
 * conditionally with another ifdef/else/endif block
 */


echo "Environment: ";
#ifdef DEV
echo "dev";
#else
echo "production";
#endif
echo "\n";

#define DEVSLEEP(n) sleep(n)
DEVSLEEP(1);


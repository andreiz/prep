<?php
echo "Environment: ";
#ifdef DEV
echo "dev";
#else
echo "production";
#endif
echo "\n";

#define DEVSLEEP(n) sleep(n)
DEVSLEEP(1);


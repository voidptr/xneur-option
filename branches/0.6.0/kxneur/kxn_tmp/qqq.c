#include <stdio.h>

main ()
{
    char s[1024];
    FILE *fl;
    int n;
    char *cmd = "xwininfo";
    // char *cmd = "perl -e 'open F,\"xwininfo -root -children |\";my %l;while(<F>){if(/\\(\\\"(\\w+)\\\"/){$l{$1}=0}}close F;for(sort keys %l){print\"$_\\n\"}'";
    if ( fl = popen(cmd, "r") ) {
	int i = 0;
	while ( fgets(s, n, fl) ) {
	    printf("%d - %s", i, s);
	    i++;
	}
	pclose(fl);
    }
    return 0;
}

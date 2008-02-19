#!/usr/bin/perl -w

open F,"xwininfo -root -children |";

my %l;
while(<F>){
    if( /\"(\w+)\"\)/ ){
	$l{$1}=0
    }
}
close F;
for(sort keys %l){
    print "$_\n"
}


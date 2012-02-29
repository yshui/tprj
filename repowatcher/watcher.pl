#!/usr/bin/perl -w
use strict;
open RW, "/home/shui/.utils/rw $ARGV[0] |";
while(<RW>){
	if($_ =~ /^C (.*)$/){
		my $name = $1;
		print "$name\n";
		if($name =~ /\.pkg\.tar\.[^.]+$/){
			`repo-add $ARGV[1] $ARGV[0]/$name`;
		}
	}
	if($_ =~ /^D (.*)$/){
		my $name = $1;
		if($name =~ /\.pkg\.tar\.[^.]+$/){
			$name =~ s/-(x86_64|i686|any)\.pkg\.tar\.[^.]+$//;
			#print "$name\n";
			my @e = `bsdtar -xOqf $ARGV[1] $name/desc`;
			#print "@e\n";
			$name = "";
			for my $i (1..$#e){
				if($e[$i] =~ /%NAME%/){
					$name = $e[$i+1];
					last;
				}
			}
			#print $name,"\n";
			if($name eq ""){
				next;
			}
			`repo-remove $ARGV[1] $name`;
		}
	}
}

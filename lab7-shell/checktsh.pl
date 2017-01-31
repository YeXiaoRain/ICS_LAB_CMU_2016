#!/usr/bin/perl
#!/usr/local/bin/perl
use Getopt::Std;
use FileHandle;
use IPC::Open2;

####################################################################
# checktsh.pl - compare the output from tsh to the reference solution
#
# Copyright (c) 2002, R. Bryant and D. O'Hallaron, All rights reserved.
# May not be used, modified, or copied without permission.
#
####################################################################

# Always flush stdout and stderr
STDIN->autoflush();
STDERR->autoflush();

#
# usage - Print help message and terminate
#
sub usage 
{
    printf STDERR "$_[0]\n";
    printf STDERR "Usage: $0 [-hve] -t <tracenum>\n";
    printf STDERR "Options:\n";
    printf STDERR "  -h              Print this message\n";
    printf STDERR "  -t <tracefile>  Check one <tracefile> (default: check all)\n";
    printf STDERR "  -v              Trace our progress\n";
    printf STDERR "  -e              Like -v, but trace output only on error\n";
    die "\n" ;
}

#
# errexit - Print the tsh output (if -e option) and die
#
sub errexit 
{
    my ($line);

    if ($etrace) {
	print "Reference output:\n";
	open(EFILE, "$tshreffile")
	    or die "$0: ERROR: Couldn't open $tshreffile for output (errexit)\n";
	while ($line = <EFILE>) {
	    print $line;
	}
	close (EFILE);
	print "Student's output:\n";
	open(EFILE, "$tshfile")
	    or die "$0: ERROR: Couldn't open $tshfile for output (errexit)\n";
	while ($line = <EFILE>) {
	    print $line;
	}
	close (EFILE);
    }
    die "\n";
}

sub check_trace11 {
    my($tshreflineold);
    my($tshlineold);
    my($tshrefline);
    my($tshline);
    my($splitcount);

    while ($tshreflineold = <TSHREFFILE>) {
	do {
       	   $tshlineold = <TSHFILE>;
     	   $tshlineold =~s/\x0A//g;
	   $tshlineold =~s/\x0D//g;
        } while( $tshlineold eq "" && not eof);

	chomp($tshreflineold);
	chomp($tshlineold);

	$tshrefline = $tshreflineold;
	$tshline = $tshlineold;

	$tshrefline =~ s/\(\d+\)/(PID)/;
	$tshline =~ s/\(\d+\)/(PID)/;
	
	$tshrefline =~s/\t+/ /g;
	$tshline =~s/\t+/ /g;

	$tshrefline =~s/\x20+/ /g;
	$tshline =~s/\x20+/ /g;

	if ($tshrefline ne $tshline) {
	    if (!$etrace) {
		print "$0: ERROR: Reference output (ref) differs from yours (tsh):\n";
		print " ref:$tshrefline\n";
		print " tsh:$tshline\n";
	    }
	    errexit();
	}

	if ($tshline =~ /tsh> \/bin\/ps/) {
	    last;
	}
    }

    $psoutput = "";
    $splitcount = 0;
    while ($tshline = <TSHFILE>) {
	$psoutput = $psoutput.$tshline;
	if ($tshline =~ /mysplit/) {
	    $splitcount++
	}
    }

    if ($splitcount > 0) {
	if (!$etrace) {
	    print "$0: ERROR: Your tsh didn't kill the foreground mysplit process:\n";
	    print "$psoutput";
	    print "Kill it with 'kill -9 PID' before rerunning $0\n";
	}
	errexit();
    }
}

sub check_trace12 {
    my($tshreflineold);
    my($tshlineold);
    my($tshrefline);
    my($tshline);
    my($splitcount);

    while ($tshreflineold = <TSHREFFILE>) {
	do {
       	   $tshlineold = <TSHFILE>;
     	   $tshlineold =~s/\x0A//g;
	   $tshlineold =~s/\x0D//g;
        } while( $tshlineold eq "" && not eof);

	chomp($tshreflineold);
	chomp($tshlineold);

	$tshrefline = $tshreflineold;
	$tshline = $tshlineold;

	$tshrefline =~ s/\(\d+\)/(PID)/;
	$tshline =~ s/\(\d+\)/(PID)/;
	
	$tshrefline =~s/\t+//g;
	$tshline =~s/\t+//g;

	$tshrefline =~s/\x20+//g;
	$tshline =~s/\x20+//g;

	if ($tshrefline ne $tshline) {
	    if (!$etrace) {
		print "$0: ERROR: Reference output (ref) differs from yours (tsh):\n";
		print " ref:$tshrefline\n";
		print " tsh:$tshline\n";
	    }
	    errexit();
	}

	if ($tshlineold =~ /tsh> \/bin\/ps/) {
	    last;
	}
    }

    $psoutput = "";
    $splitcount = 0;
    $stoppedcount = 0;
    while ($tshline = <TSHFILE>) {
	$psoutput = $psoutput.$tshline;
	if ($tshline =~ /mysplit/) {
	    $splitcount++;
	    if ($tshline =~ /\d+ .* T .*:.* .*mysplit/) {
		$stoppedcount++;
	    }
	}
    }

    
    if ($splitcount != 2) {
	print "$0: ERROR: Expected 2 mysplit processes. Got $splitcount instead:\n";
	errexit();
    }
    elsif ($stoppedcount != 2) {
	print "$0: ERROR: Expected 2 stopped mysplit processes (STAT = T). Got $stoppedcount instead:\n";
	errexit();
    }

}

sub check_trace13 {
    my($tshreflineold);
    my($tshlineold);
    my($tshrefline);
    my($tshline);
    my($splitcount);
    my($expectedline);

    while ($tshreflineold = <TSHREFFILE>) {

	do {
       	   $tshlineold = <TSHFILE>;
     	   $tshlineold =~s/\x0A//g;
	   $tshlineold =~s/\x0D//g;
        } while( $tshlineold eq "" && not eof);

	chomp($tshreflineold);
	chomp($tshlineold);

	$tshrefline = $tshreflineold;
	$tshline = $tshlineold;

	$tshrefline =~ s/\(\d+\)/(PID)/;
	$tshline =~ s/\(\d+\)/(PID)/;
	
	$tshrefline =~s/\t+//g;
	$tshline =~s/\t+//g;

	$tshrefline =~s/\x20+//g;
	$tshline =~s/\x20+//g;

	if ($tshrefline ne $tshline) {
	    if (!$etrace) {
		print "$0: ERROR: Reference output (ref) differs from yours (tsh):\n";
		print " ref:$tshrefline\n";
		print " tsh:$tshline\n";
	    }
	    errexit();
	}

	if ($tshlineold =~ /bin\/ps/) {
	    last;
	}
    }

    $psoutput = "";
    $splitcount = 0;
    $stoppedcount = 0;
    $done = 0;
    while (!$done and $tshline = <TSHFILE>) {
	$psoutput = $psoutput.$tshline;
	if ($tshline =~ /mysplit/) {
	    $splitcount++;
	    if ($tshline =~ /\d+ .* T .*:.* .*mysplit/) {
		$stoppedcount++;
	    }
	}
	if ($tshline =~ /tsh>/) {
	    $done = 1;
	    last;
	}
    }

    if ($splitcount != 2) {
	print "$0: ERROR: Expected 2 mysplit processes in the first ps. Got $splitcount instead:\n";
	errexit();
    }
    elsif ($stoppedcount != 2) {
	print "$0: ERROR: Expected 2 stopped mysplit processes (STAT = T) in the first ps. Got $stoppedcount instead:\n";
	errexit();
    }

    $expectedline = "tsh> fg %1\n";
    if ($tshline ne $expectedline) {
	if (!$etrace) {
	    print "$0: ERROR: Expected an fg %1 command after the first ps output:\n";
	    print " ref:$expectedline\n";
	    print " tsh:$tshline\n";
	}
	errexit();
    }

    $tshline = <TSHFILE>;
    $expectedline = "tsh> /bin/ps a\n";
    if ($tshline ne $expectedline) {
	if (!$etrace) {
	    print "$0: ERROR: Expected a ps command after the fg command:\n";
	    print " ref:$expectedline";
	    print " tsh:$tshline";
	}
	errexit();
    }

    
    $psoutput = "";
    $splitcount = 0;
    while ($tshline = <TSHFILE>) {
	$psoutput = $psoutput.$tshline;
	if ($tshline =~ /mysplit/) {
	    $splitcount++;
	}
    }

    if ($splitcount != 0) {
	print "$0: ERROR: Expected 0 mysplit processes in second ps output. Got $splitcount instead:\n";
	errexit();
    }
}


sub check_trace {

    my $tracefile = $_[0];
    my $driver = "./sdriver.pl";
    my $tsh = "./tsh";
    my $tshref = "./tshref";
    my $tmpdir = "/tmp/tsh$$";

    # Had to make these global for errexit() ... Ugh
    $tshreffile = "$tmpdir/tshref.out";
    $tshfile = "$tmpdir/tsh.out";

    if ($verbose) {
	print "\n**************************************\n";
	print "* $0: Checking $tracefile...\n";
	print "**************************************\n";
    }
    else {
	print "Checking $tracefile...\n";
    }

    (-e $tsh and -x $tsh) 
	or die "$0: ERROR: $tsh not found or not executable\n";
    (-e $tshref and -x $tshref) 
	or die "$0: ERROR: $tshref not found or not executable\n";

    system("rm -rf $tmpdir/*; mkdir $tmpdir") == 0
	or die "$0: ERROR: Couldn't create $tmpdir directory\n";
    
    if ($verbose) {
	printf "\n$0: Running reference shell on $tracefile...\n";
    }
    open(TSHREFFILE, ">$tshreffile")
	or die "$0: ERROR: Couldn't open $tshreffile for output\n";
    open(TSHREF, "$driver -t $tracefile -s $tshref -a '-p'|")
	or die "$0: ERROR: Couldn't run driver on $tshref\n";
    while ($line = <TSHREF>) {
	if ($verbose) {
	    print $line;
	}
	print TSHREFFILE "$line"; 
    } 
    close TSHREF;

    if ($verbose) {
	printf "\n$0: Running your shell on $tracefile...\n";
    }

    open(TSHFILE, ">$tshfile")
	or die "$0: ERROR: Couldn't open $tshfile for output\n";
    open(TSH, "$driver -t $tracefile -s $tsh -a '-p'|")
	or die "$0: ERROR: Couldn't run driver on $tsh\n";
    while ($line = <TSH>) {
	if ($verbose) {
	    print $line;
	}
	print TSHFILE "$line"; 
    } 
    close TSH;
    
    if ($verbose) {
	printf "\n$0: Comparing reference outputs to your outputs...\n";
    }
    open(TSHREFFILE, "$tshreffile")
	or die "$0: ERROR: Couldn't open $tshreffile for input\n";
    open(TSHFILE, "$tshfile")
	or die "$0: ERROR: Couldn't open $tshfile for input\n";


    if ($tracefile eq "trace11.txt") {
	check_trace11();
    }
    elsif ($tracefile eq "trace12.txt") {
	check_trace12();
    }
    elsif ($tracefile eq "trace13.txt") {
	check_trace13();
    }
    else {
	while ($tshreflineold = <TSHREFFILE>) {
	    
	    do {
		#print "abc\n";
            	$tshlineold = <TSHFILE>;
     	    	$tshlineold =~s/\x0A//g;
	    	$tshlineold =~s/\x0D//g;
            } while( $tshlineold eq "" && not eof) ;
	   
	    chomp($tshreflineold);
	    chomp($tshlineold);
	    
	    $tshrefline = $tshreflineold;
	    $tshline = $tshlineold;
   
	    $tshrefline =~s/\(\d+\)/(PID)/;
	    $tshline =~s/\(\d+\)/(PID)/;

	    $tshrefline =~s/\t+//g;
	    $tshline =~s/\t+//g;

	    $tshrefline =~s/\x20+//g;
	    $tshline =~s/\x20+//g;

	    if ($tshrefline ne $tshline) {
		if (!$etrace) {
		    print "$0: ERROR: Reference output (ref) differs from yours (tsh):\n";
		    print " ref:$tshrefline\n";
		    print " tsh:$tshline\n";
		}
		errexit();
	    }
	    
	}
    }
    
    
    close(TSHFILE);
    close(TSHREFFILE);
    
    # clean up
    system("rm -rf $tmpdir") == 0
	or die "$0: ERROR: Couldn't delete $tmpdir\n";
}

##############
# Main routine
##############

getopts('hevt:');
if ($opt_h) {
    usage();
}
$verbose = $opt_v;
$etrace = $opt_e;

$tmpdir = "/tmp/tsh$$";

if ($opt_t) {
    check_trace($opt_t);
}
else {
    foreach $tracefile ("trace01.txt", "trace02.txt", "trace03.txt", 
			"trace04.txt", "trace05.txt", "trace06.txt", 
			"trace07.txt", "trace08.txt", "trace09.txt", 
			"trace10.txt", "trace11.txt", "trace12.txt", 
			"trace13.txt", "trace14.txt", "trace15.txt",
			"trace16.txt") {
	check_trace($tracefile);
    }
}

exit;

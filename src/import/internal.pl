#!/bin/perl -w
# open file, find line, replace with special text
use warnings;

print "Creating class in masa_internal.h...\n";

# no spaces
# or special characters
# would recommend avoiding capital letters as well
# can use underscore

$name= "nicks_solution_class";
$varf = "variables.var";

# open file(s)
$old   = "../masa_internal.h";
$new   = "tmp";
$bak   = "masa_internal.bak";
$count = 0;

open INFILE , "<", $old or die $!;
open OUTFILE, ">", $new  or die $!;

# down to business...

# replace
while($line = <INFILE>)
{
    # error check
    if($line =~ /class $name/)
    {
	print "MASA IMPORT CRITICAL ERROR: Solution of that name already exists!\n";
	print "$name has already been registered!\n";
	exit 1;
    }

    # looking for our special moniker
    if($line =~ /-l33t-/)
    {
	# print header
	print OUTFILE "// ------------------------------------------------------\n";
	print OUTFILE "// --------------- $name \n";
	print OUTFILE "// ------------------------------------------------------\n";

	# print template and class
	print OUTFILE "template <typename Scalar>\n";
	print OUTFILE "class $name : public manufactured_solution<Scalar>\n{\n";

	# print pi
	print OUTFILE "  using manufactured_solution<Scalar>::pi;\n";
	print OUTFILE "  using manufactured_solution<Scalar>::PI;\n\n";
	
	# print list of variables
	print OUTFILE "private:\n";

	# open file, get list of variables, populate list here:
	open VARFILE , "<", $varf or die $!;
	while($vf = <VARFILE>)
	{	    
	    chomp($vf);
	    @values = split(' ', $vf);
	    print OUTFILE "  Scalar $values[0];\n";
	}
	close  VARFILE or die $!;
	print OUTFILE "\npublic:\n  $name();\n  int init_var();\n";

	# print list of source terms and analytical functions
	# (error check here to ensure it fits a source term we support)

	# leave moniker in and trip counter
	print OUTFILE "\n";
	print OUTFILE $line;
	$count=1;
    }
    else
    {
	print OUTFILE $line;
    }

}

if($count =~ 0)
{
    print "MASA IMPORT CRITICAL ERROR: masa_internal.h corrupted!\n";
    print "Could not find registration area!\n";
    exit 1;
}


# closing and cleaning up
print "Done with masa_core.cpp...\n";
print "Cleaning up...\n\n";

close  INFILE or die $!;
close OUTFILE or die $!;

#rename($old, $bak);
#rename($new, $old);

# nick
# 10/17/11

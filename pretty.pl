#!/usr/bin/perl
#	solve2.pl
#	(c) 2020, B D Stephenson
#	brian@bstephen.me.uk
#
#	Except as otherwise stated, this Perl script meets the default standards
#	set by 'Perl Best Practices' by Damian Conway (O'Reilly). It has been
#	tested by Perl::Critic and has passed with no violations.

use feature qw(switch say);
use warnings;
use English '-no_match_vars';
use strict;
use Readonly;
use XML::LibXML;
use XML::LibXML::PrettyPrint;

our $VERSION = 1.2;
my $rc;

my $fname = $ARGV[0];

my $document = XML::LibXML->new->parse_file($fname);
my $pp       = XML::LibXML::PrettyPrint->new(
    indent_string => '    ',
    element       => {
        compact => [
            qw/Var Threat Refut Key Tot_Up Up_Caps Up_Checks Up_Fgivers Up_Flights Type Changed Added Removed/
        ],
    }
);
$pp->pretty_print($document);
$rc = print $document->toString;
exit 0;

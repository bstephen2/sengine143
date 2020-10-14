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
use DBI;
use Readonly;

Readonly::Scalar my $MAX_FAILS => 10;
Readonly::Scalar my $EMPTY     => q{};
Readonly::Scalar my $PROG_NAME => 'solve2.pl';
Readonly::Scalar my $AUTHOR    => 'Brian Stephenson';
Readonly::Scalar my $PARAM_1   => 1;
Readonly::Scalar my $PARAM_2   => 2;
Readonly::Scalar my $PARAM_3   => 3;

our $VERSION = 1.2;

my $rc;
my $db_handle;

$rc = print "\n$PROG_NAME, written by $AUTHOR\n\n";

$db_handle = meson_connect();
process($db_handle);
meson_disconnect($db_handle);

$rc = print "\n$PROG_NAME finished.\n";

exit 0;

sub process {
    my $dbh = shift;
    my $ct  = 0;
    my $rs;
    my $sth;
    my $cth;
    my $r_row;
    my $pid;
    my $gbr;
    my $pos;
    my $stip;
    my $kings;
    my $moves;
    my $cast;
    my $ep;
    my $extra;
    my $solution;
    my $class;
    my $engineout;
    my $command;
    my $meson;
    my $soundstatus;
    my $count = 0;
    my $sql =
        'SELECT pid, gbr, position, stip, kings, sound, cast, ep '
      . 'FROM problem '
      . 'WHERE (stip = \'#2\' ) AND '
      . 'sound = \'UNTESTED\' '
      . 'ORDER BY pid '
      . 'LIMIT 10000';

    my $upsql  = 'UPDATE classol SET sol = COMPRESS(?), class = COMPRESS(?) WHERE (pid = ?)';
    my $upsqlu = 'UPDATE classol SET sol = COMPRESS(?), class = NULL WHERE (pid = ?)';
    my $nupsql = 'UPDATE problem SET sound = ? WHERE pid = ? LIMIT 1';

    $sth = $dbh->prepare($sql);
    $sth->execute();

    while ( $r_row = $sth->fetchrow_arrayref() ) {
        ## no critic (ProhibitMagicNumbers)

        $pid   = $r_row->[0];
        $gbr   = $r_row->[1];
        $pos   = $r_row->[2];
        $stip  = $r_row->[3];
        $moves = substr $stip, 1, 1;
        $kings = $r_row->[4];
        $cast  = $r_row->[6];
        $ep    = $r_row->[7];

        $extra = $EMPTY;

        if ( ( defined $cast ) && ( length $cast > 0 ) ) {
            $extra = $extra . "--castling=$cast ";
        }

        if ( ( defined $ep ) && ( length $ep > 0 ) ) {
            $extra = $extra . "--ep=$ep";
        }

        $command =
"sengine142 --moves=$moves --gbr=$gbr --kings=$kings --pos=$pos --actual --fleck --set --tries --meson --classify $extra |";

        ## use critic
        $count++;
        $rs        = print "No. $count ($stip) $pid ";
        $engineout = $EMPTY;

        ## no critic (ProhibitTwoArgOpen)

        open $meson, $command || die "Can't run sengine142!!\n";
        ## use critic

        while (<$meson>) {
            $engineout .= $_;
        }

        $rs = close $meson;

        ($soundstatus) = $engineout =~ m/<Soundness>(.*)<\/Soundness>/xsm;
        printf "Status = %s\n", $soundstatus;

        $cth = $dbh->prepare($nupsql);
        $cth->bind_param( 1, $soundstatus );
        $cth->bind_param( 2, $pid );
        $cth->execute();
        $cth->finish();

        if ( $soundstatus eq 'SOUND' ) {
            ( $solution, $class ) =
              $engineout =~ /(^<MesonSolution>.*<\/MesonSolution>).*(<MesonClass>.*<\/MesonClass>)$/xms;

            #$rs = say "$solution\n";
            #$rs = say "$class\n";

            $cth = $dbh->prepare($upsql);
            $cth->bind_param( $PARAM_1, $solution );
            $cth->bind_param( $PARAM_2, $class );
            $cth->bind_param( $PARAM_3, $pid );
            $cth->execute();
            $cth->finish();
        }
        else {
            $cth = $dbh->prepare($upsqlu);
            $cth->bind_param( $PARAM_1, $engineout );
            $cth->bind_param( $PARAM_2, $pid );
            $cth->execute();
            $cth->finish();
        }

        $ct++;
    }

    $sth->finish();

    $rs = print "\t$ct mates in 2 solved\n";

    return;
}

sub meson_connect {
    my $server   = 'localhost';
    my $db       = 'meson';
    my $user     = 'bstephen';
    my $password = 'rice37';
    my $dbh;
    my $ct = 0;
    my %attr = ( PrintError => 1, RaiseError => 1 );

    ## no critic (ProhibitPostFixControls)
    do {
        $dbh = eval {
            my $dbhandle = DBI->connect( "dbi:mysql:$db:$server", $user, $password, \%attr );

            return $dbhandle;
        };

        if ( !defined $dbh ) {
            $ct++;
            ( $ct == $MAX_FAILS )
              && ( die "10 failed connect tries. Baling out ...\n" );
        }
    } until defined $dbh;
    ## use critic

    return $dbh;
}

sub meson_disconnect {
    my ($dbh) = @_;

    $dbh->disconnect();

    return;
}

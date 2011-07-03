#!/usr/bin/perl

my ($lotto_file, $win_num) = @ARGV;

my @win_num_arr  = split ':', $win_num;
my $win_mega     = pop @win_num_arr;
my %win_num_hash = map {int($_) => 1} @win_num_arr;

print "Winning Number: @win_num_arr <b>$win_mega</b>\n<br><br/>\n";

open LOTTO_FILE_HANDLE, $lotto_file;

FILE_LOOP:
while (<LOTTO_FILE_HANDLE>) {
    my $lotto_num = $_;

    chomp $lotto_num;

    if ($lotto_num !~ m/(\d{2}:){5}\d{2}/) {
        print "$lotto_num\n";
        next FILE_LOOP;
    }

    my @lotto_num_arr = split ':', $lotto_num;
    my $lotto_mega    = pop @lotto_num_arr;

    my $lotto_num_str      = q{};
    my $num_non_mega_match = 0;

    foreach my $lotto_num (@lotto_num_arr) {
        if (defined $win_num_hash{int($lotto_num)}) {
            $lotto_num_str .= qq{<font color="#0000FF">$lotto_num</font> };
            $num_non_mega_match++;
        }
        else {
            $lotto_num_str .= "$lotto_num ";
        }
    }

    my $lotto_res_str; 

    if ($win_mega == $lotto_mega) {
        $lotto_res_str = qq{Number: &nbsp $lotto_num_str <b><font color="#0000FF">$lotto_mega</font></b>, &nbsp Match: $num_non_mega_match + Mega<br/>};
    }
    else {
        $lotto_res_str = qq{Number: &nbsp $lotto_num_str <b>$lotto_mega</b>, &nbsp Match: $num_non_mega_match<br/>};
    }

    print "$lotto_res_str\n";
}

close LOTTO_FILE_HANDLE;

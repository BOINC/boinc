#!/usr/bin/perl


print "Content-type: text/plain\n\nHet werkt!\n";
print "$_ = '$ENV{ $_ }'\n" for sort keys %ENV;

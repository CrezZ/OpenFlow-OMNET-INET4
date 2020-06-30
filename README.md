Teting ofr rewrite Openflow for INET 4

1 Change EthernetIIFrame *frame ==> Paket *paket
1.1 change *frame from function param to frame=<EthernetMacHeader *>packet
1.2 change cPaket to Paket and change logic 
1.3 sending Paket, not frame...

2 Change MANY MANY letter cases MACHeader --> MacHeader, ICMPHeader -->IcmpHeader .... 


for ((i=1;i <= 15000;i++));
do
	{
		for ((i=1;i <= 200;i++));
		do
			{
				printf "*3\r\n\$6\r\nLPUSHX\r\n\$1\r\n0\r\n\$77\r\nPOST /driviewApply.json HTTP/1.0\r\nHost: 127.0.0.1:8888\r\nConnection: close\r\n\r\n\r\n" | nc 192.168.1.15 5000 > /dev/null
			}&
		done
		wait
	}
done


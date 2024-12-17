make && scp ./libtestt.so.1.0.0 rmpp:main.so && ssh -tt rmpp "LD_PRELOAD=/home/root/main.so /usr/bin/xochitl"

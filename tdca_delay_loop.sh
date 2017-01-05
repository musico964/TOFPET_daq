#!/bin/bash
coarse_delay_start=3
coarse_delay_end=3
fine_delay_start=0
fine_delay_end=1
template_config=/home/musico/TOPEM/EndoProbe/acq/out/test_1701/delay_scan/config_delay.template

for ((i=$coarse_delay_start;i<=$coarse_delay_end;i++)); do
	for ((j=$fine_delay_start;j<=$fine_delay_end;j++)); do
		rm -f config_delay.txt
		x=$i$j
		sed s/__FNAME__/delay_$x/ $template_config >> config1.txt
		sed s/__FINE_DELAY__/$j/ config1.txt >> config2.txt
		sed s/__COARSE_DELAY__/$i/ config2.txt >> config_delay.txt
		rm -f config1.txt config2.txt
		./Daq -cfg config_delay.txt
	done
done

